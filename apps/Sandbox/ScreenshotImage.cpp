#include "ScreenshotImage.hpp"
#include <algorithm>
#include <fstream>
#include <limits>
#include <stdexcept>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace ForgeSim::Sandbox
{
    std::size_t ScreenshotByteCount(std::uint32_t width,std::uint32_t height)
    {
        if (width==0 || height==0 || width>static_cast<std::uint32_t>(std::numeric_limits<int>::max()) ||
            height>static_cast<std::uint32_t>(std::numeric_limits<int>::max()))
            throw std::invalid_argument("Screenshot dimensions must be positive and fit the image encoder.");
        const auto bytes=std::uint64_t{width}*height*4;
        if (bytes>static_cast<std::uint64_t>(std::numeric_limits<int>::max()))
            throw std::invalid_argument("Screenshot is too large for image readback.");
        return static_cast<std::size_t>(bytes);
    }
    void NormalizeScreenshot(ScreenshotImage& image)
    {
        if (image.rgba.size()!=ScreenshotByteCount(image.width,image.height))
            throw std::invalid_argument("Screenshot pixel buffer does not match its dimensions.");
        const auto stride=static_cast<std::size_t>(image.width)*4;
        for(std::size_t row=0;row<image.height/2;++row)
            for(std::size_t column=0;column<stride;++column)
                std::swap(image.rgba[row*stride+column],image.rgba[(image.height-1-row)*stride+column]);
        for(std::size_t i=3;i<image.rgba.size();i+=4) image.rgba[i]=255;
    }
    void WriteScreenshotPng(const std::filesystem::path& path,const ScreenshotImage& image)
    {
        if (image.rgba.size()!=ScreenshotByteCount(image.width,image.height))
            throw std::invalid_argument("Screenshot pixel buffer does not match its dimensions.");
        if (std::filesystem::exists(path)) throw std::runtime_error("Screenshot file already exists.");
        // std::filesystem paths support Unicode on Windows; avoid stb's narrow fopen path.
        std::ofstream output(path,std::ios::binary);
        if (!output) throw std::runtime_error("Cannot create screenshot file: "+path.string());
        try
        {
            const auto write=[](void* context,void* data,int size)
            { static_cast<std::ofstream*>(context)->write(static_cast<const char*>(data),size); };
            const int encoded=stbi_write_png_to_func(write,&output,static_cast<int>(image.width),
                static_cast<int>(image.height),4,image.rgba.data(),static_cast<int>(image.width*4));
            output.close();
            if (!encoded || !output) throw std::runtime_error("Could not finish writing screenshot: "+path.string());
        }
        catch(...)
        {
            output.close();std::error_code ignored;std::filesystem::remove(path,ignored);throw;
        }
    }
}
