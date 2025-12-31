#include "Renderer/InstancedMesh.hpp"
#include "Renderer/OBJLoader.hpp"
#include "Core/Logger.hpp"
#include <glad/glad.h>

namespace Renderer
{

    InstancedMesh::~InstancedMesh()
    {
        // 清理OpenGL资源
        if (m_vao)
        {
            glDeleteVertexArrays(1, &m_vao);
            m_vao = 0;
        }
        if (m_vbo)
        {
            glDeleteBuffers(1, &m_vbo);
            m_vbo = 0;
        }
        if (m_ebo)
        {
            glDeleteBuffers(1, &m_ebo);
            m_ebo = 0;
        }
        if (m_instanceVBO)
        {
            glDeleteBuffers(1, &m_instanceVBO);
            m_instanceVBO = 0;
        }

        // 清理纹理
        if (m_texture != nullptr)
        {
            delete m_texture;
            m_texture = nullptr;
        }
    }

    void InstancedMesh::SetVertexData(const float* vertices, size_t vertexCount, size_t stride)
    {
        m_vertices.assign(vertices, vertices + vertexCount * stride);
        m_vertexStride = stride;
        m_vertexCount = vertexCount;
    }

    void InstancedMesh::SetVertexLayout(const std::vector<size_t>& offsets, const std::vector<int>& sizes)
    {
        m_vertexAttributes.clear();
        for (size_t i = 0; i < offsets.size() && i < sizes.size(); ++i)
        {
            m_vertexAttributes.push_back({offsets[i], sizes[i]});
        }
    }

    void InstancedMesh::SetIndexData(const unsigned int* indices, size_t indexCount)
    {
        m_indices.assign(indices, indices + indexCount);
        m_indexCount = indexCount;
        m_hasIndices = true;
    }

    void InstancedMesh::SetTexture(Texture* texture)
    {
        m_texture = texture;
    }

    void InstancedMesh::AddInstance(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale, const glm::vec3& color)
    {
        InstanceData instance;
        instance.color = color;

        // 计算模型矩阵
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);

