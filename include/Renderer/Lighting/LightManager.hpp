#pragma once

#include "Renderer/Lighting/Light.hpp"
#include "Renderer/Resources/Shader.hpp"
#include <vector>
#include <memory>
#include <algorithm>

namespace Renderer
{
    namespace Lighting
    {

        /**
         * LightManager 类 - 光源管理器
         *
         * 功能：
         * - 管理场景中的所有光源
         * - 统一将光源数据传递给着色器
         * - 提供光源的增删改查接口
         * - 支持按类型查询和操作光源
         * - 限制各类光源的最大数量（与着色器中的数组大小对应）
         *
         * 设计：
         * - 单例模式，全局唯一的光源管理器
         * - 分别管理三种类型的光源（Directional, Point, Spot）
         * - 自动过滤禁用的光源
         */
        class LightManager
        {
        public:
            // 获取单例实例
            static LightManager &GetInstance();

            // 禁止拷贝和移动
            LightManager(const LightManager &) = delete;
            LightManager &operator=(const LightManager &) = delete;
            LightManager(LightManager &&) = delete;
            LightManager &operator=(LightManager &&) = delete;

            // ========================================
            // 光源数量限制（与着色器中的数组大小对应）
            // ========================================

            static const int MAX_DIRECTIONAL_LIGHTS = 4;
            static const int MAX_POINT_LIGHTS = 48;  // 更新为48以支持三层光源布局
            static const int MAX_SPOT_LIGHTS = 8;

            // ========================================
            // 添加光源
            // ========================================

            /**
             * 添加平行光
             * @return 光源的索引（用于后续操作）
             */
            int AddDirectionalLight(const DirectionalLightPtr &light);

            /**
             * 添加点光源
             * @return 光源的索引（用于后续操作）
             */
            int AddPointLight(const PointLightPtr &light);

            /**
             * 添加聚光灯
             * @return 光源的索引（用于后续操作）
             */
            int AddSpotLight(const SpotLightPtr &light);

            // ========================================
            // 移除光源
            // ========================================

            /**
             * 移除平行光
             * @param index 光源索引
             * @return 是否成功移除
             */
            bool RemoveDirectionalLight(int index);

            /**
             * 移除点光源
             * @param index 光源索引
             * @return 是否成功移除
             */
            bool RemovePointLight(int index);

            /**
             * 移除聚光灯
             * @param index 光源索引
             * @return 是否成功移除
             */
            bool RemoveSpotLight(int index);

            /**
             * 清空所有光源
             */
            void ClearAll();

            // ========================================
            // 获取光源
            // ========================================

            DirectionalLightPtr GetDirectionalLight(int index);
            PointLightPtr GetPointLight(int index);
            SpotLightPtr GetSpotLight(int index);

            // ========================================
            // 查询光源数量
            // ========================================

            int GetDirectionalLightCount() const { return static_cast<int>(m_directionalLights.size()); }
            int GetPointLightCount() const { return static_cast<int>(m_pointLights.size()); }
            int GetSpotLightCount() const { return static_cast<int>(m_spotLights.size()); }

            int GetTotalLightCount() const
            {
                return GetDirectionalLightCount() + GetPointLightCount() + GetSpotLightCount();
            }

            // ========================================
            // 应用光源到着色器
            // ========================================

            /**
             * 将所有启用的光源传递给着色器
             * @param shader 目标着色器
             */
            void ApplyToShader(Shader &shader) const;

            /**
             * 获取光源统计信息（用于调试）
             */
            std::string GetStatistics() const;

            /**
             * 打印所有光源信息（用于调试）
             */
            void PrintAllLights() const;

        private:
            // 私有构造函数（单例模式）
            LightManager() = default;
            ~LightManager() = default;

            // 光源容器
            std::vector<DirectionalLightPtr> m_directionalLights;
            std::vector<PointLightPtr> m_pointLights;
            std::vector<SpotLightPtr> m_spotLights;
        };

    } // namespace Lighting
} // namespace Renderer
