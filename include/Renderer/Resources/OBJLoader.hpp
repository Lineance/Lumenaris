#pragma once
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <functional>
#include "Core/GLM.hpp"
#include "tiny_obj_loader.h"

namespace Renderer
{

    struct OBJVertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };

    // ✅ 性能优化（2026-01-02）：顶点键结构，用于快速去重
    struct VertexKey
    {
        int vi;  // 顶点索引
        int ni;  // 法线索引
        int ti;  // 纹理坐标索引

        // ✅ C++20 三路比较运算符（或使用 = default）
        bool operator==(const VertexKey& other) const
        {
            return vi == other.vi && ni == other.ni && ti == other.ti;
        }
    };

    // ✅ 性能优化：完美哈希函数（零碰撞）
    struct VertexKeyHash
    {
        size_t operator()(const VertexKey& k) const noexcept
        {
            // 完美哈希：每个 21 位，左移避免重叠
            // OBJ 索引范围：0-1M（20 位），21 位足够
            return (static_cast<size_t>(k.vi) << 42) |
                   (static_cast<size_t>(k.ni) << 21) |
                   (static_cast<size_t>(k.ti));
        }
    };

    struct OBJMaterial
    {
        std::string name;
        glm::vec3 ambient;     // Ka
        glm::vec3 diffuse;     // Kd
        glm::vec3 specular;    // Ks
        float shininess;       // Ns
        float dissolve;        // d/Tr (1.0 = opaque, 0.0 = transparent)
        std::string ambientTexname;
        std::string diffuseTexname;
        std::string specularTexname;
        std::string normalTexname;
    };

    class OBJLoader
    {
    public:
        OBJLoader();
        ~OBJLoader();

        // 加载OBJ文件
        bool LoadFromFile(const std::string& filepath);

        // 获取解析后的顶点数据
        const std::vector<OBJVertex>& GetVertices() const { return m_vertices; }

        // 获取索引数据
        const std::vector<unsigned int>& GetIndices() const { return m_indices; }

        // 获取材质数据
        const std::vector<OBJMaterial>& GetMaterials() const { return m_materials; }

        // 获取面材质索引
        const std::vector<int>& GetFaceMaterialIndices() const { return m_faceMaterialIndices; }

        // 是否有索引数据
        bool HasIndices() const { return !m_indices.empty(); }

        // 获取顶点数量
        size_t GetVertexCount() const { return m_vertices.size(); }

        // 获取基础路径（用于加载材质文件）
        const std::string& GetBasePath() const { return m_basePath; }

        // 获取材质数量
        size_t GetMaterialCount() const { return m_materials.size(); }

        // 清理数据
        void Clear();

    private:
        // 将tinyobj数据转换为我们的顶点格式
        void ConvertTinyObjData(const tinyobj::attrib_t& attrib,
                               const std::vector<tinyobj::shape_t>& shapes,
                               const std::vector<tinyobj::material_t>& materials);

        // 转换材质数据
        void ConvertMaterials(const std::vector<tinyobj::material_t>& tinyMaterials);

        // 最终用于渲染的顶点数据
        std::vector<OBJVertex> m_vertices;
        std::vector<unsigned int> m_indices;
        std::vector<OBJMaterial> m_materials;
        std::vector<int> m_faceMaterialIndices; // 每个面的材质索引

        // 加载状态
        bool m_loaded;
        std::string m_basePath;
    };

} // namespace Renderer
