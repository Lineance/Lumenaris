#include "Renderer/Geometry/OBJModel.hpp"
#include "Core/Logger.hpp"
#include <algorithm>

namespace Renderer
{

    std::vector<OBJModel::MaterialVertexData> OBJModel::GetMaterialVertexData(const std::string& objPath)
    {
        std::vector<MaterialVertexData> materialDataList;

        // 使用 OBJLoader 加载数据
        OBJLoader loader;
        if (!loader.LoadFromFile(objPath))
        {
            Core::Logger::GetInstance().Error("Failed to load OBJ file: " + objPath);
            return materialDataList;
        }

        const auto& materials = loader.GetMaterials();
        const auto& vertices = loader.GetVertices();
        const auto& indices = loader.GetIndices();
        const auto& faceMaterialIndices = loader.GetFaceMaterialIndices();

        if (vertices.empty())
        {
            Core::Logger::GetInstance().Error("OBJ file has no vertices: " + objPath);
            return materialDataList;
        }

        // 如果没有材质，返回单个网格的数据
        if (materials.empty())
        {
            MaterialVertexData data;

            // 转换所有顶点数据
            data.vertices.reserve(vertices.size() * 8);
            for (const auto& vertex : vertices)
            {
                data.vertices.push_back(vertex.position.x);
                data.vertices.push_back(vertex.position.y);
                data.vertices.push_back(vertex.position.z);
                data.vertices.push_back(vertex.normal.x);
                data.vertices.push_back(vertex.normal.y);
                data.vertices.push_back(vertex.normal.z);
                data.vertices.push_back(vertex.texCoord.x);
                data.vertices.push_back(vertex.texCoord.y);
            }

            // 复制索引
            data.indices = indices;

            materialDataList.push_back(std::move(data));
        }
        else
        {
            // 为每个材质创建顶点数据
            for (size_t matIdx = 0; matIdx < materials.size(); ++matIdx)
            {
                MaterialVertexData data;
                data.material = materials[matIdx];
                data.texturePath = loader.GetBasePath() + materials[matIdx].diffuseTexname;

                // 收集使用此材质的所有面的索引
                for (size_t faceIdx = 0; faceIdx < faceMaterialIndices.size(); ++faceIdx)
                {
                    if (faceMaterialIndices[faceIdx] == static_cast<int>(matIdx))
                    {
                        size_t idxStart = faceIdx * 3;
                        if (idxStart + 2 < indices.size())
                        {
                            data.indices.push_back(indices[idxStart]);
                            data.indices.push_back(indices[idxStart + 1]);
                            data.indices.push_back(indices[idxStart + 2]);
                        }
                    }
                }

                // 如果此材质没有使用任何面，跳过
                if (data.indices.empty())
                {
                    continue;
                }

                // 转换所有顶点数据（每个材质网格包含全部顶点）
                data.vertices.reserve(vertices.size() * 8);
                for (const auto& vertex : vertices)
                {
                    data.vertices.push_back(vertex.position.x);
                    data.vertices.push_back(vertex.position.y);
                    data.vertices.push_back(vertex.position.z);
                    data.vertices.push_back(vertex.normal.x);
                    data.vertices.push_back(vertex.normal.y);
                    data.vertices.push_back(vertex.normal.z);
                    data.vertices.push_back(vertex.texCoord.x);
                    data.vertices.push_back(vertex.texCoord.y);
                }

                materialDataList.push_back(std::move(data));
            }
        }

        Core::Logger::GetInstance().Info("Generated material vertex data: " +
                                         std::to_string(materialDataList.size()) + " materials");
        return materialDataList;
    }

    MeshData OBJModel::GetMeshData(const std::string& objPath)
    {
        MeshData data;

        // 使用 OBJLoader 加载数据
        OBJLoader loader;
        if (!loader.LoadFromFile(objPath))
        {
            Core::Logger::GetInstance().Error("Failed to load OBJ file: " + objPath);
            return data;
        }

        const auto& vertices = loader.GetVertices();
        const auto& indices = loader.GetIndices();

        if (vertices.empty())
        {
            Core::Logger::GetInstance().Error("OBJ file has no vertices: " + objPath);
            return data;
        }

        // 转换顶点数据
        std::vector<float> vertexData;
        vertexData.reserve(vertices.size() * 8);
        for (const auto& vertex : vertices)
        {
            vertexData.push_back(vertex.position.x);
            vertexData.push_back(vertex.position.y);
            vertexData.push_back(vertex.position.z);
            vertexData.push_back(vertex.normal.x);
            vertexData.push_back(vertex.normal.y);
            vertexData.push_back(vertex.normal.z);
            vertexData.push_back(vertex.texCoord.x);
            vertexData.push_back(vertex.texCoord.y);
        }

        // 设置顶点和索引数据
        data.SetVertices(std::move(vertexData), 8);  // 8 floats per vertex
        if (!indices.empty())
        {
            data.SetIndices(indices);
        }

        return data;
    }

    std::vector<OBJMaterial> OBJModel::GetMaterials(const std::string& objPath)
    {
        OBJLoader loader;
        if (!loader.LoadFromFile(objPath))
        {
            Core::Logger::GetInstance().Error("Failed to load OBJ file: " + objPath);
            return {};
        }

        return loader.GetMaterials();
    }

    bool OBJModel::HasMaterials(const std::string& objPath)
    {
        OBJLoader loader;
        if (!loader.LoadFromFile(objPath))
        {
            return false;
        }

        return !loader.GetMaterials().empty();
    }

    void OBJModel::GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes)
    {
        offsets = {0, 3, 6};  // 位置、法线、UV 的偏移
        sizes = {3, 3, 2};     // 位置(3), 法线(3), UV(2)
    }

} // namespace Renderer
