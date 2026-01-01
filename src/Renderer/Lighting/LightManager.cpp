#include "Renderer/Lighting/LightManager.hpp"
#include "Renderer/Resources/Shader.hpp"
#include "Core/Logger.hpp"
#include "Core/GLM.hpp"
#include <sstream>
#include <shared_mutex>

namespace Renderer
{
    namespace Lighting
    {

        // ========================================
        // 构造函数和析构函数
        // ========================================

        // LightManager 现在是可实例化类，通过 RenderContext 创建
        // 不再有单例模式

        // ========================================
        // 添加光源（返回LightHandle）
        // ========================================

        LightHandle LightManager::AddDirectionalLight(const DirectionalLightPtr &light)
        {
            if (!light)
            {
                Core::Logger::GetInstance().Warning("LightManager: Attempted to add null directional light");
                return LightHandle();
            }

            std::unique_lock<std::shared_mutex> lock(m_mutex);

            if (m_directionalLights.size() >= MAX_DIRECTIONAL_LIGHTS)
            {
                Core::Logger::GetInstance().Warning("LightManager: Maximum directional lights reached (" +
                                                    std::to_string(MAX_DIRECTIONAL_LIGHTS) + ")");
                return LightHandle();
            }

            size_t id = m_nextDirectionalId++;
            m_directionalLights[id] = LightEntry{light, 1};

            Core::Logger::GetInstance().Info("LightManager: Added directional light [" + light->GetDescription() + "] with ID " + std::to_string(id));

            return LightHandle(id, 1, LightType::DIRECTIONAL);
        }

        LightHandle LightManager::AddPointLight(const PointLightPtr &light)
        {
            if (!light)
            {
                Core::Logger::GetInstance().Warning("LightManager: Attempted to add null point light");
                return LightHandle();
            }

            std::unique_lock<std::shared_mutex> lock(m_mutex);

            if (m_pointLights.size() >= MAX_POINT_LIGHTS)
            {
                Core::Logger::GetInstance().Warning("LightManager: Maximum point lights reached (" +
                                                    std::to_string(MAX_POINT_LIGHTS) + ")");
                return LightHandle();
            }

            size_t id = m_nextPointId++;
            m_pointLights[id] = LightEntry{light, 1};

            Core::Logger::GetInstance().Info("LightManager: Added point light [" + light->GetDescription() + "] with ID " + std::to_string(id));

            return LightHandle(id, 1, LightType::POINT);
        }

        LightHandle LightManager::AddSpotLight(const SpotLightPtr &light)
        {
            if (!light)
            {
                Core::Logger::GetInstance().Warning("LightManager: Attempted to add null spot light");
                return LightHandle();
            }

            std::unique_lock<std::shared_mutex> lock(m_mutex);

            if (m_spotLights.size() >= MAX_SPOT_LIGHTS)
            {
                Core::Logger::GetInstance().Warning("LightManager: Maximum spot lights reached (" +
                                                    std::to_string(MAX_SPOT_LIGHTS) + ")");
                return LightHandle();
            }

            size_t id = m_nextSpotId++;
            m_spotLights[id] = LightEntry{light, 1};

            Core::Logger::GetInstance().Info("LightManager: Added spot light [" + light->GetDescription() + "] with ID " + std::to_string(id));

            return LightHandle(id, 1, LightType::SPOT);
        }

        // ========================================
        // 移除光源（使用LightHandle）
        // ========================================

        bool LightManager::RemoveDirectionalLight(const LightHandle &handle)
        {
            if (!handle.IsValid() || handle.GetType() != LightType::DIRECTIONAL)
            {
                Core::Logger::GetInstance().Warning("LightManager: Invalid directional light handle");
                return false;
            }

            std::unique_lock<std::shared_mutex> lock(m_mutex);

            auto it = m_directionalLights.find(handle.GetId());
            if (it == m_directionalLights.end())
            {
                Core::Logger::GetInstance().Warning("LightManager: Directional light ID " +
                                                    std::to_string(handle.GetId()) + " not found");
                return false;
            }

            // 检查代数标记
            if (it->second.generation != handle.GetGeneration())
            {
                Core::Logger::GetInstance().Warning("LightManager: Directional light handle is stale (generation mismatch)");
                return false;
            }

            Core::Logger::GetInstance().Info("LightManager: Removed directional light with ID " + std::to_string(handle.GetId()));
            m_directionalLights.erase(it);
            return true;
        }

