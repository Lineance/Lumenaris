#include "Renderer/Cube.hpp"
#include "Core/Logger.hpp"
#include <glad/glad.h>
#include <vector>

namespace Renderer
{

    void Cube::Create()
    {
        Core::Logger::GetInstance().Info("Creating cube mesh...");
        std::vector<float> vertices;

        // 定义6个面的顶点数据：位置、法线、UV坐标
        // 每个面使用两个三角形，共6个顶点
        struct FaceData {
            glm::vec3 normal;    // 面的法线
            glm::vec3 vertices[4]; // 4个顶点位置（按逆时针顺序）
        };

        FaceData faces[6] = {
            // 前面 (Z负方向)
            {glm::vec3(0.0f, 0.0f, -1.0f), {
                glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f),
                glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, -0.5f)
            }},
            // 后面 (Z正方向)
            {glm::vec3(0.0f, 0.0f, 1.0f), {
                glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f),
                glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f)
            }},
            // 左面 (X负方向)
            {glm::vec3(-1.0f, 0.0f, 0.0f), {
                glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, -0.5f),
                glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, 0.5f)
            }},
            // 右面 (X正方向)
            {glm::vec3(1.0f, 0.0f, 0.0f), {
                glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f),
                glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f)
            }},
            // 下面 (Y负方向)
            {glm::vec3(0.0f, -1.0f, 0.0f), {
                glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f),
                glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f)
            }},
            // 上面 (Y正方向)
            {glm::vec3(0.0f, 1.0f, 0.0f), {
                glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, 0.5f),
                glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, -0.5f)
            }}
        };

        // 定义UV坐标（每个面的4个顶点）
        glm::vec2 uvCoords[4] = {
            glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f)
        };

        // 使用循环生成所有顶点数据
        for (int face = 0; face < 6; ++face) {
            const FaceData& faceData = faces[face];

            // 每个面使用两个三角形，共6个顶点
            // 三角形1: vertices[0], vertices[1], vertices[2]
            // 三角形2: vertices[2], vertices[3], vertices[0]

            // 三角形1
            for (int i = 0; i < 3; ++i) {
                const glm::vec3& pos = (i == 2) ? faceData.vertices[2] : faceData.vertices[i];
                const glm::vec2& uv = (i == 2) ? uvCoords[2] : uvCoords[i];

                // 添加位置、法线、UV坐标
                vertices.insert(vertices.end(), {pos.x, pos.y, pos.z});
                vertices.insert(vertices.end(), {faceData.normal.x, faceData.normal.y, faceData.normal.z});
                vertices.insert(vertices.end(), {uv.x, uv.y});
            }

            // 三角形2
            for (int i = 0; i < 3; ++i) {
                int vertexIndex = (i == 0) ? 2 : (i == 1) ? 3 : 0;
                const glm::vec3& pos = faceData.vertices[vertexIndex];
                const glm::vec2& uv = uvCoords[vertexIndex];

                // 添加位置、法线、UV坐标
                vertices.insert(vertices.end(), {pos.x, pos.y, pos.z});
                vertices.insert(vertices.end(), {faceData.normal.x, faceData.normal.y, faceData.normal.z});
                vertices.insert(vertices.end(), {uv.x, uv.y});
            }
        }

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

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
        Core::Logger::GetInstance().Info("Cube mesh created successfully - Vertices: " +
                                         std::to_string(vertices.size() / 8) + ", VAO: " +
                                         std::to_string(m_vao) + ", VBO: " + std::to_string(m_vbo));
    }

    void Cube::Draw() const
    {
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        Core::Logger::GetInstance().LogDrawCall(12); // 立方体有12个三角形
    }

    glm::mat4 Cube::GetModelMatrix() const
    {
        glm::mat4 model = glm::mat4(1.0f);

        // 应用变换（顺序很重要：缩放 -> 旋转 -> 平移）
        // 或者：平移 -> 旋转 -> 缩放（取决于你想要的旋转中心）
        model = glm::translate(model, m_position);
        model = glm::rotate(model, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)); // X轴旋转
        model = glm::rotate(model, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // Y轴旋转
        model = glm::rotate(model, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)); // Z轴旋转
        model = glm::scale(model, glm::vec3(m_scale));

        return model;
    }

} // namespace Renderer