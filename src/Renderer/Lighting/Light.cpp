#include "Renderer/Lighting/Light.hpp"
#include "Renderer/Resources/Shader.hpp"
#include "Core/Logger.hpp"
#include "Core/GLM.hpp"
#include <sstream>
#include <cmath>

namespace Renderer
{
    namespace Lighting
    {

        // ========================================
        // 静态成员定义
        // ========================================

        // ✅ 性能优化（2026-01-02）：线程局部格式化缓冲区定义
        thread_local Light::UniformFormatter Light::s_formatter;

        // ========================================
        // Light 基类实现
        // ========================================

        Light::Light(
            const glm::vec3 &color,
            float intensity,
            float ambient,
            float diffuse,
            float specular)
            : m_color(color),
              m_intensity(intensity),
              m_enabled(true),
              m_ambient(ambient),
              m_diffuse(diffuse),
              m_specular(specular)
        {
        }

        // ========================================
        // DirectionalLight 实现
        // ========================================

        DirectionalLight::DirectionalLight(
            const glm::vec3 &direction,
            const glm::vec3 &color,
            float intensity,
            float ambient,
            float diffuse,
            float specular)
            : Light(color, intensity, ambient, diffuse, specular),
              m_direction(direction)
        {
        }

        void DirectionalLight::ApplyToShader(Shader &shader, int index) const
        {
            // ✅ 性能优化（2026-01-02）：使用栈上格式化缓冲区，避免堆分配
            // 修复前：每帧 48 光源 × 10 uniform = 480 次字符串分配（~10 KB）
            // 修复后：零堆分配，直接使用 snprintf 格式化到栈上 64 字节缓冲区

            // 修复：禁用时设置零值而非跳过，避免未初始化的uniform数据
            if (m_enabled)
            {
                shader.SetVec3(s_formatter.formatUniform("dirLights", index, "direction"), m_direction);
                shader.SetVec3(s_formatter.formatUniform("dirLights", index, "color"), m_color * m_intensity);
                shader.SetFloat(s_formatter.formatUniform("dirLights", index, "ambient"), m_ambient);
                shader.SetFloat(s_formatter.formatUniform("dirLights", index, "diffuse"), m_diffuse);
                shader.SetFloat(s_formatter.formatUniform("dirLights", index, "specular"), m_specular);
            }
            else
            {
                // 禁用的光源设置零值，确保着色器不会读取随机数据
                shader.SetVec3(s_formatter.formatUniform("dirLights", index, "direction"), glm::vec3(0.0f, -1.0f, 0.0f));
                shader.SetVec3(s_formatter.formatUniform("dirLights", index, "color"), glm::vec3(0.0f));
                shader.SetFloat(s_formatter.formatUniform("dirLights", index, "ambient"), 0.0f);
                shader.SetFloat(s_formatter.formatUniform("dirLights", index, "diffuse"), 0.0f);
                shader.SetFloat(s_formatter.formatUniform("dirLights", index, "specular"), 0.0f);
            }
        }

        std::string DirectionalLight::GetDescription() const
        {
            std::ostringstream oss;
            oss << "DirectionalLight [Direction: ("
                << m_direction.x << ", " << m_direction.y << ", " << m_direction.z
                << "), Color: ("
                << m_color.r << ", " << m_color.g << ", " << m_color.b
                << "), Intensity: " << m_intensity
                << ", Enabled: " << (m_enabled ? "Yes" : "No") << "]";
            return oss.str();
        }

        // ========================================
        // LightWithAttenuation 实现 ⭐ NEW
        // ========================================

        float LightWithAttenuation::GetEffectiveRange() const
        {
            // 计算光照衰减到5%以下的距离（近似值）
            float min = 0.0f;
            float max = 100.0f;
            float threshold = 0.05f;

            for (int i = 0; i < 10; ++i)
            {
                float mid = (min + max) / 2.0f;
                float attenuation = 1.0f / (m_attenuation.constant +
                                            m_attenuation.linear * mid +
                                            m_attenuation.quadratic * mid * mid);

                if (attenuation > threshold)
                    min = mid;
                else
                    max = mid;
            }

            return (min + max) / 2.0f;
        }

        // ========================================
        // PointLight 实现
        // ========================================