        bool LightManager::RemovePointLight(const LightHandle &handle)
        {
            if (!handle.IsValid() || handle.GetType() != LightType::POINT)
            {
                Core::Logger::GetInstance().Warning("LightManager: Invalid point light handle");
                return false;
            }

            std::unique_lock<std::shared_mutex> lock(m_mutex);

            auto it = m_pointLights.find(handle.GetId());
            if (it == m_pointLights.end())
            {
                Core::Logger::GetInstance().Warning("LightManager: Point light ID " +
                                                    std::to_string(handle.GetId()) + " not found");
                return false;
            }

            // 检查代数标记
            if (it->second.generation != handle.GetGeneration())
            {
                Core::Logger::GetInstance().Warning("LightManager: Point light handle is stale (generation mismatch)");
                return false;
            }

            Core::Logger::GetInstance().Info("LightManager: Removed point light with ID " + std::to_string(handle.GetId()));
            m_pointLights.erase(it);
            return true;
        }

        bool LightManager::RemoveSpotLight(const LightHandle &handle)
        {
            if (!handle.IsValid() || handle.GetType() != LightType::SPOT)
            {
                Core::Logger::GetInstance().Warning("LightManager: Invalid spot light handle");
                return false;
            }

            std::unique_lock<std::shared_mutex> lock(m_mutex);

            auto it = m_spotLights.find(handle.GetId());
            if (it == m_spotLights.end())
            {
                Core::Logger::GetInstance().Warning("LightManager: Spot light ID " +
                                                    std::to_string(handle.GetId()) + " not found");
                return false;
            }

            // 检查代数标记
            if (it->second.generation != handle.GetGeneration())
            {
                Core::Logger::GetInstance().Warning("LightManager: Spot light handle is stale (generation mismatch)");
                return false;
            }

            Core::Logger::GetInstance().Info("LightManager: Removed spot light with ID " + std::to_string(handle.GetId()));
            m_spotLights.erase(it);
            return true;
        }

        void LightManager::ClearAll()
        {
            std::unique_lock<std::shared_mutex> lock(m_mutex);

            int total = GetTotalLightCount();
            m_directionalLights.clear();
            m_pointLights.clear();
            m_spotLights.clear();

            // 重置ID生成器
            m_nextDirectionalId = 1;
            m_nextPointId = 1;
            m_nextSpotId = 1;

            Core::Logger::GetInstance().Info("LightManager: Cleared all lights (" +
                                            std::to_string(total) + " lights removed)");
        }

        // ========================================
        // 获取光源（使用LightHandle）
        // ========================================

        DirectionalLightPtr LightManager::GetDirectionalLight(const LightHandle &handle)
        {
            if (!handle.IsValid() || handle.GetType() != LightType::DIRECTIONAL)
                return nullptr;

            std::shared_lock<std::shared_mutex> lock(m_mutex);

            auto it = m_directionalLights.find(handle.GetId());
            if (it == m_directionalLights.end())
                return nullptr;

            // 检查代数标记
            if (it->second.generation != handle.GetGeneration())
                return nullptr;

            return std::dynamic_pointer_cast<DirectionalLight>(it->second.light);
        }

        PointLightPtr LightManager::GetPointLight(const LightHandle &handle)
        {
            if (!handle.IsValid() || handle.GetType() != LightType::POINT)
                return nullptr;

            std::shared_lock<std::shared_mutex> lock(m_mutex);

            auto it = m_pointLights.find(handle.GetId());
            if (it == m_pointLights.end())
                return nullptr;

            // 检查代数标记
            if (it->second.generation != handle.GetGeneration())
                return nullptr;

            return std::dynamic_pointer_cast<PointLight>(it->second.light);
        }

        SpotLightPtr LightManager::GetSpotLight(const LightHandle &handle)
        {
            if (!handle.IsValid() || handle.GetType() != LightType::SPOT)
                return nullptr;

            std::shared_lock<std::shared_mutex> lock(m_mutex);

            auto it = m_spotLights.find(handle.GetId());
            if (it == m_spotLights.end())
                return nullptr;

            // 检查代数标记
            if (it->second.generation != handle.GetGeneration())
                return nullptr;

            return std::dynamic_pointer_cast<SpotLight>(it->second.light);
        }

