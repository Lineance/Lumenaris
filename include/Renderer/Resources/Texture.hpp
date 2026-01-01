#pragma once
#include <string>
#include <glad/glad.h>

namespace Renderer
{
    class Texture
    {
    public:
        Texture();
        ~Texture();

        // 加载纹理文件
        bool LoadFromFile(const std::string& filepath);

        // 绑定纹理到指定的纹理单元
        // ⭐ 默认使用纹理单元1（TextureUnit::MATERIAL_DIFFUSE），为ImGui预留单元0
        void Bind(GLenum textureUnit = GL_TEXTURE1) const;

        // 解绑纹理
        void Unbind() const;

        // 静态解绑方法
        static void UnbindStatic() { glBindTexture(GL_TEXTURE_2D, 0); }

        // 获取纹理ID
        GLuint GetID() const { return m_textureID; }

        // 检查纹理是否加载成功
        bool IsLoaded() const { return m_loaded; }

        // 获取纹理文件名
        const std::string& GetFilePath() const { return m_filepath; }

    private:
        GLuint m_textureID;
        bool m_loaded;
        std::string m_filepath;

        // 清理资源
        void Cleanup();
    };

} // namespace Renderer
