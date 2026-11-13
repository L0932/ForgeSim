#include "ScreenshotCapture.hpp"
#include <stdexcept>
#include <glad/gl.h>

namespace ForgeSim::Sandbox
{
    namespace
    {
        struct PackState
        {
            GLint buffer=0,alignment=0,rowLength=0,skipRows=0,skipPixels=0,imageHeight=0,skipImages=0;
            PackState()
            {
                glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING,&buffer);
                glGetIntegerv(GL_PACK_ALIGNMENT,&alignment);glGetIntegerv(GL_PACK_ROW_LENGTH,&rowLength);
                glGetIntegerv(GL_PACK_SKIP_ROWS,&skipRows);glGetIntegerv(GL_PACK_SKIP_PIXELS,&skipPixels);
                glGetIntegerv(GL_PACK_IMAGE_HEIGHT,&imageHeight);glGetIntegerv(GL_PACK_SKIP_IMAGES,&skipImages);
                glBindBuffer(GL_PIXEL_PACK_BUFFER,0);
                glPixelStorei(GL_PACK_ALIGNMENT,1);glPixelStorei(GL_PACK_ROW_LENGTH,0);
                glPixelStorei(GL_PACK_SKIP_ROWS,0);glPixelStorei(GL_PACK_SKIP_PIXELS,0);
                glPixelStorei(GL_PACK_IMAGE_HEIGHT,0);glPixelStorei(GL_PACK_SKIP_IMAGES,0);
            }
            ~PackState()
            {
                glBindBuffer(GL_PIXEL_PACK_BUFFER,static_cast<GLuint>(buffer));
                glPixelStorei(GL_PACK_ALIGNMENT,alignment);glPixelStorei(GL_PACK_ROW_LENGTH,rowLength);
                glPixelStorei(GL_PACK_SKIP_ROWS,skipRows);glPixelStorei(GL_PACK_SKIP_PIXELS,skipPixels);
                glPixelStorei(GL_PACK_IMAGE_HEIGHT,imageHeight);glPixelStorei(GL_PACK_SKIP_IMAGES,skipImages);
            }
        };
        struct ReadBufferState
        {
            GLint framebuffer=0,defaultReadBuffer=0;
            ReadBufferState()
            {
                glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING,&framebuffer);
                glBindFramebuffer(GL_READ_FRAMEBUFFER,0);
                glGetIntegerv(GL_READ_BUFFER,&defaultReadBuffer);glReadBuffer(GL_BACK);
            }
            ~ReadBufferState()
            {
                glBindFramebuffer(GL_READ_FRAMEBUFFER,0);
                glReadBuffer(static_cast<GLenum>(defaultReadBuffer));
                glBindFramebuffer(GL_READ_FRAMEBUFFER,static_cast<GLuint>(framebuffer));
            }
        };
        void CheckReadback()
        {
            if(glGetError()!=GL_NO_ERROR) throw std::runtime_error("OpenGL screenshot readback failed.");
        }
    }
    ScreenshotImage CaptureSceneTexture(unsigned int texture)
    {
        if (!texture || !glIsTexture(texture)) throw std::runtime_error("Scene viewport has no rendered image to capture.");
        GLint width=0,height=0;
        glGetTextureLevelParameteriv(texture,0,GL_TEXTURE_WIDTH,&width);
        glGetTextureLevelParameteriv(texture,0,GL_TEXTURE_HEIGHT,&height);
        if(width<=0 || height<=0) throw std::runtime_error("Scene viewport has no drawable image.");
        ScreenshotImage image{static_cast<std::uint32_t>(width),static_cast<std::uint32_t>(height),{}};
        image.rgba.resize(ScreenshotByteCount(image.width,image.height));
        {
            const PackState saved;
            glGetTextureImage(texture,0,GL_RGBA,GL_UNSIGNED_BYTE,static_cast<GLsizei>(image.rgba.size()),image.rgba.data());
            CheckReadback();
        }
        NormalizeScreenshot(image);return image;
    }
    ScreenshotImage CaptureEditorBackbuffer(std::uint32_t width,std::uint32_t height)
    {
        ScreenshotImage image{width,height,{}};image.rgba.resize(ScreenshotByteCount(width,height));
        {
            const PackState pack;
            const ReadBufferState read;
            glReadPixels(0,0,static_cast<GLsizei>(width),static_cast<GLsizei>(height),GL_RGBA,GL_UNSIGNED_BYTE,image.rgba.data());
            CheckReadback();
        }
        NormalizeScreenshot(image);return image;
    }
}
