#include <ForgeSim/Assets/ImageLoader.hpp>
#include <stb_image.h>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

int main()
{
    struct Fixture { const char* name; std::vector<std::uint8_t> png; };
    const std::vector<Fixture> fixtures{
        {"color0-adam70", {137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,3,0,0,0,2,16,0,0,0,0,232,143,229,133,0,0,0,20,73,68,65,84,120,156,99,96,96,96,96,100,248,15,196,140,140,245,255,1,10,26,2,130,91,76,118,119,0,0,0,0,73,69,78,68,174,66,96,130}},
        {"color0-transparency", {137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,3,0,0,0,2,16,0,0,0,0,232,143,229,133,0,0,0,2,116,82,78,83,0,0,118,147,205,56,0,0,0,20,73,68,65,84,120,156,99,96,96,96,96,100,248,15,196,140,140,245,255,1,10,26,2,130,91,76,118,119,0,0,0,0,73,69,78,68,174,66,96,130}},
        {"color0-adam71", {137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,3,0,0,0,2,16,0,0,0,1,159,136,213,19,0,0,0,20,73,68,65,84,120,156,99,96,0,130,255,12,12,140,32,200,88,255,31,0,13,23,2,130,173,100,86,247,0,0,0,0,73,69,78,68,174,66,96,130}},
        {"color2-adam70", {137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,3,0,0,0,2,16,2,0,0,0,66,134,45,14,0,0,0,37,73,68,65,84,120,156,99,96,96,96,96,100,248,207,200,192,200,88,255,191,129,225,255,191,255,255,25,132,76,86,159,53,9,67,150,0,0,213,140,11,198,232,219,230,70,0,0,0,0,73,69,78,68,174,66,96,130}},
        {"color2-transparency", {137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,3,0,0,0,2,16,2,0,0,0,66,134,45,14,0,0,0,6,116,82,78,83,0,0,0,1,0,255,66,102,130,43,0,0,0,37,73,68,65,84,120,156,99,96,96,96,96,100,248,207,200,192,200,88,255,191,129,225,255,191,255,255,25,132,76,86,159,53,9,67,150,0,0,213,140,11,198,232,219,230,70,0,0,0,0,73,69,78,68,174,66,96,130}},
        {"color2-adam71", {137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,3,0,0,0,2,16,2,0,0,1,53,129,29,152,0,0,0,39,73,68,65,84,120,156,99,96,96,96,96,100,248,207,208,192,240,255,223,255,255,64,38,35,99,253,127,6,33,147,213,103,77,194,32,82,16,33,0,237,229,11,198,43,87,79,30,0,0,0,0,73,69,78,68,174,66,96,130}},
        {"color4-adam70", {137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,3,0,0,0,2,16,4,0,0,0,103,237,114,210,0,0,0,35,73,68,65,84,120,156,99,96,96,96,96,100,248,207,200,192,200,88,255,159,161,129,225,255,191,255,255,133,76,86,159,53,9,3,0,88,201,9,69,163,140,246,248,0,0,0,0,73,69,78,68,174,66,96,130}},
        {"color4-adam71", {137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,3,0,0,0,2,16,4,0,0,1,16,234,66,68,0,0,0,37,73,68,65,84,120,156,99,96,96,96,96,100,96,100,172,255,207,192,240,159,145,129,161,129,225,255,191,255,255,133,76,86,159,53,9,3,0,92,77,9,69,141,231,15,86,0,0,0,0,73,69,78,68,174,66,96,130}},
        {"color6-adam70", {137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,3,0,0,0,2,16,6,0,0,0,205,228,186,89,0,0,0,37,73,68,65,84,120,156,99,96,96,96,96,100,248,207,200,192,200,88,255,191,129,225,255,191,255,255,133,76,86,159,53,9,99,192,37,1,0,148,65,18,137,125,148,68,189,0,0,0,0,73,69,78,68,174,66,96,130}},
        {"color6-adam71", {137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,3,0,0,0,2,16,6,0,0,1,186,227,138,207,0,0,0,42,73,68,65,84,120,156,99,96,96,96,96,100,248,207,200,192,240,255,191,144,201,234,179,38,97,12,140,140,245,255,27,24,254,255,99,128,73,193,4,96,42,0,156,211,18,137,236,164,23,156,0,0,0,0,73,69,78,68,174,66,96,130}},
    };
    try
    {
        for (const auto& fixture : fixtures)
        {
            int width=0, height=0, channels=0;
            const std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> reference{
                stbi_load_from_memory(fixture.png.data(), static_cast<int>(fixture.png.size()),
                    &width,&height,&channels,4), stbi_image_free};
            if (!reference) throw std::runtime_error("Invalid test fixture: " + std::string(fixture.name));
            const auto image = ForgeSim::Assets::DecodeImage(fixture.png, fixture.name);
            const std::vector<std::uint8_t> expected(reference.get(), reference.get()+width*height*4);
            if (image.width!=3 || image.height!=2 || image.channelCount!=4 || image.pixels!=expected)
                throw std::runtime_error("RGBA8 mismatch: " + std::string(fixture.name));
        }
        auto truncated=fixtures.front().png;
        truncated.resize(26);
        bool rejected=false;
        try { static_cast<void>(ForgeSim::Assets::DecodeImage(truncated,"truncated PNG")); }
        catch(const std::runtime_error&) { rejected=true; }
        if(!rejected) throw std::runtime_error("Truncated PNG must fail");
        // Exercise both zlib-wrapped PNG and raw CgBI inflate, including growth
        // beyond the initial capacity and malformed-stream rejection.
        const std::vector<std::uint8_t> compressed{120,156,115,116,28,5,163,96,20,140,130,81,48,10,70,193,72,3,0,133,239,8,31};
        for (int wrapped : {0,1})
        {
            int length=0;
            const auto* data=compressed.data()+(wrapped ? 0 : 2);
            const auto size=compressed.size()-(wrapped ? 0 : 6);
            const std::unique_ptr<char, decltype(&stbi_image_free)> inflated{
                stbi_zlib_decode_malloc_guesssize_headerflag(reinterpret_cast<const char*>(data),
                    static_cast<int>(size),1,&length,wrapped), stbi_image_free};
            if(!inflated || length!=2048) throw std::runtime_error("Inflate growth failed");
            for(int i=0;i<length;++i) if(inflated.get()[i]!='A') throw std::runtime_error("Inflate bytes changed");
        }
        const std::unique_ptr<char, decltype(&stbi_image_free)> invalid{
            stbi_zlib_decode_malloc_guesssize_headerflag("bad",3,8,nullptr,1), stbi_image_free};
        if(invalid) throw std::runtime_error("Corrupt zlib stream must fail");
        std::cout << "16-bit PNG grayscale/RGB/alpha, transparency, Adam7 and rejection checks passed\n";
    }
    catch(const std::exception& error) { std::cerr << error.what() << '\n';return 1; }
}
