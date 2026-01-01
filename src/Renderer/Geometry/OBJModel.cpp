#include "Renderer/Geometry/OBJModel.hpp"
#include "Core/Logger.hpp"
#include <glad/glad.h>
#include <iostream>

namespace Renderer
{

    OBJModel::OBJModel()
    {
    }

    OBJModel::OBJModel(const std::string& filepath)
    {
        LoadFromFile(filepath);
    }

    OBJModel::~OBJModel()
    {
        Cleanup();
    }

    bool OBJModel::LoadFromFile(const std::string& filepath)
    {
        Core::Logger::GetInstance().Info("Loading OBJ model from: " + filepath);
        m_filepath = filepath;

        if (!m_loader.LoadFromFile(filepath))
        {
            Core::Logger::GetInstance().Error("Failed to load OBJ model: " + filepath);
            return false;
        }

        Core::Logger::GetInstance().Info("OBJ model loaded successfully, vertices: " +
                                        std::to_string(m_loader.GetVertices().size()));

        // 自动加载材质和纹理
        LoadMaterialsAndTextures();

        return true;
    }

    void OBJModel::Create()
    {
        Core::Logger::GetInstance().Info("Creating OpenGL buffers for OBJ model: " + m_filepath);

        if (m_loader.GetVertices().empty())
        {
            Core::Logger::GetInstance().Error("No vertex data loaded. Call LoadFromFile() first.");
            return;
        }

        const auto& vertices = m_loader.GetVertices();

        // 生成OpenGL缓冲区对象
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        // 检查VBO生成是否成功
        if (m_vbo == 0) {
            Core::Logger::GetInstance().Error("Failed to generate VBO!");
            return;
        }

        // 绑定VAO
        glBindVertexArray(m_vao);

        // 绑定并设置VBO数据
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(OBJVertex), vertices.data(), GL_STATIC_DRAW);

        // 检查OpenGL错误
        GLenum error2 = glGetError();
        if (error2 != GL_NO_ERROR) {
            Core::Logger::GetInstance().Error("OpenGL error before setting vertex attributes: " + std::to_string(error2));
        }

        // 设置顶点属性指针
        // 位置属性 (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(OBJVertex), (void*)offsetof(OBJVertex, position));
        glEnableVertexAttribArray(0);

        // 法线属性 (location = 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(OBJVertex), (void*)offsetof(OBJVertex, normal));
        glEnableVertexAttribArray(1);

        // 纹理坐标属性 (location = 2)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(OBJVertex), (void*)offsetof(OBJVertex, texCoord));
        glEnableVertexAttribArray(2);

        // 如果有索引数据，也创建EBO
        if (m_loader.HasIndices())
        {
            glGenBuffers(1, &m_ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
            const auto& indices = m_loader.GetIndices();
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        }

        // 解绑VAO
        glBindVertexArray(0);
    }

    void OBJModel::Draw() const
    {
        DrawWithMaterial(m_currentMaterialIndex); // TODO 这里有必要吗，是不是最好统一 即使是普通问题也应该包含material，只不过会被设定为默认值
    }

    void OBJModel::DrawWithMaterial(int materialIndex) const
    {
        if (m_vao == 0)
        {
            Core::Logger::GetInstance().Error("OBJModel not created. Call Create() first.");
            return;
        }

        // 如果没有材质信息或materialIndex无效，直接绘制整个模型
        const auto& faceMaterials = m_loader.GetFaceMaterialIndices();
        if (faceMaterials.empty() || materialIndex < 0)
        {
            // 绑定纹理（如果有的话）
            // ⭐ 使用纹理单元1（TextureUnit::MATERIAL_DIFFUSE），为ImGui预留单元0
            if (HasTexture() && materialIndex >= 0 && materialIndex < static_cast<int>(m_textures.size()))
            {
                m_textures[materialIndex].Bind(GL_TEXTURE1);
            }

            // 绑定VAO并绘制整个模型
            glBindVertexArray(m_vao);

            if (m_loader.HasIndices())
            {
                const auto& indices = m_loader.GetIndices();
                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
            }
            else
            {
                const auto& vertices = m_loader.GetVertices();
                glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
            }

            glBindVertexArray(0);

            // 解绑纹理
            if (HasTexture())
            {
                Texture::UnbindStatic();
            }
            return;
        }

        // 分材质渲染 - 遍历所有三角形，只渲染使用指定材质的
        glBindVertexArray(m_vao);

        if (m_loader.HasIndices())
        {
            const auto& indices = m_loader.GetIndices();

            // 绑定纹理
            // ⭐ 使用纹理单元1（TextureUnit::MATERIAL_DIFFUSE），为ImGui预留单元0
            if (HasTexture() && materialIndex >= 0 && materialIndex < static_cast<int>(m_textures.size()))
            {
                m_textures[materialIndex].Bind(GL_TEXTURE1);
            }

            // 遍历每个三角形（3个索引为一组）
            for (size_t i = 0; i < indices.size(); i += 3)
            {
                // 计算对应的face索引（每个face对应一个三角形）
                size_t faceIndex = i / 3;

                // 检查这个face是否使用指定的材质
                if (faceIndex < faceMaterials.size() && faceMaterials[faceIndex] == materialIndex)
                {
                    // 渲染这个三角形
                    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT,
                                 (const void*)(i * sizeof(unsigned int)));
                }
            }
        }

