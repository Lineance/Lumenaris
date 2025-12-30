#include "Renderer/OBJLoader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

namespace Renderer
{

    OBJLoader::OBJLoader()
    {
        // 初始化顶点缓存
        m_vertexCache.resize(3); // position, texcoord, normal
    }

    OBJLoader::~OBJLoader()
    {
        Clear();
    }

    bool OBJLoader::LoadFromFile(const std::string& filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            std::cerr << "Failed to open OBJ file: " << filepath << std::endl;
            return false;
        }

        Clear();

        std::string line;
        while (std::getline(file, line))
        {
            // 移除行首行尾的空白字符
            line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
            line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), line.end());

            if (line.empty() || line[0] == '#')
                continue;

            ParseLine(line);
        }

        file.close();

        std::cout << "OBJ file loaded successfully: " << filepath << std::endl;
        std::cout << "Vertices: " << m_positions.size() << std::endl;
        std::cout << "TexCoords: " << m_texCoords.size() << std::endl;
        std::cout << "Normals: " << m_normals.size() << std::endl;
        std::cout << "Final vertices: " << m_vertices.size() << std::endl;

        return true;
    }

    void OBJLoader::ParseLine(const std::string& line)
    {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v")
        {
            ParseVertex(line);
        }
        else if (prefix == "vt")
        {
            ParseTexCoord(line);
        }
        else if (prefix == "vn")
        {
            ParseNormal(line);
        }
        else if (prefix == "f")
        {
            ParseFace(line);
        }
        // 忽略其他行（如材质定义等）
    }

    void OBJLoader::ParseVertex(const std::string& line)
    {
        auto values = ExtractFloats(line, 3);
        if (values.size() >= 3)
        {
            m_positions.emplace_back(values[0], values[1], values[2]);
        }
    }

    void OBJLoader::ParseTexCoord(const std::string& line)
    {
        auto values = ExtractFloats(line, 2);
        if (values.size() >= 2)
        {
            m_texCoords.emplace_back(values[0], values[1]);
        }
    }

    void OBJLoader::ParseNormal(const std::string& line)
    {
        auto values = ExtractFloats(line, 3);
        if (values.size() >= 3)
        {
            m_normals.emplace_back(values[0], values[1], values[2]);
        }
    }

    void OBJLoader::ParseFace(const std::string& line)
    {
        std::istringstream iss(line.substr(1)); // 跳过'f'
        std::string vertexStr;
        std::vector<glm::ivec3> faceIndices;

        while (iss >> vertexStr)
        {
            glm::ivec3 indices = ExtractFaceIndices(vertexStr);
            faceIndices.push_back(indices);
        }

        // 只处理三角形（3个顶点）或四边形（4个顶点，拆分为2个三角形）
        if (faceIndices.size() >= 3)
        {
            // 为三角形的每个顶点创建顶点数据
            for (size_t i = 0; i < 3; ++i)
            {
                glm::ivec3 idx = faceIndices[i];

                OBJVertex vertex;
                vertex.position = m_positions[idx.x - 1]; // OBJ索引从1开始

                if (idx.y > 0 && idx.y <= static_cast<int>(m_texCoords.size()))
                {
                    vertex.texCoord = m_texCoords[idx.y - 1];
                }
                else
                {
                    vertex.texCoord = glm::vec2(0.0f, 0.0f);
                }

                if (idx.z > 0 && idx.z <= static_cast<int>(m_normals.size()))
                {
                    vertex.normal = m_normals[idx.z - 1];
                }
                else
                {
                    // 如果没有法线，使用默认法线
                    vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                m_vertices.push_back(vertex);
            }

            // 如果是四边形，添加第二个三角形
            if (faceIndices.size() == 4)
            {
                // 第二个三角形：顶点2、3、0（相对于面的顶点索引）
                for (int i : {1, 2, 3}) // 对应原始顶点索引1, 2, 3（0-based: 1,2,3）
                {
                    int vertexIndex = (i == 3) ? 0 : i; // 3->0, 1->1, 2->2
                    glm::ivec3 idx = faceIndices[vertexIndex];

                    OBJVertex vertex;
                    vertex.position = m_positions[idx.x - 1];

                    if (idx.y > 0 && idx.y <= static_cast<int>(m_texCoords.size()))
                    {
                        vertex.texCoord = m_texCoords[idx.y - 1];
                    }
                    else
                    {
                        vertex.texCoord = glm::vec2(0.0f, 0.0f);
                    }

                    if (idx.z > 0 && idx.z <= static_cast<int>(m_normals.size()))
                    {
                        vertex.normal = m_normals[idx.z - 1];
                    }
                    else
                    {
                        vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                    }

                    m_vertices.push_back(vertex);
                }
            }
        }
    }

    std::vector<float> OBJLoader::ExtractFloats(const std::string& str, int count)
    {
        std::vector<float> result;
        std::istringstream iss(str);
        std::string token;

        // 跳过第一个token（前缀）
        iss >> token;

        for (int i = 0; i < count && iss >> token; ++i)
        {
            try
            {
                result.push_back(std::stof(token));
            }
            catch (const std::exception&)
            {
                break;
            }
        }

        return result;
    }

    glm::ivec3 OBJLoader::ExtractFaceIndices(const std::string& faceStr)
    {
        glm::ivec3 indices(-1, -1, -1); // -1表示未设置
        std::istringstream iss(faceStr);
        std::string token;

        // OBJ面格式：position/texture/normal 或 position//normal 或 position/texture
        if (std::getline(iss, token, '/'))
        {
            indices.x = std::stoi(token); // position index
        }

        if (std::getline(iss, token, '/'))
        {
            if (!token.empty())
            {
                indices.y = std::stoi(token); // texture index
            }
        }

        if (std::getline(iss, token, '/'))
        {
            if (!token.empty())
            {
                indices.z = std::stoi(token); // normal index
            }
        }

        return indices;
    }

    void OBJLoader::Clear()
    {
        m_positions.clear();
        m_texCoords.clear();
        m_normals.clear();
        m_vertices.clear();
        m_indices.clear();
    }

} // namespace Renderer