        instance.modelMatrix = model;
        m_instances.push_back(instance);
        m_instanceMatrices.push_back(model);
        m_instanceColors.push_back(color);
    }

    void InstancedMesh::ClearInstances()
    {
        m_instances.clear();
        m_instanceMatrices.clear();
        m_instanceColors.clear();
    }

    void InstancedMesh::SetInstances(const std::vector<InstanceData>& instances)
    {
        m_instances = instances;
        m_instanceMatrices.clear();
        m_instanceColors.clear();

        for (const auto& instance : m_instances)
        {
            m_instanceMatrices.push_back(instance.modelMatrix);
            m_instanceColors.push_back(instance.color);
        }
    }

    void InstancedMesh::UpdateInstanceBuffers()
    {
        if (!m_instanceVBO || m_instances.empty())
            return;

        glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);

        // 更新矩阵数据
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_instanceMatrices.size() * sizeof(glm::mat4), m_instanceMatrices.data());

        // 更新颜色数据
        size_t matrixDataSize = m_instanceMatrices.size() * sizeof(glm::mat4);
        glBufferSubData(GL_ARRAY_BUFFER, matrixDataSize, m_instanceColors.size() * sizeof(glm::vec3), m_instanceColors.data());

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void InstancedMesh::Create()
    {
        if (m_vertices.empty())
        {
            Core::Logger::GetInstance().Error("InstancedMesh::Create() called with no vertices!");
            return;
        }

        Core::Logger::GetInstance().Info("Creating instanced mesh with " +
                                         std::to_string(m_instances.size()) + " instances and " +
                                         std::to_string(m_vertexCount) + " vertices per instance...");

        // 创建VAO和VBO
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_instanceVBO);

        glBindVertexArray(m_vao);

        // 上传顶点数据
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), m_vertices.data(), GL_STATIC_DRAW);

        // 设置顶点属性
        for (size_t i = 0; i < m_vertexAttributes.size(); ++i)
        {
            const auto& attr = m_vertexAttributes[i];
            glVertexAttribPointer(i, attr.size, GL_FLOAT, GL_FALSE, m_vertexStride * sizeof(float), (void*)(attr.offset * sizeof(float)));
            glEnableVertexAttribArray(i);
        }

        // 如果有索引数据，创建EBO
        if (m_hasIndices && !m_indices.empty())
        {
            glGenBuffers(1, &m_ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
            Core::Logger::GetInstance().Info("Created EBO with " + std::to_string(m_indices.size()) + " indices");
        }

        // 上传实例数据
        glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);

        size_t matrixDataSize = m_instanceMatrices.size() * sizeof(glm::mat4);
        size_t colorDataSize = m_instanceColors.size() * sizeof(glm::vec3);
        size_t totalInstanceSize = matrixDataSize + colorDataSize;

        glBufferData(GL_ARRAY_BUFFER, totalInstanceSize, nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, matrixDataSize, m_instanceMatrices.data());
        glBufferSubData(GL_ARRAY_BUFFER, matrixDataSize, colorDataSize, m_instanceColors.data());

        // 设置实例矩阵属性 (location 3, 4, 5, 6)
        for (size_t i = 0; i < 4; ++i)
        {
            glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
            glEnableVertexAttribArray(3 + i);
            glVertexAttribDivisor(3 + i, 1);
        }

        // 设置实例颜色属性 (location 7)
        size_t colorOffset = matrixDataSize;
        glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)colorOffset);
        glEnableVertexAttribArray(7);
        glVertexAttribDivisor(7, 1);

        glBindVertexArray(0);

        Core::Logger::GetInstance().Info("Instanced mesh created successfully - VAO: " +
                                         std::to_string(m_vao) + ", VBO: " + std::to_string(m_vbo) +
                                         ", Instance VBO: " + std::to_string(m_instanceVBO) +
                                         (m_hasIndices ? ", EBO: " + std::to_string(m_ebo) : ""));
    }

    void InstancedMesh::Draw() const
    {
        if (m_instances.empty())
        {
            Core::Logger::GetInstance().Warning("InstancedMesh::Draw() called with no instances!");
            return;
        }

        // 绑定纹理（如果有）
        if (m_texture != nullptr)
        {
            m_texture->Bind(GL_TEXTURE0);
        }

        glBindVertexArray(m_vao);

        if (m_hasIndices)
        {
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_INT, 0, static_cast<GLsizei>(m_instances.size()));
        }
        else
        {
            glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertexCount), static_cast<GLsizei>(m_instances.size()));
        }

        glBindVertexArray(0);

        // 解绑纹理
        if (m_texture != nullptr)
        {
            Texture::UnbindStatic();
        }

        size_t triangleCount = ((m_hasIndices ? m_indexCount : m_vertexCount) / 3) * m_instances.size();
        Core::Logger::GetInstance().LogDrawCall(triangleCount);
    }

    // 静态方法：从 Cube 创建实例化网格
    InstancedMesh InstancedMesh::CreateFromCube(size_t instanceCount)
    {
        InstancedMesh instancedMesh;

        // 立方体的标准顶点数据（36个顶点，每个顶点8个float：位置+法线+UV）
        std::vector<float> cubeVertices = {
            // 前面
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

            // 后面
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,

            // 左面
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

            // 右面
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

            // 下面
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

            // 上面
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f
        };

        instancedMesh.SetVertexData(cubeVertices.data(), 36, 8);
        instancedMesh.SetVertexLayout({0, 3, 6}, {3, 3, 2});

        Core::Logger::GetInstance().Info("Created instanced mesh from Cube template");

        return instancedMesh;
    }

    // 静态方法：从 OBJ 文件创建实例化网格（返回多个网格，每个材质一个）
    std::vector<InstancedMesh> InstancedMesh::CreateFromOBJ(const std::string& objPath, size_t instanceCount)
    {
        std::vector<InstancedMesh> instancedMeshes;

        // 直接使用 OBJLoader 加载数据
        OBJLoader loader;

        if (!loader.LoadFromFile(objPath))
        {
            Core::Logger::GetInstance().Error("Failed to load OBJ file for instanced mesh: " + objPath);
            return instancedMeshes;
        }

        const auto& materials = loader.GetMaterials();
        const auto& vertices = loader.GetVertices();
        const auto& indices = loader.GetIndices();
        const auto& faceMaterialIndices = loader.GetFaceMaterialIndices();

        if (vertices.empty())
        {
            Core::Logger::GetInstance().Error("OBJ file has no vertices: " + objPath);
            return instancedMeshes;
        }

        Core::Logger::GetInstance().Info("Creating instanced meshes from OBJ: " + objPath +
                                         " with " + std::to_string(materials.size()) + " materials");

        // 如果没有材质，创建单个网格
        if (materials.empty())
        {
            InstancedMesh mesh;

            // 转换顶点数据
            std::vector<float> expandedVertices;
            expandedVertices.reserve(vertices.size() * 8);
            for (const auto& vertex : vertices)
            {
                expandedVertices.push_back(vertex.position.x);
                expandedVertices.push_back(vertex.position.y);
                expandedVertices.push_back(vertex.position.z);
                expandedVertices.push_back(vertex.normal.x);
                expandedVertices.push_back(vertex.normal.y);
                expandedVertices.push_back(vertex.normal.z);
                expandedVertices.push_back(vertex.texCoord.x);
                expandedVertices.push_back(vertex.texCoord.y);
            }

            mesh.SetVertexData(expandedVertices.data(), vertices.size(), 8);
            mesh.SetVertexLayout({0, 3, 6}, {3, 3, 2});

            // 如果有索引，设置索引数据
            if (!indices.empty())
            {
                mesh.SetIndexData(indices.data(), indices.size());
            }

            instancedMeshes.push_back(std::move(mesh));
        }
        else
        {
            // 为每个材质创建一个实例化网格
            for (size_t matIdx = 0; matIdx < materials.size(); ++matIdx)
            {
                InstancedMesh mesh;

                // 收集使用此材质的所有面的索引
                std::vector<unsigned int> materialIndices;

                for (size_t faceIdx = 0; faceIdx < faceMaterialIndices.size(); ++faceIdx)
                {
                    if (faceMaterialIndices[faceIdx] == static_cast<int>(matIdx))
                    {
                        // 这个面使用当前材质，添加其3个索引
                        size_t idxStart = faceIdx * 3;
                        if (idxStart + 2 < indices.size())
                        {
                            materialIndices.push_back(indices[idxStart]);
                            materialIndices.push_back(indices[idxStart + 1]);
                            materialIndices.push_back(indices[idxStart + 2]);
                        }
                    }
                }

                if (materialIndices.empty())
                {
                    continue; // 此材质没有使用任何面，跳过
                }

                // 转换所有顶点数据
                std::vector<float> expandedVertices;
                expandedVertices.reserve(vertices.size() * 8);
                for (const auto& vertex : vertices)
                {
                    expandedVertices.push_back(vertex.position.x);
                    expandedVertices.push_back(vertex.position.y);
                    expandedVertices.push_back(vertex.position.z);
                    expandedVertices.push_back(vertex.normal.x);
                    expandedVertices.push_back(vertex.normal.y);
                    expandedVertices.push_back(vertex.normal.z);
                    expandedVertices.push_back(vertex.texCoord.x);
                    expandedVertices.push_back(vertex.texCoord.y);
                }

                mesh.SetVertexData(expandedVertices.data(), vertices.size(), 8);
                mesh.SetVertexLayout({0, 3, 6}, {3, 3, 2});
                mesh.SetIndexData(materialIndices.data(), materialIndices.size());

                // 设置材质颜色
                mesh.SetMaterialColor(materials[matIdx].diffuse);

                // 加载并设置纹理
                const std::string& textureName = materials[matIdx].diffuseTexname;
                if (!textureName.empty())
                {
                    // 创建纹理并存储（注意：这里需要管理纹理生命周期）
                    Texture* texture = new Texture();

                    // 构建完整的纹理路径（基础路径 + 纹理文件名）
                    std::string texturePath = loader.GetBasePath() + textureName;

                    if (texture->LoadFromFile(texturePath))
                    {
                        mesh.SetTexture(texture);
                        Core::Logger::GetInstance().Info("Loaded texture for material " + std::to_string(matIdx) + ": " + texturePath);
                    }
                    else
                    {
                        delete texture;
                        texture = nullptr;
                        Core::Logger::GetInstance().Warning("Failed to load texture: " + texturePath);
                    }
                }

                instancedMeshes.push_back(std::move(mesh));

                Core::Logger::GetInstance().Info("Created instanced mesh for material " + std::to_string(matIdx) +
                                                 " with " + std::to_string(materialIndices.size()) + " indices");
            }
        }

        return instancedMeshes;
    }

} // namespace Renderer
