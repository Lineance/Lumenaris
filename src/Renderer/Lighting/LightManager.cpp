#include "Renderer/Lighting/LightManager.hpp"
#include "Renderer/Resources/Shader.hpp"
#include "Core/Logger.hpp"
#include "Core/GLM.hpp"
#include <sstream>

namespace Renderer
{
    namespace Lighting
    {

        // ========================================
        // 单例实现
        // ========================================

        LightManager &LightManager::GetInstance()
        {
            static LightManager instance;
            return instance;
        }

        // ========================================
        // 添加光源
        // ========================================

        int LightManager::AddDirectionalLight(const DirectionalLightPtr &light)
        {
            if (!light)
            {
                Core::Logger::GetInstance().Warning("LightManager: Attempted to add null directional light");
                return -1;
            }

            if (m_directionalLights.size() >= MAX_DIRECTIONAL_LIGHTS)
            {
                Core::Logger::GetInstance().Warning("LightManager: Maximum directional lights reached (" +
                                                    std::to_string(MAX_DIRECTIONAL_LIGHTS) + ")");
                return -1;
            }

            m_directionalLights.push_back(light);
            Core::Logger::GetInstance().Info("LightManager: Added directional light [" + light->GetDescription() + "]");

            return static_cast<int>(m_directionalLights.size()) - 1;
        }

        int LightManager::AddPointLight(const PointLightPtr &light)
        {
            if (!light)
            {
                Core::Logger::GetInstance().Warning("LightManager: Attempted to add null point light");
                return -1;
            }

            if (m_pointLights.size() >= MAX_POINT_LIGHTS)
            {
                Core::Logger::GetInstance().Warning("LightManager: Maximum point lights reached (" +
                                                    std::to_string(MAX_POINT_LIGHTS) + ")");
                return -1;
            }

            m_pointLights.push_back(light);
            Core::Logger::GetInstance().Info("LightManager: Added point light [" + light->GetDescription() + "]");

            return static_cast<int>(m_pointLights.size()) - 1;
        }

        int LightManager::AddSpotLight(const SpotLightPtr &light)
        {
            if (!light)
            {
                Core::Logger::GetInstance().Warning("LightManager: Attempted to add null spot light");
                return -1;
            }

            if (m_spotLights.size() >= MAX_SPOT_LIGHTS)
            {
                Core::Logger::GetInstance().Warning("LightManager: Maximum spot lights reached (" +
                                                    std::to_string(MAX_SPOT_LIGHTS) + ")");
                return -1;
            }

            m_spotLights.push_back(light);
            Core::Logger::GetInstance().Info("LightManager: Added spot light [" + light->GetDescription() + "]");

            return static_cast<int>(m_spotLights.size()) - 1;
        }

        // ========================================
        // 移除光源
        // ========================================

        bool LightManager::RemoveDirectionalLight(int index)
        {
            if (index < 0 || index >= static_cast<int>(m_directionalLights.size()))
            {
                Core::Logger::GetInstance().Warning("LightManager: Invalid directional light index: " +
                                                    std::to_string(index));
                return false;
            }

            Core::Logger::GetInstance().Info("LightManager: Removed directional light at index " +
                                            std::to_string(index));
            m_directionalLights.erase(m_directionalLights.begin() + index);
            return true;
        }

        bool LightManager::RemovePointLight(int index)
        {
            if (index < 0 || index >= static_cast<int>(m_pointLights.size()))
            {
                Core::Logger::GetInstance().Warning("LightManager: Invalid point light index: " +
                                                    std::to_string(index));
                return false;
            }

            Core::Logger::GetInstance().Info("LightManager: Removed point light at index " +
                                            std::to_string(index));
            m_pointLights.erase(m_pointLights.begin() + index);
            return true;
        }

