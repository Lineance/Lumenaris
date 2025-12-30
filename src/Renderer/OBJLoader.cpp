#define TINYOBJLOADER_IMPLEMENTATION
#include "Renderer/OBJLoader.hpp"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace Renderer
{

    OBJLoader::OBJLoader()
        : m_loaded(false)
    {
    }

    OBJLoader::~OBJLoader()
    {
        Clear();
    }

    bool OBJLoader::LoadFromFile(const std::string& filepath)
    {
        Clear();

        // 获取文件所在目录，用于加载材质文件
        fs::path path(filepath);
        m_basePath = path.parent_path().string();

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string err;

        // 使用tinyobjloader加载OBJ文件
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
                                   filepath.c_str(), m_basePath.c_str(), true);

        // 检查错误和警告信息（tinyobj将警告也包含在err中）
        if (!err.empty()) {
            // 检查是否包含警告信息
            if (err.find("WARN") != std::string::npos) {
                std::cout << "OBJ Loader Warning: " << err << std::endl;
            } else {
                std::cerr << "OBJ Loader Error: " << err << std::endl;
            }
        }

        if (!ret) {
            std::cerr << "Failed to load OBJ file: " << filepath << std::endl;
            return false;
        }

        // 转换tinyobj数据为我们的格式
        ConvertTinyObjData(attrib, shapes, materials);
        ConvertMaterials(materials);

        m_loaded = true;

        // OBJ文件加载成功

        return true;
    }

    void OBJLoader::ConvertTinyObjData(const tinyobj::attrib_t& attrib,
                                      const std::vector<tinyobj::shape_t>& shapes,
                                      const std::vector<tinyobj::material_t>& materials)
    {
        // 清空之前的数据
        m_vertices.clear();
        m_indices.clear();

        // 用于避免顶点重复的映射
        std::map<std::tuple<int, int, int>, unsigned int> vertexMap;

        // 处理每个shape
        for (const auto& shape : shapes) {
            size_t index_offset = 0;

            // 处理每个face
            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
                int fv = shape.mesh.num_face_vertices[f];

                // 只处理三角形（fv == 3）或将四边形转换为三角形
                if (fv == 3 || fv == 4) {
                    // 为每个三角形创建顶点
                    std::vector<unsigned int> faceIndices;

                    for (size_t v = 0; v < fv; v++) {
                        tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                        // 创建顶点键（位置、法线、纹理坐标索引）
                        auto key = std::make_tuple(idx.vertex_index, idx.normal_index, idx.texcoord_index);

                        // 检查是否已经创建过这个顶点
                        auto it = vertexMap.find(key);
                        if (it != vertexMap.end()) {
                            // 已经存在的顶点，直接使用索引
                            faceIndices.push_back(it->second);
                        } else {
                            // 创建新顶点
                            OBJVertex vertex;

                            // 位置（tinyobj使用float数组，每个顶点3个float）
                            if (idx.vertex_index >= 0) {
                                vertex.position = glm::vec3(
                                    attrib.vertices[3 * idx.vertex_index + 0],
                                    attrib.vertices[3 * idx.vertex_index + 1],
                                    attrib.vertices[3 * idx.vertex_index + 2]
                                );
                            }

                            // 法线
                            if (idx.normal_index >= 0) {
                                vertex.normal = glm::vec3(
                                    attrib.normals[3 * idx.normal_index + 0],
                                    attrib.normals[3 * idx.normal_index + 1],
                                    attrib.normals[3 * idx.normal_index + 2]
                                );
                            } else {
                                // 如果没有法线，设置为默认值
                                vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                            }

                            // 纹理坐标（tinyobj使用float数组，每个纹理坐标2个float）
                            if (idx.texcoord_index >= 0) {
                                vertex.texCoord = glm::vec2(
                                    attrib.texcoords[2 * idx.texcoord_index + 0],
                                    attrib.texcoords[2 * idx.texcoord_index + 1]
                                );
                            } else {
                                vertex.texCoord = glm::vec2(0.0f, 0.0f);
                            }

                            // 添加顶点到数组
                            unsigned int newIndex = static_cast<unsigned int>(m_vertices.size());
                            m_vertices.push_back(vertex);
                            vertexMap[key] = newIndex;
                            faceIndices.push_back(newIndex);
                        }
                    }

                    // 如果是三角形，直接添加索引
                    if (fv == 3) {
                        m_indices.insert(m_indices.end(), faceIndices.begin(), faceIndices.end());
                    }
                    // 如果是四边形，拆分为两个三角形
                    else if (fv == 4) {
                        m_indices.push_back(faceIndices[0]);
                        m_indices.push_back(faceIndices[1]);
                        m_indices.push_back(faceIndices[2]);

                        m_indices.push_back(faceIndices[2]);
                        m_indices.push_back(faceIndices[3]);
                        m_indices.push_back(faceIndices[0]);
                    }
                }

                index_offset += fv;
            }
        }
    }

    void OBJLoader::ConvertMaterials(const std::vector<tinyobj::material_t>& tinyMaterials)
    {
        m_materials.clear();

        for (const auto& tinyMat : tinyMaterials) {
            OBJMaterial mat;

            mat.name = tinyMat.name;

            // 转换颜色值
            mat.ambient = glm::vec3(tinyMat.ambient[0], tinyMat.ambient[1], tinyMat.ambient[2]);
            mat.diffuse = glm::vec3(tinyMat.diffuse[0], tinyMat.diffuse[1], tinyMat.diffuse[2]);
            mat.specular = glm::vec3(tinyMat.specular[0], tinyMat.specular[1], tinyMat.specular[2]);

            mat.shininess = tinyMat.shininess;
            mat.dissolve = tinyMat.dissolve;

            // 纹理文件名
            mat.ambientTexname = tinyMat.ambient_texname;
            mat.diffuseTexname = tinyMat.diffuse_texname;
            mat.specularTexname = tinyMat.specular_texname;
            mat.normalTexname = tinyMat.normal_texname;

            m_materials.push_back(mat);
        }
    }

    void OBJLoader::Clear()
    {
        m_vertices.clear();
        m_indices.clear();
        m_materials.clear();
        m_loaded = false;
        m_basePath.clear();
    }

} // namespace Renderer