        PointLight::PointLight(
            const glm::vec3 &position,
            const glm::vec3 &color,
            float intensity,
            float ambient,
            float diffuse,
            float specular,
            const Attenuation &attenuation)
            : LightWithAttenuation(color, intensity, ambient, diffuse, specular, position, attenuation)
        {
        }

        void PointLight::ApplyToShader(Shader &shader, int index) const
        {
            // ✅ 性能优化（2026-01-02）：使用栈上格式化缓冲区，避免堆分配
            // 修复前：每帧 48 光源 × 10 uniform = 480 次字符串分配（~10 KB）
            // 修复后：零堆分配，直接使用 snprintf 格式化到栈上 64 字节缓冲区

            // 修复：禁用时设置零值而非跳过，避免未初始化的uniform数据
            if (m_enabled)
            {
                shader.SetVec3(s_formatter.formatUniform("pointLights", index, "position"), m_position);
                shader.SetVec3(s_formatter.formatUniform("pointLights", index, "color"), m_color * m_intensity);
                shader.SetFloat(s_formatter.formatUniform("pointLights", index, "ambient"), m_ambient);
                shader.SetFloat(s_formatter.formatUniform("pointLights", index, "diffuse"), m_diffuse);
                shader.SetFloat(s_formatter.formatUniform("pointLights", index, "specular"), m_specular);
                shader.SetFloat(s_formatter.formatUniform("pointLights", index, "constant"), m_attenuation.constant);
                shader.SetFloat(s_formatter.formatUniform("pointLights", index, "linear"), m_attenuation.linear);
                shader.SetFloat(s_formatter.formatUniform("pointLights", index, "quadratic"), m_attenuation.quadratic);
            }
            else
            {
                // 禁用的光源设置零值，确保着色器不会读取随机数据
                shader.SetVec3(s_formatter.formatUniform("pointLights", index, "position"), glm::vec3(0.0f));
                shader.SetVec3(s_formatter.formatUniform("pointLights", index, "color"), glm::vec3(0.0f));
                shader.SetFloat(s_formatter.formatUniform("pointLights", index, "ambient"), 0.0f);
                shader.SetFloat(s_formatter.formatUniform("pointLights", index, "diffuse"), 0.0f);
                shader.SetFloat(s_formatter.formatUniform("pointLights", index, "specular"), 0.0f);
                shader.SetFloat(s_formatter.formatUniform("pointLights", index, "constant"), 1.0f);
                shader.SetFloat(s_formatter.formatUniform("pointLights", index, "linear"), 0.0f);
                shader.SetFloat(s_formatter.formatUniform("pointLights", index, "quadratic"), 0.0f);
            }
        }

        std::string PointLight::GetDescription() const
        {
            std::ostringstream oss;
            oss << "PointLight [Position: ("
                << m_position.x << ", " << m_position.y << ", " << m_position.z
                << "), Color: ("
                << m_color.r << ", " << m_color.g << ", " << m_color.b
                << "), Intensity: " << m_intensity
                << ", Range: ~" << GetEffectiveRange() << "m"
                << ", Enabled: " << (m_enabled ? "Yes" : "No") << "]";
            return oss.str();
        }

        /**
         * PointLight::GetEffectiveRange - ⭐ 重写虚函数
         *
         * 纯距离衰减计算
         */
        float PointLight::GetEffectiveRange() const
        {
            // 调用基类方法（纯距离衰减）
            return LightWithAttenuation::GetEffectiveRange();
        }

        // ========================================
        // SpotLight 实现（⭐ 重构：继承 LightWithAttenuation）
        // ========================================

        SpotLight::SpotLight(
            const glm::vec3 &position,
            const glm::vec3 &direction,
            const glm::vec3 &color,
            float intensity,
            float ambient,
            float diffuse,
            float specular,
            const LightWithAttenuation::Attenuation &attenuation,
            float cutOff,
            float outerCutOff)
            : LightWithAttenuation(color, intensity, ambient, diffuse, specular, position, attenuation),
              m_direction(direction),
              m_cutOff(cutOff),
              m_outerCutOff(outerCutOff)
        {
        }

