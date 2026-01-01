#pragma once
#include "Core/GLM.hpp"
#include <vector>
#include <cstddef>

namespace Renderer
{

    /**
     * @class MeshData
     * @brief 纯数据容器 - 存储网格的顶点和索引数据（CPU 内存）
     *
     * 设计原则：
     * - ✅ 纯数据容器，类似 InstanceData
     * - ✅ 无 GPU 资源（无 VAO/VBO/EBO）
     * - ✅ 无渲染能力（无 Draw/Render）
     * - ✅ 可序列化，可传递，可复制
     *
     * 使用场景：
     * - 作为数据交换格式
     * - 用于序列化/反序列化
     * - 传递给 MeshBuffer 上传到 GPU
     */
    class MeshData
    {
    public:
        MeshData() = default;
        ~MeshData() = default;

        // ============================================================
        // 数据设置接口
        // ============================================================

        /**
         * @brief 设置顶点数据
         * @param vertices 顶点数据数组
         * @param stride 每个顶点的步长（字节数 / sizeof(float)）
         */
        void SetVertices(const std::vector<float>& vertices, size_t stride);

        /**
         * @brief 设置索引数据
         * @param indices 索引数据数组
         */
        void SetIndices(const std::vector<unsigned int>& indices);

        /**
         * @brief 设置顶点属性布局
         * @param offsets 每个属性在顶点中的偏移（float 索引）
         * @param sizes 每个属性的大小（float 数量）
         *
         * 例如：位置(3) + 法线(3) + UV(2)
         * offsets = {0, 3, 6}
         * sizes = {3, 3, 2}
         */
        void SetVertexLayout(const std::vector<size_t>& offsets, const std::vector<int>& sizes);

        /**
         * @brief 设置材质颜色
         */
        void SetMaterialColor(const glm::vec3& color) { m_materialColor = color; }

        // ============================================================
        // 数据访问接口
        // ============================================================

        const std::vector<float>& GetVertices() const { return m_vertices; }
        const std::vector<unsigned int>& GetIndices() const { return m_indices; }
        size_t GetVertexStride() const { return m_vertexStride; }
        size_t GetVertexCount() const { return m_vertexCount; }
        size_t GetIndexCount() const { return m_indexCount; }
        bool HasIndices() const { return !m_indices.empty(); }
        const glm::vec3& GetMaterialColor() const { return m_materialColor; }

        const std::vector<size_t>& GetAttributeOffsets() const { return m_attributeOffsets; }
        const std::vector<int>& GetAttributeSizes() const { return m_attributeSizes; }

        // ============================================================
        // 工具方法
        // ============================================================

        /**
         * @brief 清空所有数据
         */
        void Clear();

        /**
         * @brief 检查是否为空
         */
        bool IsEmpty() const { return m_vertices.empty(); }

        /**
         * @brief 计算顶点数据的字节大小
         */
        size_t GetVertexDataSizeBytes() const {
            return m_vertices.size() * sizeof(float);
        }

        /**
         * @brief 计算索引数据的字节大小
         */
        size_t GetIndexDataSizeBytes() const {
            return m_indices.size() * sizeof(unsigned int);
        }

    private:
        // 顶点数据
        std::vector<float> m_vertices;
        std::vector<unsigned int> m_indices;
        size_t m_vertexStride = 0;      // 每个顶点的 float 数量
        size_t m_vertexCount = 0;
        size_t m_indexCount = 0;

        // 材质数据
        glm::vec3 m_materialColor = glm::vec3(1.0f);

        // 顶点属性布局
        std::vector<size_t> m_attributeOffsets;  // 每个属性的偏移（float 索引）
        std::vector<int> m_attributeSizes;       // 每个属性的大小（float 数量）
    };

} // namespace Renderer
