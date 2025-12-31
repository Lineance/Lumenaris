#pragma once
#include <string>

namespace Renderer
{

    /**
     * @interface IRenderer
     * @brief 渲染器抽象接口
     *
     * 职责：
     * - 定义渲染器的统一接口
     * - 支持初始化和渲染操作
     *
     * 说明：
     * - 与 IMesh 接口分离，强调"渲染器"的概念
     * - IMesh：表示可渲染的几何体
     * - IRenderer：表示渲染逻辑的执行者
     */
    class IRenderer
    {
    public:
        virtual ~IRenderer() = default;

        /**
         * @brief 初始化渲染器（创建 OpenGL 缓冲区等）
         */
        virtual void Initialize() = 0;

        /**
         * @brief 执行渲染
         */
        virtual void Render() const = 0;

        /**
         * @brief 获取渲染器名称（用于调试）
         */
        virtual std::string GetName() const = 0;
    };

} // namespace Renderer
