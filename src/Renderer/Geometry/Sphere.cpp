#include "Renderer/Geometry/Sphere.hpp"
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
#if ENABLE_RENDER_STATS
        Core::Logger::GetInstance().LogDrawCall(m_indexCount / 3); // 每个三角形3个索引
#endif
    }

    glm::mat4 Sphere::GetModelMatrix() const
    {
        glm::mat4 model = glm::mat4(1.0f);

        // 应用变换（顺序：缩放 -> 旋转 -> 平移）
        model = glm::translate(model, m_position);
        model = glm::scale(model, glm::vec3(m_scale));

        return model;
    }

    std::vector<float> Sphere::GetVertexData()
    {
        // 返回球体的标准顶点数据（使用默认参数：radius=1.0f, stacks=32, slices=32）
        const int stacks = 32;
        const int slices = 32;
        const float radius = 1.0f;

        std::vector<float> vertices;

        // 生成顶点数据
        for (int stack = 0; stack <= stacks; ++stack)
        {
            float phi = PI * stack / stacks; // 纬度角度 [0, PI]
            float y = radius * cosf(phi);
            float r = radius * sinf(phi);

            for (int slice = 0; slice <= slices; ++slice)
            {
                float theta = 2.0f * PI * slice / slices; // 经度角度 [0, 2PI]

                // 位置
                float x = r * cosf(theta);
                float z = r * sinf(theta);

                // 法线 (归一化位置向量)
                float nx = x / radius;
                float ny = y / radius;
                float nz = z / radius;

                // UV坐标
                float u = static_cast<float>(slice) / slices;
                float v = static_cast<float>(stack) / stacks;

                // 添加顶点数据: 位置(3) + 法线(3) + UV(2) = 8 floats
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

        return vertices;
    }

    std::vector<unsigned int> Sphere::GetIndexData()
    {
        // 返回球体的标准索引数据（使用默认参数：stacks=32, slices=32）
        const int stacks = 32;
        const int slices = 32;

        std::vector<unsigned int> indices;

        // 生成索引数据
        for (int stack = 0; stack < stacks; ++stack)
        {
            for (int slice = 0; slice < slices; ++slice)
            {
                int first = stack * (slices + 1) + slice;
                int second = first + slices + 1;

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

        return indices;
    }

    void Sphere::GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes)
    {
        // 球体顶点布局：位置(偏移0, 大小3) + 法线(偏移3, 大小3) + UV(偏移6, 大小2)
        offsets = {0, 3, 6};
        sizes = {3, 3, 2};
    }

} // namespace Renderer
