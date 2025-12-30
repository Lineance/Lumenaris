#pragma once
#include <Core/MathTypes.hpp>
#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace Renderer
{

    struct OBJVertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
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

        // 获取索引数据（如果有的话）
        const std::vector<unsigned int>& GetIndices() const { return m_indices; }

        // 是否有索引数据
        bool HasIndices() const { return !m_indices.empty(); }

        // 清理数据
        void Clear();

    private:
        // 解析OBJ文件行
        void ParseLine(const std::string& line);

        // 解析顶点位置 (v x y z)
        void ParseVertex(const std::string& line);

        // 解析纹理坐标 (vt u v)
        void ParseTexCoord(const std::string& line);

        // 解析法线 (vn x y z)
        void ParseNormal(const std::string& line);

        // 解析面 (f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ...)
        void ParseFace(const std::string& line);

        // 从字符串中提取浮点数
        std::vector<float> ExtractFloats(const std::string& str, int count);

        // 从面定义中提取索引
        glm::ivec3 ExtractFaceIndices(const std::string& faceStr);

        // 原始数据存储
        std::vector<glm::vec3> m_positions;    // 顶点位置
        std::vector<glm::vec2> m_texCoords;    // 纹理坐标
        std::vector<glm::vec3> m_normals;      // 法线

        // 最终用于渲染的顶点数据
        std::vector<OBJVertex> m_vertices;
        std::vector<unsigned int> m_indices;

        // 用于避免重复顶点的映射
        std::vector<std::vector<std::vector<unsigned int>>> m_vertexCache;
    };

} // namespace Renderer