        // ========================================
        // 查询光源数量
        // ========================================

        int LightManager::GetDirectionalLightCount() const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            return static_cast<int>(m_directionalLights.size());
        }

        int LightManager::GetPointLightCount() const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            return static_cast<int>(m_pointLights.size());
        }

        int LightManager::GetSpotLightCount() const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            return static_cast<int>(m_spotLights.size());
        }

        int LightManager::GetTotalLightCount() const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);
            return GetDirectionalLightCount() + GetPointLightCount() + GetSpotLightCount();
        }

        // ========================================
        // 应用光源到着色器（线程安全）
        // ========================================

        void LightManager::ApplyToShader(Shader &shader) const
        {
            // 使用共享锁，允许多个渲染线程并发调用
            std::shared_lock<std::shared_mutex> lock(m_mutex);

            // 设置各类光源的数量
            shader.SetInt("nrDirLights", static_cast<int>(m_directionalLights.size()));
            shader.SetInt("nrPointLights", static_cast<int>(m_pointLights.size()));
            shader.SetInt("nrSpotLights", static_cast<int>(m_spotLights.size()));

            // 应用平行光
            int index = 0;
            for (const auto& pair : m_directionalLights)
            {
                auto dirLight = std::dynamic_pointer_cast<DirectionalLight>(pair.second.light);
                if (dirLight)
                {
                    dirLight->ApplyToShader(shader, index);
                    index++;
                }
            }

            // 应用点光源
            index = 0;
            for (const auto& pair : m_pointLights)
            {
                auto pointLight = std::dynamic_pointer_cast<PointLight>(pair.second.light);
                if (pointLight)
                {
                    pointLight->ApplyToShader(shader, index);
                    index++;
                }
            }

            // 应用聚光灯
            index = 0;
            for (const auto& pair : m_spotLights)
            {
                auto spotLight = std::dynamic_pointer_cast<SpotLight>(pair.second.light);
                if (spotLight)
                {
                    spotLight->ApplyToShader(shader, index);
                    index++;
                }
            }
        }

        // ========================================
        // 统计和调试信息
        // ========================================

        std::string LightManager::GetStatistics() const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);

            std::ostringstream oss;
            oss << "LightManager Statistics:\n";
            oss << "  Directional Lights: " << m_directionalLights.size() << "/" << MAX_DIRECTIONAL_LIGHTS << "\n";
            oss << "  Point Lights: " << m_pointLights.size() << "/" << MAX_POINT_LIGHTS << "\n";
            oss << "  Spot Lights: " << m_spotLights.size() << "/" << MAX_SPOT_LIGHTS << "\n";
            oss << "  Total Lights: " << (m_directionalLights.size() + m_pointLights.size() + m_spotLights.size());

            return oss.str();
        }

        void LightManager::PrintAllLights() const
        {
            std::shared_lock<std::shared_mutex> lock(m_mutex);

            Core::Logger::GetInstance().Info("========================================");
            Core::Logger::GetInstance().Info(GetStatistics());
            Core::Logger::GetInstance().Info("========================================");

            for (const auto& pair : m_directionalLights)
            {
                auto dirLight = std::dynamic_pointer_cast<DirectionalLight>(pair.second.light);
                if (dirLight)
                {
                    Core::Logger::GetInstance().Info("  [ID:" + std::to_string(pair.first) + "] " +
                                                     dirLight->GetDescription());
                }
            }

            for (const auto& pair : m_pointLights)
            {
                auto pointLight = std::dynamic_pointer_cast<PointLight>(pair.second.light);
                if (pointLight)
                {
                    Core::Logger::GetInstance().Info("  [ID:" + std::to_string(pair.first) + "] " +
                                                     pointLight->GetDescription());
                }
            }

            for (const auto& pair : m_spotLights)
            {
                auto spotLight = std::dynamic_pointer_cast<SpotLight>(pair.second.light);
                if (spotLight)
                {
                    Core::Logger::GetInstance().Info("  [ID:" + std::to_string(pair.first) + "] " +
                                                     spotLight->GetDescription());
                }
            }

            Core::Logger::GetInstance().Info("========================================");
        }

    } // namespace Lighting
} // namespace Renderer
