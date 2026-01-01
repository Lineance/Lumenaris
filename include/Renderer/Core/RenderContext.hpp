#pragma once

#include "Renderer/Lighting/LightManager.hpp"
#include "Renderer/Environment/Skybox.hpp"
#include "Renderer/Environment/AmbientLighting.hpp"
#include <memory>

namespace Renderer
{
    namespace Core
    {

        /**
         * RenderContext 类 - 渲染上下文（多Context架构核心）
         *
         * 设计目标：
         * - 每个渲染场景（主场景、ImGui层、预览窗口）拥有独立的Context
         * - Context包含完全独立的光照、环境、渲染状态
         * - 避免单例模式导致的全局状态污染
         *
         * 架构优势：
         * - ✅ ImGui2D层可以拥有零光照环境，不影响主场景
         * - ✅ 多窗口预览完全独立，无状态覆盖问题
         * - ✅ 线程安全：每个Context独立加锁，无跨Context锁竞争
         * - ✅ 可测试性：每个Context独立创建销毁
         *
         * 使用示例：
         * ```cpp
         * // 创建多个独立Context
         * RenderContext mainContext;     // 主游戏场景
         * RenderContext imguiContext;    // ImGui2D层
         * RenderContext previewContext;  // 编辑器预览
         *
         * // 主场景渲染（丰富光照）
         * mainContext.GetLightManager().AddPointLight(mainLight);
         * mainContext.GetLightManager().ApplyToShader(shader);
         *
         * // ImGui渲染（零光照，不影响主场景）
         * imguiContext.GetLightManager().ClearAll();  // ✅ 只影响imguiContext
         * imguiRenderer.Render(imguiContext);
         *
         * // 预览窗口（完全独立的场景）
         * previewContext.GetLightManager().LoadFrom(previewData);
         * previewContext.GetLightManager().ApplyToShader(previewShader);
         * ```
         */
        class RenderContext
        {
        public:
            // ========================================
            // 构造函数和析构函数
            // ========================================

            RenderContext() = default;
            ~RenderContext() = default;

            // 禁止拷贝，允许移动
            RenderContext(const RenderContext &) = delete;
            RenderContext &operator=(const RenderContext &) = delete;
            RenderContext(RenderContext &&) noexcept = default;
            RenderContext &operator=(RenderContext &&) noexcept = default;

            // ========================================
            // 光照管理
            // ========================================

            /**
             * 获取光照管理器
             *
             * 每个RenderContext拥有独立的光照管理器实例
             * 修改此Context的光照不会影响其他Context
             */
            Lighting::LightManager &GetLightManager() { return m_lightManager; }
            const Lighting::LightManager &GetLightManager() const { return m_lightManager; }

            // ========================================
            // 环境渲染
            // ========================================

            /**
             * 获取天空盒
             *
             * 每个Context可以拥有独立的天空盒
             */
            Skybox &GetSkybox() { return m_skybox; }
            const Skybox &GetSkybox() const { return m_skybox; }

            /**
             * 获取环境光照
             *
             * 每个Context可以拥有独立的环境光照设置
             */
            AmbientLighting &GetAmbientLighting() { return m_ambientLighting; }
            const AmbientLighting &GetAmbientLighting() const { return m_ambientLighting; }

            // ========================================
            // Context控制
            // ========================================

            /**
             * 清空Context所有状态
             *
             * 清除所有光源、重置环境光照
             * 不影响其他Context
             */
            void Clear();

            /**
             * 获取Context统计信息（用于调试）
             */
            std::string GetStatistics() const;

        private:
            // ========================================
            // 成员变量
            // ========================================

            Lighting::LightManager m_lightManager;     // 独立的光照管理器
            Skybox m_skybox;                            // 独立的天空盒
            AmbientLighting m_ambientLighting;          // 独立的环境光照
        };

    } // namespace Core
} // namespace Renderer