        glBindVertexArray(0);

        // 解绑纹理
        if (HasTexture())
        {
            Texture::UnbindStatic();
        }
    }

    glm::mat4 OBJModel::GetModelMatrix() const
    {
        glm::mat4 model = glm::mat4(1.0f);

        // 应用变换（顺序很重要：缩放 -> 旋转 -> 平移）
        model = glm::translate(model, m_position);
        model = glm::rotate(model, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)); // X轴旋转
        model = glm::rotate(model, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // Y轴旋转
        model = glm::rotate(model, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)); // Z轴旋转
        model = glm::scale(model, glm::vec3(m_scale));

        return model;
    }

    void OBJModel::LoadMaterialsAndTextures()
    {
        if (m_materialsLoaded)
            return;

        const auto& materials = m_loader.GetMaterials();
        if (materials.empty())
        {
            Core::Logger::GetInstance().Info("No materials found in OBJ file: " + m_filepath);
            m_materialsLoaded = true;
            return;
        }

        Core::Logger::GetInstance().Info("Loading materials and textures for " + std::to_string(materials.size()) + " materials");

        // 为每个材质加载纹理
        m_textures.resize(materials.size());

        for (size_t i = 0; i < materials.size(); ++i)
        {
            const auto& material = materials[i];

            // 尝试加载漫反射纹理
            if (!material.diffuseTexname.empty())
            {
                std::string texPath = m_loader.GetBasePath() + "/" + material.diffuseTexname;
                if (m_textures[i].LoadFromFile(texPath))
                {
                    // Texture loaded successfully
                }
                else
                {
                    Core::Logger::GetInstance().Warning("Failed to load texture for material '" + material.name + "': " + texPath);
                }
            }
        }

        m_materialsLoaded = true;
        Core::Logger::GetInstance().Info("Loaded " + std::to_string(materials.size()) + " materials with textures");
    }

    void OBJModel::SetCurrentMaterialIndex(int index)
    {
        const auto& materials = m_loader.GetMaterials();
        if (index >= 0 && index < static_cast<int>(materials.size()))
        {
            m_currentMaterialIndex = index;
        }
        else
        {
            Core::Logger::GetInstance().Warning("Invalid material index: " + std::to_string(index) +
                                               " (valid range: 0-" + std::to_string(materials.size() - 1) + ")");
        }
    }

    bool OBJModel::HasTexture() const
    {
        if (m_currentMaterialIndex < 0 || m_currentMaterialIndex >= static_cast<int>(m_textures.size()))
            return false;

        return m_textures[m_currentMaterialIndex].IsLoaded();
    }

    const OBJMaterial* OBJModel::GetCurrentMaterial() const
    {
        const auto& materials = m_loader.GetMaterials();
        if (m_currentMaterialIndex >= 0 && m_currentMaterialIndex < static_cast<int>(materials.size()))
        {
            return &materials[m_currentMaterialIndex];
        }
        return nullptr;
    }

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

    void OBJModel::Cleanup()
    {
        if (m_ebo != 0)
        {
            glDeleteBuffers(1, &m_ebo);
            m_ebo = 0;
        }
        if (m_vbo != 0)
        {
            glDeleteBuffers(1, &m_vbo);
            m_vbo = 0;
        }
        if (m_vao != 0)
        {
            glDeleteVertexArrays(1, &m_vao);
            m_vao = 0;
        }

        // 清理纹理
        m_textures.clear();
        m_materialsLoaded = false;
    }

} // namespace Renderer
