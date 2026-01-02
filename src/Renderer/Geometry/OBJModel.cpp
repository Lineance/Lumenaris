#include "Renderer/Geometry/OBJModel.hpp"
#include "Core/Logger.hpp"
#include <algorithm>
#include <unordered_map>

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
            // ✅ 性能优化：为每个材质只收集实际使用的顶点
            // 避免复制全部顶点，减少内存占用和GPU传输

            // 第一步：收集每个材质使用的唯一顶点索引
            std::vector<std::vector<unsigned int>> materialVertexIndices(materials.size());
            for (size_t matIdx = 0; matIdx < materials.size(); ++matIdx)
            {
                // 收集使用此材质的所有面的索引
                for (size_t faceIdx = 0; faceIdx < faceMaterialIndices.size(); ++faceIdx)
                {
                    if (faceMaterialIndices[faceIdx] == static_cast<int>(matIdx))
                    {
                        size_t idxStart = faceIdx * 3;
                        if (idxStart + 2 < indices.size())
                        {
                            materialVertexIndices[matIdx].push_back(indices[idxStart]);
                            materialVertexIndices[matIdx].push_back(indices[idxStart + 1]);
                            materialVertexIndices[matIdx].push_back(indices[idxStart + 2]);
                        }
                    }
                }

                // 如果此材质没有使用任何面，跳过
                if (materialVertexIndices[matIdx].empty())
                {
                    continue;
                }

                // 第二步：获取唯一顶点索引（去重）
                std::vector<unsigned int> uniqueIndices = materialVertexIndices[matIdx];
                std::sort(uniqueIndices.begin(), uniqueIndices.end());
                uniqueIndices.erase(std::unique(uniqueIndices.begin(), uniqueIndices.end()), uniqueIndices.end());

                // 第三步：创建局部顶点索引映射（全局索引 → 局部索引）
                std::unordered_map<unsigned int, unsigned int> globalToLocalMap;
                for (size_t i = 0; i < uniqueIndices.size(); ++i)
                {
                    globalToLocalMap[uniqueIndices[i]] = static_cast<unsigned int>(i);
                }

                // 第四步：只复制实际使用的顶点
                MaterialVertexData data;
                data.material = materials[matIdx];
                data.texturePath = loader.GetBasePath() + materials[matIdx].diffuseTexname;

                // 预分配空间：只分配实际使用的顶点
                data.vertices.reserve(uniqueIndices.size() * 8);
                data.indices.reserve(materialVertexIndices[matIdx].size());

                // 复制实际使用的顶点数据
                for (unsigned int globalIdx : uniqueIndices)
                {
                    const auto& vertex = vertices[globalIdx];
                    data.vertices.push_back(vertex.position.x);
                    data.vertices.push_back(vertex.position.y);
                    data.vertices.push_back(vertex.position.z);
                    data.vertices.push_back(vertex.normal.x);
                    data.vertices.push_back(vertex.normal.y);
                    data.vertices.push_back(vertex.normal.z);
                    data.vertices.push_back(vertex.texCoord.x);
                    data.vertices.push_back(vertex.texCoord.y);
                }

                // 转换索引：全局索引 → 局部索引
                for (unsigned int globalIdx : materialVertexIndices[matIdx])
                {
                    data.indices.push_back(globalToLocalMap[globalIdx]);
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
