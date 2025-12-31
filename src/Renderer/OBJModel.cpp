#include "Renderer/OBJModel.hpp"
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
        m_filepath = filepath;
        if (!m_loader.LoadFromFile(filepath))
        {
            std::cerr << "Failed to load OBJ model: " << filepath << std::endl;
            return false;
        }

        // 自动加载材质和纹理
        LoadMaterialsAndTextures();

        return true;
    }

    void OBJModel::Create()
    {
        if (m_loader.GetVertices().empty())
        {
            std::cerr << "No vertex data loaded. Call LoadFromFile() first." << std::endl;
            return;
        }

        const auto& vertices = m_loader.GetVertices();

        // 生成OpenGL缓冲区对象
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        // 检查VBO生成是否成功
        if (m_vbo == 0) {
            std::cerr << "Failed to generate VBO!" << std::endl;
            return;
        }

        // 绑定VAO
        glBindVertexArray(m_vao);
        std::cout << "Bound VAO: " << m_vao << std::endl;

        // 绑定并设置VBO数据
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(OBJVertex), vertices.data(), GL_STATIC_DRAW);

        // 检查OpenGL错误
        GLenum error2 = glGetError();
        if (error2 != GL_NO_ERROR) {
            std::cerr << "OpenGL error before setting vertex attributes: " << error2 << std::endl;
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
            std::cerr << "OBJModel not created. Call Create() first." << std::endl;
            return;
        }

        // 如果没有材质信息或materialIndex无效，直接绘制整个模型
        const auto& faceMaterials = m_loader.GetFaceMaterialIndices();
        if (faceMaterials.empty() || materialIndex < 0)
        {
            // 绑定纹理（如果有的话）
            if (HasTexture() && materialIndex >= 0 && materialIndex < static_cast<int>(m_textures.size()))
            {
                m_textures[materialIndex].Bind(GL_TEXTURE0);
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
            if (HasTexture() && materialIndex >= 0 && materialIndex < static_cast<int>(m_textures.size()))
            {
                m_textures[materialIndex].Bind(GL_TEXTURE0);
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
            std::cout << "No materials found in OBJ file" << std::endl;
            m_materialsLoaded = true;
            return;
        }

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
                    std::cout << "Loaded texture for material '" << material.name << "': " << texPath << std::endl;
                }
                else
                {
                    std::cerr << "Failed to load texture for material '" << material.name << "': " << texPath << std::endl;
                }
            }
        }

        m_materialsLoaded = true;
        std::cout << "Loaded " << materials.size() << " materials with textures" << std::endl;
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
            std::cerr << "Invalid material index: " << index << std::endl;
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
