#pragma once

#include "Renderer/Resources/Shader.hpp"
#include "Core/GLM.hpp"
#include <memory>

namespace Renderer
{

    /**
     * AmbientLighting 类 - 轻量级环境光照系统
     *
     * 功能：
     * - 从天空盒采样环境光（替代固定颜色的环境光）
     * - 与Phong光照系统完美兼容
     * - 不需要HDR或PBR，使用普通纹理即可
     * - 支持半球光照（hemisphere lighting）
     *
     * 使用场景：
     * - 让物体的暗部显示天空盒的颜色
     * - 创建更自然的环境光过渡
     * - 不增加太多性能开销
     */
    class AmbientLighting
    {
    public:
        AmbientLighting();
        ~AmbientLighting();

        // 禁止拷贝，允许移动
        AmbientLighting(const AmbientLighting&) = delete;
        AmbientLighting& operator=(const AmbientLighting&) = delete;
        AmbientLighting(AmbientLighting&& other) noexcept;
        AmbientLighting& operator=(AmbientLighting&& other) noexcept;

        /**
         * 初始化环境光照系统
         */
        bool Initialize();

        /**
         * 从已加载的天空盒创建环境光照
         * @param skyboxTextureID 天空盒纹理ID
         * @param intensity 环境光强度 (0.0 - 1.0)
         */
        bool LoadFromSkybox(unsigned int skyboxTextureID, float intensity = 0.3f);

        /**
         * 应用环境光设置到着色器
         * @param shader 目标着色器
         */
        void ApplyToShader(Shader& shader) const;

        /**
         * 设置环境光强度
         */
        void SetIntensity(float intensity) { m_intensity = intensity; }

        /**
         * 获取环境光强度
         */
        float GetIntensity() const { return m_intensity; }

        /**
         * 启用/禁用天空盒环境光
         */
        void SetEnabled(bool enabled) { m_enabled = enabled; }
        bool IsEnabled() const { return m_enabled; }

        /**
         * 设置环境光模式
         */
        enum class Mode
        {
            SOLID_COLOR,    // 固定颜色（传统Phong）
            SKYBOX_SAMPLE,  // 从天空盒采样
            HEMISPHERE       // 半球光照（天空/地面渐变）
        };

        void SetMode(Mode mode) { m_mode = mode; }
        Mode GetMode() const { return m_mode; }

        /**
         * 设置半球光照的颜色
         * @param skyColor 天空颜色
         * @param groundColor 地面颜色
         */
        void SetHemisphereColors(const glm::vec3& skyColor, const glm::vec3& groundColor)
        {
            m_skyColor = skyColor;
            m_groundColor = groundColor;
        }

        /**
         * 检查是否已加载
         */
        bool IsLoaded() const { return m_skyboxTextureID != 0; }

    private:
        void BindTexture(unsigned int textureUnit) const;  // 绑定天空盒纹理

        unsigned int m_skyboxTextureID;  // 天空盒纹理ID（不拥有所有权）
        float m_intensity;               // 环境光强度
        bool m_enabled;                  // 是否启用
        Mode m_mode;                     // 环境光模式
        glm::vec3 m_skyColor;            // 半球光照天空颜色
        glm::vec3 m_groundColor;         // 半球光照地面颜色
    };

} // namespace Renderer
