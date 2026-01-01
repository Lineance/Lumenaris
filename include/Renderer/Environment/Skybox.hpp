#pragma once

#include "Renderer/Resources/Shader.hpp"
#include "Renderer/Environment/SkyboxLoader.hpp"
#include "Core/GLM.hpp"
#include <string>

namespace Renderer
{

    /**
     * Skybox 类 - 天空盒渲染器
     *
     * 实现立方体天空盒的渲染，支持：
     * - 6个面的环境贴图
     * - 无深度写入的正确渲染顺序
     * - 视差移除（天空盒跟随摄像机）
     */
    class Skybox
    {
    public:
        Skybox();
        ~Skybox();

        // 禁止拷贝，允许移动
        Skybox(const Skybox&) = delete;
        Skybox& operator=(const Skybox&) = delete;
        Skybox(Skybox&& other) noexcept;
        Skybox& operator=(Skybox&& other) noexcept;

        /**
         * 初始化天空盒
         * @return 初始化是否成功
         */
        bool Initialize();

        /**
         * 从6个纹理文件加载天空盒
         */
        bool Load(
            const std::string& right,
            const std::string& left,
            const std::string& top,
            const std::string& bottom,
            const std::string& back,
            const std::string& front
        );

        /**
         * 从配置加载天空盒
         * @param config 天空盒配置（支持多种约定）
         */
        bool LoadFromConfig(const SkyboxConfig& config);

        /**
         * 加载着色器
         */
        bool LoadShaders(const std::string& vertexPath, const std::string& fragmentPath);

        /**
         * 渲染天空盒
         * @param projection 投影矩阵
         * @param view 视图矩阵（移除了平移分量）
         */
        void Render(const glm::mat4& projection, const glm::mat4& view);

        /**
         * 绑定天空盒纹理到指定的纹理单元
         */
        void BindTexture(unsigned int textureUnit = 0) const;

        /**
         * 获取天空盒纹理ID
         */
        unsigned int GetTextureID() const { return m_textureID; }

        /**
         * 检查天空盒是否已加载
         */
        bool IsLoaded() const { return m_textureID != 0 && m_isInitialized; }

        /**
         * 设置旋转角度
         */
        void SetRotation(float rotationDegrees) { m_rotation = rotationDegrees; }

        /**
         * 获取旋转角度
         */
        float GetRotation() const { return m_rotation; }

    private:
        unsigned int m_textureID;  // 立方体贴图ID
        Shader m_shader;
        unsigned int m_VAO, m_VBO;
        bool m_isInitialized;
        float m_rotation;  // 天空盒旋转角度（度）

        /**
         * 创建天空盒的立方体网格
         */
        void CreateCubeMesh();
    };

} // namespace Renderer