        bool LightManager::RemoveSpotLight(int index)
        {
            if (index < 0 || index >= static_cast<int>(m_spotLights.size()))
            {
                Core::Logger::GetInstance().Warning("LightManager: Invalid spot light index: " +
                                                    std::to_string(index));
                return false;
            }

            Core::Logger::GetInstance().Info("LightManager: Removed spot light at index " +
                                            std::to_string(index));
            m_spotLights.erase(m_spotLights.begin() + index);
            return true;
        }

        void LightManager::ClearAll()
        {
            int total = GetTotalLightCount();
            m_directionalLights.clear();
            m_pointLights.clear();
            m_spotLights.clear();
            Core::Logger::GetInstance().Info("LightManager: Cleared all lights (" +
                                            std::to_string(total) + " lights removed)");
        }

        // ========================================
        // 获取光源
        // ========================================

        DirectionalLightPtr LightManager::GetDirectionalLight(int index)
        {
            if (index < 0 || index >= static_cast<int>(m_directionalLights.size()))
                return nullptr;
            return m_directionalLights[index];
        }

        PointLightPtr LightManager::GetPointLight(int index)
        {
            if (index < 0 || index >= static_cast<int>(m_pointLights.size()))
                return nullptr;
            return m_pointLights[index];
        }

        SpotLightPtr LightManager::GetSpotLight(int index)
        {
            if (index < 0 || index >= static_cast<int>(m_spotLights.size()))
                return nullptr;
            return m_spotLights[index];
        }

        // ========================================
        // 应用光源到着色器
        // ========================================

        void LightManager::ApplyToShader(Shader &shader) const
        {
            // 设置各类光源的数量
            shader.SetInt("nrDirLights", static_cast<int>(m_directionalLights.size()));
            shader.SetInt("nrPointLights", static_cast<int>(m_pointLights.size()));
            shader.SetInt("nrSpotLights", static_cast<int>(m_spotLights.size()));

            // 应用平行光
            for (size_t i = 0; i < m_directionalLights.size(); ++i)
            {
                m_directionalLights[i]->ApplyToShader(shader, static_cast<int>(i));
            }

            // 应用点光源
            for (size_t i = 0; i < m_pointLights.size(); ++i)
            {
                m_pointLights[i]->ApplyToShader(shader, static_cast<int>(i));
            }

            // 应用聚光灯
            for (size_t i = 0; i < m_spotLights.size(); ++i)
            {
                m_spotLights[i]->ApplyToShader(shader, static_cast<int>(i));
            }
        }

        // ========================================
        // 统计和调试信息
        // ========================================

        std::string LightManager::GetStatistics() const
        {
            std::ostringstream oss;
            oss << "LightManager Statistics:\n";
            oss << "  Directional Lights: " << m_directionalLights.size() << "/" << MAX_DIRECTIONAL_LIGHTS << "\n";
            oss << "  Point Lights: " << m_pointLights.size() << "/" << MAX_POINT_LIGHTS << "\n";
            oss << "  Spot Lights: " << m_spotLights.size() << "/" << MAX_SPOT_LIGHTS << "\n";
            oss << "  Total Lights: " << GetTotalLightCount();

            return oss.str();
        }

        void LightManager::PrintAllLights() const
        {
            Core::Logger::GetInstance().Info("========================================");
            Core::Logger::GetInstance().Info(GetStatistics());
            Core::Logger::GetInstance().Info("========================================");

            for (size_t i = 0; i < m_directionalLights.size(); ++i)
            {
                Core::Logger::GetInstance().Info("  [" + std::to_string(i) + "] " +
                                                 m_directionalLights[i]->GetDescription());
            }

            for (size_t i = 0; i < m_pointLights.size(); ++i)
            {
                Core::Logger::GetInstance().Info("  [" + std::to_string(i) + "] " +
                                                 m_pointLights[i]->GetDescription());
            }

            for (size_t i = 0; i < m_spotLights.size(); ++i)
            {
                Core::Logger::GetInstance().Info("  [" + std::to_string(i) + "] " +
                                                 m_spotLights[i]->GetDescription());
            }

            Core::Logger::GetInstance().Info("========================================");
        }

    } // namespace Lighting
} // namespace Renderer
