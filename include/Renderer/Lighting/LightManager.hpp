#pragma once

#include "Renderer/Lighting/Light.hpp"
#include "Renderer/Resources/Shader.hpp"
#include <vector>
#include <memory>
#include <shared_mutex>
#include <unordered_map>

namespace Renderer
{
    namespace Lighting
    {

        /**
         * LightManager 类 - 光源管理器（多Context架构版）
         *
         * 修复内容：
         * - ⭐ 完全移除单例模式，支持多Context架构
         * - 添加线程安全支持（shared_mutex读写锁）
         * - 使用 LightHandle 替代索引（修复索引失效问题）
         * - 使用 constexpr 避免ODR违规
         * - 线程安全警告和使用指南
         *
         * 功能：
         * - 管理场景中的所有光源
         * - 统一将光源数据传递给着色器
         * - 提供光源的增删改查接口（基于LightHandle）
         * - 支持按类型查询和操作光源
         * - 限制各类光源的最大数量（与着色器中的数组大小对应）
         *
         * 设计：
         * - ⭐ 可实例化类，每个RenderContext拥有独立实例
         * - 支持移动语义，便于传递所有权
         * - 分别管理三种类型的光源（Directional, Point, Spot）
         * - 使用 LightHandle 保证引用稳定性
         * - 使用 shared_mutex 提供读写并发支持
         *
         * 线程安全：
         * - 所有公共方法都是线程安全的
         * - 读操作使用共享锁（允许并发读）
         * - 写操作使用独占锁
         * - ApplyToShader 可在渲染线程并发调用
         *
         * 使用方式：
         * - 通过 RenderContext::GetLightManager() 获取实例
         * - 每个RenderContext拥有独立的LightManager实例
         * - 完全隔离，无全局状态污染
         */
        class LightManager
        {
        public:
            // ========================================
            // 构造函数和析构函数
            // ========================================

            LightManager() = default;
            ~LightManager() = default;

            // 禁止拷贝，允许移动
            LightManager(const LightManager &) = delete;
            LightManager &operator=(const LightManager &) = delete;
            LightManager(LightManager &&) noexcept = default;
            LightManager &operator=(LightManager &&) noexcept = default;

            // ========================================
            // 光源数量限制（修复ODR违规：使用constexpr）
            // ========================================

            // C++17 inline constexpr 避免ODR违规
            inline static constexpr int MAX_DIRECTIONAL_LIGHTS = 4;
            inline static constexpr int MAX_POINT_LIGHTS = 48;
            inline static constexpr int MAX_SPOT_LIGHTS = 8;

            // ========================================
            // 添加光源（返回LightHandle）
            // ========================================

            /**
             * 添加平行光
             * @return LightHandle（稳定的引用句柄）
             */
            LightHandle AddDirectionalLight(const DirectionalLightPtr &light);

            /**
             * 添加点光源
             * @return LightHandle（稳定的引用句柄）
             */
            LightHandle AddPointLight(const PointLightPtr &light);

            /**
             * 添加聚光灯
             * @return LightHandle（稳定的引用句柄）
             */
            LightHandle AddSpotLight(const SpotLightPtr &light);

            // ========================================
            // 移除光源（使用LightHandle）
            // ========================================

            /**
             * 移除平行光
             * @param handle 光源句柄
             * @return 是否成功移除
             */
            bool RemoveDirectionalLight(const LightHandle &handle);

            /**
             * 移除点光源
             * @param handle 光源句柄
             * @return 是否成功移除
             */
            bool RemovePointLight(const LightHandle &handle);

            /**
             * 移除聚光灯
             * @param handle 光源句柄
             * @return 是否成功移除
             */
            bool RemoveSpotLight(const LightHandle &handle);

            /**
             * 清空所有光源
             */
            void ClearAll();

            // ========================================
            // 获取光源（使用LightHandle）
            // ========================================

            DirectionalLightPtr GetDirectionalLight(const LightHandle &handle);
            PointLightPtr GetPointLight(const LightHandle &handle);
            SpotLightPtr GetSpotLight(const LightHandle &handle);

            // ========================================
            // 查询光源数量
            // ========================================

            int GetDirectionalLightCount() const;
            int GetPointLightCount() const;
            int GetSpotLightCount() const;

            int GetTotalLightCount() const;

            // ========================================
            // 应用光源到着色器（线程安全）
            // ========================================

            /**
             * 将所有光源传递给着色器（线程安全）
             *
             * 注意：
             * - 禁用的光源也会设置 uniform 为零值（避免未初始化数据）
             * - 此方法使用共享锁，允许多个渲染线程并发调用
             * - 主线程可以在渲染期间安全地添加/移除光源
             *
             * @param shader 目标着色器
             */
            void ApplyToShader(Shader &shader) const;

            // ========================================
            // 调试和统计
            // ========================================

            /**
             * 获取光源统计信息（用于调试）
             */
            std::string GetStatistics() const;

            /**
             * 打印所有光源信息（用于调试）
             */
            void PrintAllLights() const;

        private:
            // ========================================
            // 内部辅助结构
            // ========================================

            struct LightEntry {
                LightPtr light;
                size_t generation;  // 代数标记，用于检测失效句柄
            };

            // ========================================
            // 成员变量
            // ========================================

            // 光源容器（使用 unordered_map 存储稳定的ID映射）
            std::unordered_map<size_t, LightEntry> m_directionalLights;
            std::unordered_map<size_t, LightEntry> m_pointLights;
            std::unordered_map<size_t, LightEntry> m_spotLights;

            // ID 生成器
            size_t m_nextDirectionalId = 1;
            size_t m_nextPointId = 1;
            size_t m_nextSpotId = 1;

            // 线程安全：读写锁（C++17 shared_mutex）
            mutable std::shared_mutex m_mutex;
        };

    } // namespace Lighting
} // namespace Renderer
