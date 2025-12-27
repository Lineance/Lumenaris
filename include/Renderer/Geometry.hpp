#pragma once
#include <Core/MathTypes.hpp>
#include <glad/glad.h>
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>

namespace Renderer
{

    class IGeometry
    {
    public:
        virtual void Create() = 0;
        virtual void Draw() const = 0;
        virtual ~IGeometry() = default;
    };

    // 工程模式
    class GeometryFactory
    {
        // C++17 inline static，强制编译期定义，避免类外初始化
        inline static std::unordered_map<std::string, std::function<std::unique_ptr<IGeometry>()>> s_registry;

    public:
        static void Register(const std::string &type, std::function<std::unique_ptr<IGeometry>()> creator);
        static std::unique_ptr<IGeometry> Create(const std::string &type);
    };

} // namespace Renderer