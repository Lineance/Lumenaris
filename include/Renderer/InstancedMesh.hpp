#pragma once
#include "Renderer/Mesh.hpp"
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

    // 实例化网格类
    class InstancedMesh : public IMesh
    {
    public:
        InstancedMesh() = default;
        ~InstancedMesh();

        // IMesh接口实现
        void Create() override;
        void Draw() const override;

        // 设置基础网格数据（在调用Create之前）
        void SetVertices(const float* vertices, size_t count);
        void SetVertexLayout(size_t stride, const std::vector<size_t>& offsets, const std::vector<int>& sizes);

        // 实例管理
        void AddInstance(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale, const glm::vec3& color);
        void ClearInstances();
        void UpdateInstanceBuffers();

        // 批量设置实例
        void SetInstances(const std::vector<InstanceData>& instances);

        // 获取信息
        size_t GetInstanceCount() const { return m_instances.size(); }
        size_t GetVertexCount() const { return m_vertexCount; }

    private:
        // 基础网格数据
        unsigned int m_vao = 0, m_vbo = 0;
        unsigned int m_instanceVBO = 0;
        std::vector<float> m_vertices;
        size_t m_vertexStride = 0;
        size_t m_vertexCount = 0;

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
