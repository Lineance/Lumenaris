#define STB_IMAGE_IMPLEMENTATION
#include "Renderer/Texture.hpp"
#include <iostream>
#include <stb_image.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace Renderer
{

    Texture::Texture()
        : m_textureID(0), m_loaded(false)
    {
    }

    Texture::~Texture()
    {
        Cleanup();
    }

    bool Texture::LoadFromFile(const std::string& filepath)
    {
        // 如果已经有纹理，先清理
        Cleanup();

        m_filepath = filepath;

        // 检查文件是否存在
        if (!fs::exists(filepath))
        {
            std::cerr << "Texture file not found: " << filepath << std::endl;
            return false;
        }

        // 加载图像数据
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true); // OpenGL的纹理坐标Y轴是反的
        unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);

        if (!data)
        {
            std::cerr << "Failed to load texture: " << filepath << std::endl;
            std::cerr << "STB Image error: " << stbi_failure_reason() << std::endl;
            return false;
        }

        // 生成纹理
        glGenTextures(1, &m_textureID);
        glBindTexture(GL_TEXTURE_2D, m_textureID);

        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // 根据通道数确定格式
        GLenum format;
        if (channels == 4)
            format = GL_RGBA;
        else if (channels == 3)
            format = GL_RGB;
        else if (channels == 1)
            format = GL_RED;
        else
        {
            std::cerr << "Unsupported texture format with " << channels << " channels" << std::endl;
            stbi_image_free(data);
            Cleanup();
            return false;
        }

        // 上传纹理数据
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // 释放图像数据
        stbi_image_free(data);

        // 检查OpenGL错误
        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            std::cerr << "OpenGL error loading texture: " << error << std::endl;
            Cleanup();
            return false;
        }

        // 解绑纹理
        glBindTexture(GL_TEXTURE_2D, 0);

        m_loaded = true;
        std::cout << "Loaded texture: " << filepath << " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;

        return true;
    }

    void Texture::Bind(GLenum textureUnit) const
    {
        if (!m_loaded)
            return;

        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, m_textureID);
    }

    void Texture::Unbind() const
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture::Cleanup()
    {
        if (m_textureID != 0)
        {
            glDeleteTextures(1, &m_textureID);
            m_textureID = 0;
        }
        m_loaded = false;
        m_filepath.clear();
    }

} // namespace Renderer
