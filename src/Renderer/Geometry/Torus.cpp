#include "Renderer/Geometry/Torus.hpp"
#include "Core/Logger.hpp"
#include <glad/glad.h>
#include <vector>
#include <cmath>

#define PI 3.14159265358979323846f

namespace Renderer
{

    Torus::Torus(float majorRadius, float minorRadius, int majorSegments, int minorSegments)
        : m_majorRadius(majorRadius), m_minorRadius(minorRadius),
          m_majorSegments(majorSegments), m_minorSegments(minorSegments)
    {
    }

    void Torus::Create()
    {
        Core::Logger::GetInstance().Info("Creating torus mesh - Major Radius: " + std::to_string(m_majorRadius) +
                                         ", Minor Radius: " + std::to_string(m_minorRadius) +
                                         ", Major Segments: " + std::to_string(m_majorSegments) +
                                         ", Minor Segments: " + std::to_string(m_minorSegments));

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        float majorStep = 2.0f * PI / m_majorSegments;
        float minorStep = 2.0f * PI / m_minorSegments;

        // ========================================
        // 生成顶点数据
        // ========================================

        for (int i = 0; i <= m_majorSegments; ++i)
        {
            float u = i * majorStep;
            float cosU = cosf(u);
            float sinU = sinf(u);

            for (int j = 0; j <= m_minorSegments; ++j)
            {
                float v = j * minorStep;
                float cosV = cosf(v);
                float sinV = sinf(v);

                // 圆环体参数化方程
                float x = (m_majorRadius + m_minorRadius * cosV) * cosU;
                float y = m_minorRadius * sinV;
                float z = (m_majorRadius + m_minorRadius * cosV) * sinU;

                // 法线（从管中心指向表面）
                float nx = cosV * cosU;
                float ny = sinV;
                float nz = cosV * sinU;

                // UV坐标
                float tx = static_cast<float>(i) / m_majorSegments;
                float ty = static_cast<float>(j) / m_minorSegments;

                // 添加顶点数据: 位置(3) + 法线(3) + UV(2) = 8 floats
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                vertices.push_back(nx);
                vertices.push_back(ny);
                vertices.push_back(nz);
                vertices.push_back(tx);
                vertices.push_back(ty);
            }
        }

        m_vertexCount = vertices.size() / 8;

        // ========================================
        // 生成索引数据
        // ========================================

        for (int i = 0; i < m_majorSegments; ++i)
        {
            for (int j = 0; j < m_minorSegments; ++j)
            {
                int current = i * (m_minorSegments + 1) + j;
                int next = current + m_minorSegments + 1;

                // 第一个三角形
                indices.push_back(current);
                indices.push_back(next);
                indices.push_back(current + 1);

                // 第二个三角形
                indices.push_back(next);
                indices.push_back(next + 1);
                indices.push_back(current + 1);
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

        Core::Logger::GetInstance().Info("Torus mesh created successfully - Vertices: " +
                                         std::to_string(m_vertexCount) + ", Indices: " +
                                         std::to_string(m_indexCount) + ", VAO: " +
                                         std::to_string(m_vao));
    }

    void Torus::Draw() const
    {
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
#if ENABLE_RENDER_STATS
        Core::Logger::GetInstance().LogDrawCall(m_indexCount / 3);
#endif
    }

    glm::mat4 Torus::GetModelMatrix() const
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

    std::vector<float> Torus::GetVertexData(
        float majorRadius,
        float minorRadius,
        int majorSegments,
        int minorSegments)
    {
        // ⭐ 使用传入的参数，而不是硬编码
        std::vector<float> vertices;
        float majorStep = 2.0f * PI / majorSegments;
        float minorStep = 2.0f * PI / minorSegments;

        for (int i = 0; i <= majorSegments; ++i)
        {
            float u = i * majorStep;
            float cosU = cosf(u);
            float sinU = sinf(u);

            for (int j = 0; j <= minorSegments; ++j)
            {
                float v = j * minorStep;
                float cosV = cosf(v);
                float sinV = sinf(v);

                float x = (majorRadius + minorRadius * cosV) * cosU;
                float y = minorRadius * sinV;
                float z = (majorRadius + minorRadius * cosV) * sinU;

                float nx = cosV * cosU;
                float ny = sinV;
                float nz = cosV * sinU;

                float tx = static_cast<float>(i) / majorSegments;
                float ty = static_cast<float>(j) / minorSegments;

                vertices.insert(vertices.end(), {x, y, z, nx, ny, nz, tx, ty});
            }
        }

        return vertices;
    }

    std::vector<unsigned int> Torus::GetIndexData(
        int majorSegments,
        int minorSegments)
    {
        // ⭐ 使用传入的参数，而不是硬编码
        std::vector<unsigned int> indices;

        for (int i = 0; i < majorSegments; ++i)
        {
            for (int j = 0; j < minorSegments; ++j)
            {
                int current = i * (minorSegments + 1) + j;
                int next = current + minorSegments + 1;

                indices.insert(indices.end(), {
                    static_cast<unsigned int>(current),
                    static_cast<unsigned int>(next),
                    static_cast<unsigned int>(current + 1),
                    static_cast<unsigned int>(next),
                    static_cast<unsigned int>(next + 1),
                    static_cast<unsigned int>(current + 1)
                });
            }
        }

        return indices;
    }

    void Torus::GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes)
    {
        offsets = {0, 3, 6};
        sizes = {3, 3, 2};
    }

} // namespace Renderer
