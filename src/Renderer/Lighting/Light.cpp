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
            if (!m_enabled)
                return;

            std::string prefix = "dirLights[" + std::to_string(index) + "].";

            shader.SetVec3(prefix + "direction", m_direction);
            shader.SetVec3(prefix + "color", m_color * m_intensity);
            shader.SetFloat(prefix + "ambient", m_ambient);
            shader.SetFloat(prefix + "diffuse", m_diffuse);
            shader.SetFloat(prefix + "specular", m_specular);
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
            : Light(color, intensity, ambient, diffuse, specular),
              m_position(position),
              m_attenuation(attenuation)
        {
        }

        void PointLight::ApplyToShader(Shader &shader, int index) const
        {
            if (!m_enabled)
                return;

            std::string prefix = "pointLights[" + std::to_string(index) + "].";

            shader.SetVec3(prefix + "position", m_position);
            shader.SetVec3(prefix + "color", m_color * m_intensity);
            shader.SetFloat(prefix + "ambient", m_ambient);
            shader.SetFloat(prefix + "diffuse", m_diffuse);
            shader.SetFloat(prefix + "specular", m_specular);
            shader.SetFloat(prefix + "constant", m_attenuation.constant);
            shader.SetFloat(prefix + "linear", m_attenuation.linear);
            shader.SetFloat(prefix + "quadratic", m_attenuation.quadratic);
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

        float PointLight::GetEffectiveRange() const
        {
            // 计算光照衰减到5%以下的距离（近似值）
            // 使用二分法求解: 1 / (constant + linear*d + quadratic*d^2) = 0.05
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
        // SpotLight 实现
        // ========================================

        SpotLight::SpotLight(
            const glm::vec3 &position,
            const glm::vec3 &direction,
            const glm::vec3 &color,
            float intensity,
            float ambient,
            float diffuse,
            float specular,
            const PointLight::Attenuation &attenuation,
            float cutOff,
            float outerCutOff)
            : PointLight(position, color, intensity, ambient, diffuse, specular, attenuation),
              m_direction(direction),
              m_cutOff(cutOff),
              m_outerCutOff(outerCutOff)
        {
        }

        void SpotLight::ApplyToShader(Shader &shader, int index) const
        {
            if (!m_enabled)
                return;

            std::string prefix = "spotLights[" + std::to_string(index) + "].";

            shader.SetVec3(prefix + "position", GetPosition());
            shader.SetVec3(prefix + "direction", m_direction);
            shader.SetVec3(prefix + "color", m_color * m_intensity);
            shader.SetFloat(prefix + "ambient", m_ambient);
            shader.SetFloat(prefix + "diffuse", m_diffuse);
            shader.SetFloat(prefix + "specular", m_specular);
            shader.SetFloat(prefix + "constant", GetAttenuation().constant);
            shader.SetFloat(prefix + "linear", GetAttenuation().linear);
            shader.SetFloat(prefix + "quadratic", GetAttenuation().quadratic);
            shader.SetFloat(prefix + "cutOff", m_cutOff);
            shader.SetFloat(prefix + "outerCutOff", m_outerCutOff);
        }

        std::string SpotLight::GetDescription() const
        {
            std::ostringstream oss;
            oss << "SpotLight [Position: ("
                << GetPosition().x << ", " << GetPosition().y << ", " << GetPosition().z
                << "), Direction: ("
                << m_direction.x << ", " << m_direction.y << ", " << m_direction.z
                << "), Color: ("
                << m_color.r << ", " << m_color.g << ", " << m_color.b
                << "), Intensity: " << m_intensity
                << ", CutOff: " << GetCutOffDegrees() << "°"
                << ", Range: ~" << GetEffectiveRange() << "m"
                << ", Enabled: " << (m_enabled ? "Yes" : "No") << "]";
            return oss.str();
        }

    } // namespace Lighting
} // namespace Renderer
