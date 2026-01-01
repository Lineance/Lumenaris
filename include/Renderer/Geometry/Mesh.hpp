#pragma once
#include <glad/glad.h>
#include "Core/GLM.hpp"
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>

namespace Renderer
{

    class IMesh
    {
    public:
        virtual void Create() = 0;
        virtual void Draw() const = 0;
        virtual ~IMesh() = default;

        // 获取 OpenGL VAO（用于实例化渲染）
        virtual unsigned int GetVAO() const = 0;

        // 获取顶点和索引数量（用于实例化渲染）
        virtual size_t GetVertexCount() const = 0;
        virtual size_t GetIndexCount() const = 0;
        virtual bool HasIndices() const = 0;

        // 获取纹理（可选）
        virtual bool HasTexture() const { return false; }
    };

    // 工厂模式 - 用于创建各种网格对象
    class MeshFactory
    {
        // C++17 inline static，强制编译期定义，避免类外初始化
        inline static std::unordered_map<std::string, std::function<std::unique_ptr<IMesh>()>> s_registry;

    public:
        static void Register(const std::string &type, std::function<std::unique_ptr<IMesh>()> creator);
        static std::unique_ptr<IMesh> Create(const std::string &type);
    };

} // namespace Renderer