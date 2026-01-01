#pragma once
#include "Renderer/Mesh.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/OBJModel.hpp"
#include "Core/GLM.hpp"
#include <vector>
#include <memory>
#include <glad/glad.h>

namespace Renderer
{

    /**
     * @class SimpleMesh
     * @brief 简单网格类 - 纯粹的数据容器，用于实例化渲染
     *
     * 职责：
     * - 存储顶点数据（位置、法线、UV）
     * - 存储 VAO/VBO/EBO
     * - 提供基本的 Create() 和 Draw() 方法
     *
     * 与 InstancedRenderer 配合使用：
     * - SimpleMesh: 网格数据
     * - InstanceData: 实例数据
     * - InstancedRenderer: 渲染逻辑
     */
    class SimpleMesh : public IMesh
    {
    public:
        SimpleMesh() = default;
        ~SimpleMesh();

        // 拷贝构造函数（深拷贝）
        SimpleMesh(const SimpleMesh& other);

        // 拷贝赋值运算符（深拷贝）
        SimpleMesh& operator=(const SimpleMesh& other);

        // 移动构造函数（显式实现以提高效率）
        SimpleMesh(SimpleMesh&& other) noexcept;

        // 移动赋值运算符（显式实现以提高效率）
        SimpleMesh& operator=(SimpleMesh&& other) noexcept;

        // IMesh 接口实现
        void Create() override;
        void Draw() const override;

        // IMesh 接口扩展
        unsigned int GetVAO() const override { return m_vao; }
        size_t GetVertexCount() const override { return m_vertexCount; }
        size_t GetIndexCount() const override { return m_indexCount; }
        bool HasIndices() const override { return m_hasIndices; }
        bool HasTexture() const override { return m_texture != nullptr; }

        // 设置顶点数据
        void SetVertexData(const std::vector<float>& vertices, size_t stride);
        void SetVertexLayout(const std::vector<size_t>& offsets, const std::vector<int>& sizes);

        // 设置索引数据
        void SetIndexData(const std::vector<unsigned int>& indices);

        // 设置纹理（使用 shared_ptr 管理所有权）
        void SetTexture(std::shared_ptr<Texture> texture);
        std::shared_ptr<Texture> GetTexture() const { return m_texture; }

        // 设置材质颜色
        void SetMaterialColor(const glm::vec3& color) { m_materialColor = color; }
        const glm::vec3& GetMaterialColor() const { return m_materialColor; }

        // 静态辅助方法：从 Cube 创建
        static SimpleMesh CreateFromCube();

        // 静态辅助方法：从 OBJ 材质数据创建
        static SimpleMesh CreateFromMaterialData(const OBJModel::MaterialVertexData& materialData);

    private:
        // OpenGL 对象
        unsigned int m_vao = 0;
        unsigned int m_vbo = 0;
        unsigned int m_ebo = 0;

        // 顶点数据
        std::vector<float> m_vertices;
        std::vector<unsigned int> m_indices;
        size_t m_vertexStride = 0;
        size_t m_vertexCount = 0;
        size_t m_indexCount = 0;
        bool m_hasIndices = false;

        // 材质和纹理
        std::shared_ptr<Texture> m_texture;
        glm::vec3 m_materialColor = glm::vec3(1.0f);

        // 顶点属性布局
        struct VertexAttribute {
            size_t offset;
            int size;
        };
        std::vector<VertexAttribute> m_vertexAttributes;
    };

} // namespace Renderer
