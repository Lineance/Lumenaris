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

        // 绑定VAO
        glBindVertexArray(m_vao);

        // 绑定并设置VBO数据
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(OBJVertex), vertices.data(), GL_STATIC_DRAW);

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

        std::cout << "OBJModel created successfully. Vertices: " << vertices.size() << std::endl;
    }

    void OBJModel::Draw() const
    {
        if (m_vao == 0)
        {
            std::cerr << "OBJModel not created. Call Create() first." << std::endl;
            return;
        }

        glBindVertexArray(m_vao);

        if (m_loader.HasIndices())
        {
            // 使用索引绘制
            const auto& indices = m_loader.GetIndices();
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        }
        else
        {
            // 使用顶点数组绘制
            const auto& vertices = m_loader.GetVertices();
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
        }

        glBindVertexArray(0);
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
    }

} // namespace Renderer
