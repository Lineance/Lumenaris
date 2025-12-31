#include "Renderer/Sphere.hpp"
#include "Core/Logger.hpp"
#include <glad/glad.h>
#include <vector>
#include <cmath>
#define PI 3.14159265358979323846f

namespace Renderer
{

    Sphere::Sphere(float radius, int stacks, int slices)
        : m_radius(radius), m_stacks(stacks), m_slices(slices)
    {
    }

    void Sphere::Create()
    {
        Core::Logger::GetInstance().Info("Creating sphere mesh - Radius: " + std::to_string(m_radius) +
                                         ", Stacks: " + std::to_string(m_stacks) +
                                         ", Slices: " + std::to_string(m_slices));

        // 计算顶点和索引数量
        m_vertexCount = (m_stacks + 1) * (m_slices + 1);
        m_indexCount = m_stacks * m_slices * 6; // 每个面6个索引

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        // 生成顶点数据
        vertices.reserve(m_vertexCount * 8); // 每个顶点8个float (位置3 + 法线3 + UV2)

        for (int stack = 0; stack <= m_stacks; ++stack)
        {
            float phi = PI * stack / m_stacks; // 纬度角度 [0, PI]
            float y = m_radius * cosf(phi);
            float r = m_radius * sinf(phi);

            for (int slice = 0; slice <= m_slices; ++slice)
            {
                float theta = 2.0f * PI * slice / m_slices; // 经度角度 [0, 2PI]

                // 位置
                float x = r * cosf(theta);
                float z = r * sinf(theta);

                // 法线 (归一化位置向量)
                float nx = x / m_radius;
                float ny = y / m_radius;
                float nz = z / m_radius;

                // UV坐标
                float u = static_cast<float>(slice) / m_slices;
                float v = static_cast<float>(stack) / m_stacks;

                // 添加顶点数据
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                vertices.push_back(nx);
                vertices.push_back(ny);
                vertices.push_back(nz);
                vertices.push_back(u);
                vertices.push_back(v);
            }
        }

        // 生成索引数据
        indices.reserve(m_indexCount);

        for (int stack = 0; stack < m_stacks; ++stack)
        {
            for (int slice = 0; slice < m_slices; ++slice)
            {
                int first = stack * (m_slices + 1) + slice;
                int second = first + m_slices + 1;

                // 第一个三角形
                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                // 第二个三角形
                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }

        // 创建OpenGL缓冲对象
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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        // 法线属性 (location = 1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // UV属性 (location = 2)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
        Core::Logger::GetInstance().Info("Sphere mesh created successfully - Vertices: " +
                                         std::to_string(m_vertexCount) + ", Indices: " +
                                         std::to_string(m_indexCount) + ", VAO: " +
                                         std::to_string(m_vao) + ", VBO: " + std::to_string(m_vbo) +
                                         ", EBO: " + std::to_string(m_ebo));
    }

    void Sphere::Draw() const
    {
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        Core::Logger::GetInstance().LogDrawCall(m_indexCount / 3); // 每个三角形3个索引
    }

    glm::mat4 Sphere::GetModelMatrix() const
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, m_position);
        model = glm::scale(model, glm::vec3(m_scale));
        return model;
    }

} // namespace Renderer
