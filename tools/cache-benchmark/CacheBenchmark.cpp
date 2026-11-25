#include <ForgeSim/Assets/GltfLoader.hpp>
#include <ForgeSim/Assets/ImageLoader.hpp>
#include <libdeflate.h>

#include <algorithm>
#include <charconv>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    namespace fs = std::filesystem;
    using Bytes = std::vector<std::uint8_t>;
    using Clock = std::chrono::steady_clock;
    double Elapsed(Clock::time_point start)
    {
        return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
    }
    double Median(std::vector<double> values)
    {
        std::sort(values.begin(), values.end());
        return values[values.size() / 2]; // Run count is required to be odd.
    }
    struct Scratch
    {
        fs::path path;
        Scratch()
        {
            for (int attempt = 0; attempt < 16; ++attempt)
            {
                auto candidate = fs::temp_directory_path() /
                    ("ForgeSim-cache-benchmark-" + std::to_string(std::random_device{}()));
                if (fs::create_directory(candidate)) { path = std::move(candidate); return; }
            }
            throw std::runtime_error("Could not create benchmark scratch directory.");
        }
        ~Scratch() { std::error_code error; fs::remove_all(path, error); }
    };
    void Write(const fs::path& path, const Bytes& bytes)
    {
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        out.close();
        if (!out) throw std::runtime_error("Could not write scratch payload.");
    }
    Bytes Read(const fs::path& path, std::size_t size)
    {
        Bytes bytes(size);
        std::ifstream in(path, std::ios::binary);
        if (!in.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(size)))
            throw std::runtime_error("Could not read scratch payload.");
        return bytes;
    }
    void Benchmark(const ForgeSim::Assets::ImageData& image, std::size_t index,
        int runs, const fs::path& scratch)
    {
        const auto& original = image.pixels;
        if (original.empty()) throw std::runtime_error("Empty image.");
        const auto expectedCrc = libdeflate_crc32(0, original.data(), original.size());
        for (const int level : {0, 1, 3, 6})
        {
            std::unique_ptr<libdeflate_compressor, decltype(&libdeflate_free_compressor)> compressor{
                level ? libdeflate_alloc_compressor(level) : nullptr, libdeflate_free_compressor};
            std::unique_ptr<libdeflate_decompressor, decltype(&libdeflate_free_decompressor)> decompressor{
                libdeflate_alloc_decompressor(), libdeflate_free_decompressor};
            if ((level && !compressor) || !decompressor) throw std::bad_alloc{};
            Bytes encoded;
            std::vector<double> compression, writes, reads;
            std::size_t stored = 0;
            // One unreported warm-up followed by independently measured runs.
            for (int run = -1; run < runs; ++run)
            {
                auto start = Clock::now();
                if (level)
                {
                    encoded.resize(libdeflate_deflate_compress_bound(compressor.get(), original.size()));
                    stored = libdeflate_deflate_compress(compressor.get(), original.data(), original.size(),
                        encoded.data(), encoded.size());
                    if (!stored) throw std::runtime_error("Compression failed.");
                    encoded.resize(stored);
                }
                else stored = original.size();
                const double compressMs = level ? Elapsed(start) : 0;
                const auto& payload = level ? encoded : original;
                start = Clock::now();
                Write(scratch, payload);
                const double writeMs = Elapsed(start);
                start = Clock::now();
                auto input = Read(scratch, stored);
                Bytes restored;
                if (level)
                {
                    restored.resize(original.size());
                    std::size_t consumed = 0;
                    if (libdeflate_deflate_decompress_ex(decompressor.get(), input.data(), input.size(),
                        restored.data(), restored.size(), &consumed, nullptr) != LIBDEFLATE_SUCCESS ||
                        consumed != input.size()) throw std::runtime_error("Decompression failed.");
                }
                else restored = std::move(input);
                const auto crc = libdeflate_crc32(0, restored.data(), restored.size());
                const double readMs = Elapsed(start);
                // Full byte comparison is deliberately outside the timed read.
                if (crc != expectedCrc || restored != original)
                    throw std::runtime_error("Lossless round trip changed image pixels.");
                if (run >= 0)
                {
                    compression.push_back(compressMs);
                    writes.push_back(writeMs);
                    reads.push_back(readMs);
                }
            }
            std::cout << index << ',' << image.width << ',' << image.height << ',' << original.size() << ','
                << level << ',' << stored << ',' << 100.0 * (1.0 - static_cast<double>(stored) /
                    static_cast<double>(original.size())) << ','
                << Median(compression) << ',' << Median(writes) << ',' << Median(reads) << ",yes\n";
        }
    }
}
int main(int argc, char** argv)
{
    try
    {
        if (argc < 2 || argc > 3)
        {
            std::cerr << "Usage: ForgeSimCacheBenchmark model.glb|image.png [odd-runs: 3..21]\n";
            return 2;
        }
        int runs = 3;
        if (argc == 3)
        {
            const std::string value = argv[2];
            const auto result = std::from_chars(value.data(), value.data() + value.size(), runs);
            if (result.ec != std::errc{} || result.ptr != value.data() + value.size() ||
                runs < 3 || runs > 21 || runs % 2 == 0)
                throw std::invalid_argument("Run count must be odd and between 3 and 21.");
        }
        const fs::path source = argv[1];
        std::vector<ForgeSim::Assets::ImageData> images;
        const auto start = Clock::now();
        if (source.extension() == ".glb" || source.extension() == ".gltf")
        {
            // Persistent caching remains disabled; benchmark decoded source pixels.
            ForgeSim::Assets::GltfImportOptions options;
            options.texturePolicy = ForgeSim::Assets::GltfTexturePolicy::SkipUnusableTextures;
            auto scene = ForgeSim::Assets::LoadGltfScene(source, options);
            images = std::move(scene.textureResources.images);
        }
        else images.push_back(ForgeSim::Assets::LoadImage(source));
        std::cerr << "Source CPU load: " << Elapsed(start) << " ms; images=" << images.size()
            << "; runs=" << runs << ". Scratch reads use the OS file cache.\n";
        if (images.empty()) throw std::runtime_error("Source has no images to benchmark.");
        Scratch scratch;
        std::cout << "image,width,height,decoded_bytes,deflate_level,stored_bytes,saved_percent,"
            "compression_ms,write_io_ms,warm_read_restore_crc_ms,exact_pixels\n";
        for (std::size_t i = 0; i < images.size(); ++i)
            Benchmark(images[i], i, runs, scratch.path / "payload.bin");
        return 0;
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
