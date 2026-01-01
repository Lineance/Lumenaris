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
            std::string prefix = "dirLights[" + std::to_string(index) + "].";

            // 修复：禁用时设置零值而非跳过，避免未初始化的uniform数据
            if (m_enabled)
            {
                shader.SetVec3(prefix + "direction", m_direction);
                shader.SetVec3(prefix + "color", m_color * m_intensity);
                shader.SetFloat(prefix + "ambient", m_ambient);
                shader.SetFloat(prefix + "diffuse", m_diffuse);
                shader.SetFloat(prefix + "specular", m_specular);
            }
            else
            {
                // 禁用的光源设置零值，确保着色器不会读取随机数据
                shader.SetVec3(prefix + "direction", glm::vec3(0.0f, -1.0f, 0.0f));
                shader.SetVec3(prefix + "color", glm::vec3(0.0f));
                shader.SetFloat(prefix + "ambient", 0.0f);
                shader.SetFloat(prefix + "diffuse", 0.0f);
                shader.SetFloat(prefix + "specular", 0.0f);
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
            std::string prefix = "pointLights[" + std::to_string(index) + "].";

            // 修复：禁用时设置零值而非跳过，避免未初始化的uniform数据
            if (m_enabled)
            {
                shader.SetVec3(prefix + "position", m_position);
                shader.SetVec3(prefix + "color", m_color * m_intensity);
                shader.SetFloat(prefix + "ambient", m_ambient);
                shader.SetFloat(prefix + "diffuse", m_diffuse);
                shader.SetFloat(prefix + "specular", m_specular);
                shader.SetFloat(prefix + "constant", m_attenuation.constant);
                shader.SetFloat(prefix + "linear", m_attenuation.linear);
                shader.SetFloat(prefix + "quadratic", m_attenuation.quadratic);
            }
            else
            {
                // 禁用的光源设置零值，确保着色器不会读取随机数据
                shader.SetVec3(prefix + "position", glm::vec3(0.0f));
                shader.SetVec3(prefix + "color", glm::vec3(0.0f));
                shader.SetFloat(prefix + "ambient", 0.0f);
                shader.SetFloat(prefix + "diffuse", 0.0f);
                shader.SetFloat(prefix + "specular", 0.0f);
                shader.SetFloat(prefix + "constant", 1.0f);
                shader.SetFloat(prefix + "linear", 0.0f);
                shader.SetFloat(prefix + "quadratic", 0.0f);
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
            std::string prefix = "spotLights[" + std::to_string(index) + "].";

            // 修复：禁用时设置零值而非跳过，避免未初始化的uniform数据
            if (m_enabled)
            {
                shader.SetVec3(prefix + "position", m_position);  // 继承自 LightWithAttenuation
                shader.SetVec3(prefix + "direction", m_direction);
                shader.SetVec3(prefix + "color", m_color * m_intensity);
                shader.SetFloat(prefix + "ambient", m_ambient);
                shader.SetFloat(prefix + "diffuse", m_diffuse);
                shader.SetFloat(prefix + "specular", m_specular);
                shader.SetFloat(prefix + "constant", m_attenuation.constant);  // 继承自 LightWithAttenuation
                shader.SetFloat(prefix + "linear", m_attenuation.linear);
                shader.SetFloat(prefix + "quadratic", m_attenuation.quadratic);
                shader.SetFloat(prefix + "cutOff", m_cutOff);
                shader.SetFloat(prefix + "outerCutOff", m_outerCutOff);
            }
            else
            {
                // 禁用的光源设置零值，确保着色器不会读取随机数据
                shader.SetVec3(prefix + "position", glm::vec3(0.0f));
                shader.SetVec3(prefix + "direction", glm::vec3(0.0f, -1.0f, 0.0f));
                shader.SetVec3(prefix + "color", glm::vec3(0.0f));
                shader.SetFloat(prefix + "ambient", 0.0f);
                shader.SetFloat(prefix + "diffuse", 0.0f);
                shader.SetFloat(prefix + "specular", 0.0f);
                shader.SetFloat(prefix + "constant", 1.0f);
                shader.SetFloat(prefix + "linear", 0.0f);
                shader.SetFloat(prefix + "quadratic", 0.0f);
                shader.SetFloat(prefix + "cutOff", glm::cos(glm::radians(0.0f)));
                shader.SetFloat(prefix + "outerCutOff", glm::cos(glm::radians(0.0f)));
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
