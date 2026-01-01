#include "Renderer/Geometry/Plane.hpp"
#include "Core/Logger.hpp"
#include <glad/glad.h>
#include <vector>

namespace Renderer
{

    Plane::Plane(float width, float height, int widthSegments, int heightSegments)
        : m_width(width), m_height(height),
          m_widthSegments(widthSegments), m_heightSegments(heightSegments)
    {
    }

    void Plane::Create()
    {
        Core::Logger::GetInstance().Info("Creating plane mesh - Width: " + std::to_string(m_width) +
                                         ", Height: " + std::to_string(m_height) +
                                         ", Width Segments: " + std::to_string(m_widthSegments) +
                                         ", Height Segments: " + std::to_string(m_heightSegments));

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        float halfWidth = m_width * 0.5f;
        float halfHeight = m_height * 0.5f;
        float widthStep = m_width / m_widthSegments;
        float heightStep = m_height / m_heightSegments;

        // ========================================
        // 生成顶点数据
        // ========================================

        for (int y = 0; y <= m_heightSegments; ++y)
        {
            float yPos = -halfHeight + y * heightStep;
            float v = static_cast<float>(y) / m_heightSegments;

            for (int x = 0; x <= m_widthSegments; ++x)
            {
                float xPos = -halfWidth + x * widthStep;
                float u = static_cast<float>(x) / m_widthSegments;

                // 位置（XY平面，Z=0）
                vertices.push_back(xPos);
                vertices.push_back(yPos);
                vertices.push_back(0.0f);

                // 法线（指向+Z方向）
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);

                // UV坐标
                vertices.push_back(u);
                vertices.push_back(v);
            }
        }

        m_vertexCount = vertices.size() / 8;

        // ========================================
        // 生成索引数据
        // ========================================

        for (int y = 0; y < m_heightSegments; ++y)
        {
            for (int x = 0; x < m_widthSegments; ++x)
            {
                int topLeft = y * (m_widthSegments + 1) + x;
                int topRight = topLeft + 1;
                int bottomLeft = (y + 1) * (m_widthSegments + 1) + x;
                int bottomRight = bottomLeft + 1;

                // 第一个三角形
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                // 第二个三角形
                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        m_indexCount = indices.size();

        // ========================================
        // 创建OpenGL缓冲对象
        // ========================================

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);

        glBindVertexArray(m_vao);

        // 顶点缓冲
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        // 索引缓冲
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // 位置属性 (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // 法线属性 (location = 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // UV属性 (location = 2)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);

        Core::Logger::GetInstance().Info("Plane mesh created successfully - Vertices: " +
                                         std::to_string(m_vertexCount) + ", Indices: " +
                                         std::to_string(m_indexCount) + ", VAO: " +
                                         std::to_string(m_vao));
    }

    void Plane::Draw() const
    {
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
#if ENABLE_RENDER_STATS
        Core::Logger::GetInstance().LogDrawCall(m_indexCount / 3);
#endif
    }

    glm::mat4 Plane::GetModelMatrix() const
    {
        glm::mat4 model = glm::mat4(1.0f);

        // 应用变换（缩放 -> 旋转 -> 平移）
        model = glm::translate(model, m_position);
        model = glm::rotate(model, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(m_scale));

        return model;
    }

    std::vector<float> Plane::GetVertexData()
    {
        const float width = 1.0f;
        const float height = 1.0f;
        const int widthSegments = 1;
        const int heightSegments = 1;

        std::vector<float> vertices;
        float halfWidth = width * 0.5f;
        float halfHeight = height * 0.5f;

        for (int y = 0; y <= heightSegments; ++y)
        {
            float yPos = -halfHeight + y * height;
            float v = static_cast<float>(y) / heightSegments;

            for (int x = 0; x <= widthSegments; ++x)
            {
                float xPos = -halfWidth + x * width;
                float u = static_cast<float>(x) / widthSegments;

                vertices.insert(vertices.end(), {xPos, yPos, 0.0f, 0.0f, 0.0f, 1.0f, u, v});
            }
        }

        return vertices;
    }

    std::vector<unsigned int> Plane::GetIndexData()
    {
        const int widthSegments = 1;
        const int heightSegments = 1;

        std::vector<unsigned int> indices;

        for (int y = 0; y < heightSegments; ++y)
        {
            for (int x = 0; x < widthSegments; ++x)
            {
                int topLeft = y * (widthSegments + 1) + x;
                int topRight = topLeft + 1;
                int bottomLeft = (y + 1) * (widthSegments + 1) + x;
                int bottomRight = bottomLeft + 1;

                indices.insert(indices.end(), {
                    static_cast<unsigned int>(topLeft),
                    static_cast<unsigned int>(bottomLeft),
                    static_cast<unsigned int>(topRight),
                    static_cast<unsigned int>(topRight),
                    static_cast<unsigned int>(bottomLeft),
                    static_cast<unsigned int>(bottomRight)
                });
            }
        }

        return indices;
    }

    void Plane::GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes)
    {
        offsets = {0, 3, 6};
        sizes = {3, 3, 2};
    }

} // namespace Renderer
