#include "Renderer/Environment/Skybox.hpp"
#include "Core/Logger.hpp"
#include <glad/glad.h>
#include <vector>
#include <cmath>
#include <stb_image.h>
#include <filesystem>

namespace fs = std::filesystem;

namespace Renderer
{

    Skybox::Skybox()
        : m_textureID(0), m_VAO(0), m_VBO(0), m_isInitialized(false), m_rotation(0.0f)
    {
    }

    Skybox::~Skybox()
    {
        if (m_VAO != 0)
        {
            glDeleteVertexArrays(1, &m_VAO);
            m_VAO = 0;
        }
        if (m_VBO != 0)
        {
            glDeleteBuffers(1, &m_VBO);
            m_VBO = 0;
        }
        if (m_textureID != 0)
        {
            glDeleteTextures(1, &m_textureID);
            m_textureID = 0;
        }
    }

    Skybox::Skybox(Skybox&& other) noexcept
        : m_textureID(other.m_textureID)
        , m_shader(std::move(other.m_shader))
        , m_VAO(other.m_VAO)
        , m_VBO(other.m_VBO)
        , m_isInitialized(other.m_isInitialized)
        , m_rotation(other.m_rotation)
    {
        other.m_textureID = 0;
        other.m_VAO = 0;
        other.m_VBO = 0;
        other.m_isInitialized = false;
    }

    Skybox& Skybox::operator=(Skybox&& other) noexcept
    {
        if (this != &other)
        {
            if (m_VAO != 0) glDeleteVertexArrays(1, &m_VAO);
            if (m_VBO != 0) glDeleteBuffers(1, &m_VBO);
            if (m_textureID != 0) glDeleteTextures(1, &m_textureID);

            m_textureID = other.m_textureID;
            m_shader = std::move(other.m_shader);
            m_VAO = other.m_VAO;
            m_VBO = other.m_VBO;
            m_isInitialized = other.m_isInitialized;
            m_rotation = other.m_rotation;

            other.m_textureID = 0;
            other.m_VAO = 0;
            other.m_VBO = 0;
            other.m_isInitialized = false;
        }
        return *this;
    }

    bool Skybox::Initialize()
    {
        if (m_isInitialized)
        {
            Core::Logger::GetInstance().Warning("Skybox already initialized");
            return true;
        }

        CreateCubeMesh();
        m_isInitialized = true;

        Core::Logger::GetInstance().Info("Skybox initialized successfully");
        return true;
    }

    bool Skybox::Load(
        const std::string& right,
        const std::string& left,
        const std::string& top,
        const std::string& bottom,
        const std::string& back,
        const std::string& front)
    {
        if (!m_isInitialized)
        {
            Core::Logger::GetInstance().Error("Skybox not initialized. Call Initialize() first.");
            return false;
        }

        // 存储6个面的文件路径
        std::vector<std::string> faces = {
            right, left, top, bottom, back, front
        };

        // 生成cubemap纹理
        glGenTextures(1, &m_textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

        // 加载6个面的纹理
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            // 检查文件是否存在
            if (!fs::exists(faces[i]))
            {
                Core::Logger::GetInstance().Error("Skybox texture file not found: " + faces[i]);
                glDeleteTextures(1, &m_textureID);
                m_textureID = 0;
                return false;
            }

            int width, height, channels;
            stbi_set_flip_vertically_on_load(false); // 天空盒不需要翻转
            unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &channels, 0);

            if (!data)
            {
                Core::Logger::GetInstance().Error("Failed to load skybox texture: " + faces[i]);
                Core::Logger::GetInstance().Error("STB Image error: " + std::string(stbi_failure_reason()));
                glDeleteTextures(1, &m_textureID);
                m_textureID = 0;
                return false;
            }

            // 确定格式
            GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

            // 上传纹理数据到cubemap的对应面
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format,
                        width, height, 0, format, GL_UNSIGNED_BYTE, data);

            stbi_image_free(data);
        }

        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        Core::Logger::GetInstance().Info("Skybox cubemap loaded successfully (ID: " +
                                        std::to_string(m_textureID) + ")");
        return true;
    }

    bool Skybox::LoadShaders(const std::string& vertexPath, const std::string& fragmentPath)
    {
        m_shader.Load(vertexPath, fragmentPath);
        return true;
    }

    bool Skybox::LoadFromConfig(const SkyboxConfig& config)
    {
        if (!m_isInitialized)
        {
            Core::Logger::GetInstance().Error("Skybox not initialized. Call Initialize() first.");
            return false;
        }

        if (config.faceFilenames.size() != 6)
        {
            Core::Logger::GetInstance().Error("SkyboxConfig must have exactly 6 face filenames");
            return false;
        }

        // 使用配置中的文件路径（已经是OpenGL顺序）
        return Load(
            config.faceFilenames[0], // right
            config.faceFilenames[1], // left
            config.faceFilenames[2], // top
            config.faceFilenames[3], // bottom
            config.faceFilenames[4], // back
            config.faceFilenames[5]  // front
        );
    }

    void Skybox::Render(const glm::mat4& projection, const glm::mat4& view)
    {
        if (!m_isInitialized || m_textureID == 0)
        {
            Core::Logger::GetInstance().Error("Skybox not initialized or loaded");
            return;
        }

        // 深度测试设置为GL_LEQUAL，确保天空盒在最远处
        glDepthFunc(GL_LEQUAL);

        // 禁用深度写入，避免天空盒遮挡其他物体
        glDepthMask(GL_FALSE);

        m_shader.Use();

        // 移除视图矩阵的平移分量，使天空盒始终围绕摄像机
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));

        // 应用旋转
        if (std::abs(m_rotation) > 0.001f)
        {
            skyboxView = glm::rotate(skyboxView, glm::radians(m_rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        }

        m_shader.SetMat4("projection", projection);
        m_shader.SetMat4("view", skyboxView);

        // ⭐ 绑定天空盒纹理到单元15（TextureUnit::SKYBOX_CUBEMAP）
        glBindVertexArray(m_VAO);
        glActiveTexture(GL_TEXTURE15);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);
        m_shader.SetInt("skybox", 15);

        // 绘制天空盒
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 恢复深度写入
        glDepthMask(GL_TRUE);

        // 恢复默认深度测试
        glDepthFunc(GL_LESS);

        glBindVertexArray(0);
    }

    void Skybox::BindTexture(unsigned int textureUnit) const
    {
        if (m_textureID != 0)
        {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);
        }
    }

    void Skybox::CreateCubeMesh()
    {
        // 天空盒立方体顶点数据
        float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

        // 位置属性
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glBindVertexArray(0);
    }

} // namespace Renderer
