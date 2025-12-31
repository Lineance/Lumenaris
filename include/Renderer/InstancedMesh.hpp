#pragma once
#include "Renderer/Mesh.hpp"
#include "Renderer/Texture.hpp"
#include "Core/GLM.hpp"
#include <vector>
#include <glad/glad.h>

namespace Renderer
{

    // 实例数据结构
    struct InstanceData
    {
        glm::mat4 modelMatrix;
        glm::vec3 color;
    };

    // 通用实例化网格类 - 支持从任意 IMesh 派生类创建实例化渲染
    class InstancedMesh : public IMesh
    {
    public:
        InstancedMesh() = default;
        ~InstancedMesh();

        // IMesh接口实现
        void Create() override;
        void Draw() const override;

        // 从现有网格复制顶点数据
        void SetVertexData(const float* vertices, size_t vertexCount, size_t stride);
        void SetVertexLayout(const std::vector<size_t>& offsets, const std::vector<int>& sizes);

        // 设置索引数据（用于 EBO）
        void SetIndexData(const unsigned int* indices, size_t indexCount);

        // 设置纹理和材质颜色
        void SetTexture(Texture* texture);
        void SetMaterialColor(const glm::vec3& color) { m_materialColor = color; }
        const glm::vec3& GetMaterialColor() const { return m_materialColor; }

        // 实例管理
        void AddInstance(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale, const glm::vec3& color);
        void ClearInstances();
        void UpdateInstanceBuffers();

        // 批量设置实例
        void SetInstances(const std::vector<InstanceData>& instances);

        // 获取信息
        size_t GetInstanceCount() const { return m_instances.size(); }
        size_t GetVertexCount() const { return m_vertexCount; }
        size_t GetIndexCount() const { return m_indexCount; }
        bool HasIndices() const { return m_hasIndices; }
        bool HasTexture() const { return m_texture != nullptr; }

        // 静态辅助方法：从 Cube 创建
        static InstancedMesh CreateFromCube(size_t instanceCount);

        // 静态辅助方法：从 OBJModel 创建（返回多个实例化网格，每个材质一个）
        static std::vector<InstancedMesh> CreateFromOBJ(const std::string& objPath, size_t instanceCount);

    private:
        // 基础网格数据
        unsigned int m_vao = 0, m_vbo = 0, m_ebo = 0;
        unsigned int m_instanceVBO = 0;
        std::vector<float> m_vertices;
        std::vector<unsigned int> m_indices;
        size_t m_vertexStride = 0;
        size_t m_vertexCount = 0;
        size_t m_indexCount = 0;
        bool m_hasIndices = false;

        // 纹理和材质
        Texture* m_texture = nullptr;
        glm::vec3 m_materialColor = glm::vec3(1.0f, 1.0f, 1.0f); // 默认白色

        // 实例数据
        std::vector<InstanceData> m_instances;
        std::vector<glm::mat4> m_instanceMatrices;
        std::vector<glm::vec3> m_instanceColors;

        // 顶点布局信息
        struct VertexAttribute {
            size_t offset;
            int size;
        };
        std::vector<VertexAttribute> m_vertexAttributes;
    };

} // namespace Renderer
