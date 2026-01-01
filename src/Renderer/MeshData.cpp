#include "Renderer/MeshData.hpp"

namespace Renderer
{

    void MeshData::SetVertices(const std::vector<float>& vertices, size_t stride)
    {
        m_vertices = vertices;
        m_vertexStride = stride;
        m_vertexCount = stride > 0 ? vertices.size() / stride : 0;
    }

    void MeshData::SetIndices(const std::vector<unsigned int>& indices)
    {
        m_indices = indices;
        m_indexCount = indices.size();
    }

    void MeshData::SetVertexLayout(const std::vector<size_t>& offsets, const std::vector<int>& sizes)
    {
        m_attributeOffsets = offsets;
        m_attributeSizes = sizes;
    }

    void MeshData::Clear()
    {
        m_vertices.clear();
        m_indices.clear();
        m_attributeOffsets.clear();
        m_attributeSizes.clear();
        m_vertexStride = 0;
        m_vertexCount = 0;
        m_indexCount = 0;
        m_materialColor = glm::vec3(1.0f);
    }

} // namespace Renderer
