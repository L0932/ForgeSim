#include "../engine/Assets/src/ImageCacheCodec.hpp"
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>

using namespace ForgeSim::Assets::Detail;
namespace
{
    void Require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
}
int main()
{
    try
    {
        std::vector<std::uint8_t> source(256 * 256 * 4);
        for (std::size_t i = 0; i < source.size(); ++i) source[i] = static_cast<std::uint8_t>(i % 13);
        const auto encoded = CompressCachePixels(source);
        Require(!encoded.empty() && encoded.size() <= source.size() * 7 / 8, "Compressible data must meet threshold");
        ImageCacheHeader h{256,256,4,123,ImageCacheMagic,1,encoded.size(),source.size()};
        std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
        Require(WriteImageCacheHeader(stream,h), "Write header");
        Require(stream.str().size()==64 && stream.str()[0]==0 && stream.str()[1]==1,
            "Header must use explicit little endian fields");
        ImageCacheHeader read{};
        Require(ReadImageCacheHeader(stream,read) && read==h, "Read header exactly");
        std::vector<std::uint8_t> restored;
        Require(RestoreCachePixels(h,encoded,restored) && restored==source, "LZ4 pixel roundtrip");
        auto bad=h; bad[0]=std::numeric_limits<std::uint64_t>::max();
        Require(!ValidateImageCacheHeader(bad,64+encoded.size()), "Overflowing dimensions");
        for (std::size_t field : {1u,2u,4u,5u,6u,7u})
        {
            bad=h; bad[field]=std::numeric_limits<std::uint64_t>::max();
            Require(!ValidateImageCacheHeader(bad,64+encoded.size()), "Reject malformed field before allocation");
        }
        Require(!ValidateImageCacheHeader(h,64+encoded.size()+1), "Reject trailing file data");
        auto truncated=encoded; truncated.pop_back();
        Require(!RestoreCachePixels(h,truncated,restored), "Reject payload length mismatch");
        bad=h; --bad[6];
        Require(!RestoreCachePixels(bad,truncated,restored), "Reject truncated LZ4 stream");
        std::vector<std::uint8_t> noise(source.size());
        std::mt19937 random(148);
        for(auto& byte:noise) byte=static_cast<std::uint8_t>(random());
        Require(CompressCachePixels(noise).empty(), "Incompressible data must use raw fallback");
        ImageCacheHeader raw{256,256,4,0,ImageCacheMagic,0,noise.size(),noise.size()};
        Require(RestoreCachePixels(raw,noise,restored) && restored==noise, "Raw roundtrip preserves alpha and all channels");
        Require(CompressCachePixels(std::vector<std::uint8_t>(65532,0)).empty(), "Small data must remain raw");
        std::istringstream shortHeader("short");
        Require(!ReadImageCacheHeader(shortHeader,read), "Truncated header");
        std::cout << "Cache codec bounds, lossless roundtrips and fallback passed\n";
        return 0;
    }
    catch(const std::exception& error) { std::cerr<<error.what()<<'\n'; return 1; }
}
