#include "../engine/Assets/src/ImageCacheChecksum.hpp"
#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>

int main()
{
    using namespace ForgeSim::Assets::Detail;
    try
    {
        const std::array<std::uint8_t,9> known{'1','2','3','4','5','6','7','8','9'};
        if (ImageCrc32(known) != 0xcbf43926u || ImageCrc32({}) != 0)
            throw std::runtime_error("IEEE CRC32 known vectors failed");
        std::vector<std::uint8_t> bytes(65540);
        std::uint32_t state=135;
        for(auto& byte:bytes) { state=state*1664525u+1013904223u;byte=static_cast<std::uint8_t>(state>>24); }
        for (auto size : {0,1,2,3,4,7,8,16,31,32,63,64,255,256,1023,4096,65537})
        {
            const auto input=std::span<const std::uint8_t>(bytes).subspan(3,static_cast<std::size_t>(size));
            if(ImageCrc32(input)!=PortableImageCrc32(input))
                throw std::runtime_error("Accelerated and portable checksums must agree");
        }
        if (ImageCacheChecksum(known,1,9)==ImageCacheChecksum(known,9,1))
            throw std::runtime_error("Dimensions must contribute to integrity check");
        std::cout<<"Cache CRC32 vectors, tails, alignment and build-path compatibility passed\n";
    }
    catch(const std::exception& error) { std::cerr<<error.what()<<'\n';return 1; }
}