        void SpotLight::ApplyToShader(Shader &shader, int index) const
        {
            // ✅ 性能优化（2026-01-02）：使用栈上格式化缓冲区，避免堆分配
            // 修复前：每帧 48 光源 × 10 uniform = 480 次字符串分配（~10 KB）
            // 修复后：零堆分配，直接使用 snprintf 格式化到栈上 64 字节缓冲区

            // 修复：禁用时设置零值而非跳过，避免未初始化的uniform数据
            if (m_enabled)
            {
                shader.SetVec3(s_formatter.formatUniform("spotLights", index, "position"), m_position);  // 继承自 LightWithAttenuation
                shader.SetVec3(s_formatter.formatUniform("spotLights", index, "direction"), m_direction);
                shader.SetVec3(s_formatter.formatUniform("spotLights", index, "color"), m_color * m_intensity);
                shader.SetFloat(s_formatter.formatUniform("spotLights", index, "ambient"), m_ambient);
                shader.SetFloat(s_formatter.formatUniform("spotLights", index, "diffuse"), m_diffuse);
                shader.SetFloat(s_formatter.formatUniform("spotLights", index, "specular"), m_specular);
                shader.SetFloat(s_formatter.formatUniform("spotLights", index, "constant"), m_attenuation.constant);  // 继承自 LightWithAttenuation
                shader.SetFloat(s_formatter.formatUniform("spotLights", index, "linear"), m_attenuation.linear);
                shader.SetFloat(s_formatter.formatUniform("spotLights", index, "quadratic"), m_attenuation.quadratic);
                shader.SetFloat(s_formatter.formatUniform("spotLights", index, "cutOff"), m_cutOff);
                shader.SetFloat(s_formatter.formatUniform("spotLights", index, "outerCutOff"), m_outerCutOff);
            }
            else
            {
                // 禁用的光源设置零值，确保着色器不会读取随机数据
                shader.SetVec3(s_formatter.formatUniform("spotLights", index, "position"), glm::vec3(0.0f));
                shader.SetVec3(s_formatter.formatUniform("spotLights", index, "direction"), glm::vec3(0.0f, -1.0f, 0.0f));
                shader.SetVec3(s_formatter.formatUniform("spotLights", index, "color"), glm::vec3(0.0f));
                shader.SetFloat(s_formatter.formatUniform("spotLights", index, "ambient"), 0.0f);
                shader.SetFloat(s_formatter.formatUniform("spotLights", index, "diffuse"), 0.0f);
                shader.SetFloat(s_formatter.formatUniform("spotLights", index, "specular"), 0.0f);
                shader.SetFloat(s_formatter.formatUniform("spotLights", index, "constant"), 1.0f);
                shader.SetFloat(s_formatter.formatUniform("spotLights", index, "linear"), 0.0f);
                shader.SetFloat(s_formatter.formatUniform("spotLights", index, "quadratic"), 0.0f);
                shader.SetFloat(s_formatter.formatUniform("spotLights", index, "cutOff"), glm::cos(glm::radians(0.0f)));
                shader.SetFloat(s_formatter.formatUniform("spotLights", index, "outerCutOff"), glm::cos(glm::radians(0.0f)));
            }
        }

        std::string SpotLight::GetDescription() const
        {
            std::ostringstream oss;
            oss << "SpotLight [Position: ("
                << m_position.x << ", " << m_position.y << ", " << m_position.z  // 继承自 LightWithAttenuation
                << "), Direction: ("
                << m_direction.x << ", " << m_direction.y << ", " << m_direction.z
                << "), Color: ("
                << m_color.r << ", " << m_color.g << ", " << m_color.b
                << "), Intensity: " << m_intensity
                << ", CutOff: " << GetCutOffDegrees() << "°"
                << ", Range: ~" << GetEffectiveRange() << "m"  // ⭐ 虚函数调用
                << ", Enabled: " << (m_enabled ? "Yes" : "No") << "]";
            return oss.str();
        }

        /**
         * SpotLight::GetEffectiveRange - ⭐ 重写虚函数
         *
         * 考虑聚光灯的角度衰减
         */
        float SpotLight::GetEffectiveRange() const
        {
            // 首先计算基于距离衰减的有效范围（调用基类方法）
            float distanceRange = LightWithAttenuation::GetEffectiveRange();

            // 然后考虑角度衰减：聚光灯的有效范围受锥角限制
            // 内锥角越小，有效距离越短
            float angleFactor = glm::cos(m_cutOff);  // 0~1之间，越小越窄

            // 综合距离和角度因素（简化模型）
            return distanceRange * angleFactor;
        }

    } // namespace Lighting
} // namespace Renderer
