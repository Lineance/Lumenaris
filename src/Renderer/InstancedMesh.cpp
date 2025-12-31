#include "Renderer/InstancedMesh.hpp"
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
        if (m_instanceVBO)
        {
            glDeleteBuffers(1, &m_instanceVBO);
            m_instanceVBO = 0;
        }
    }

    void InstancedMesh::SetVertices(const float* vertices, size_t count)
    {
        m_vertices.assign(vertices, vertices + count);
    }

    void InstancedMesh::SetVertexLayout(size_t stride, const std::vector<size_t>& offsets, const std::vector<int>& sizes)
    {
        m_vertexStride = stride;
        m_vertexAttributes.clear();
        for (size_t i = 0; i < offsets.size() && i < sizes.size(); ++i)
        {
            m_vertexAttributes.push_back({offsets[i], sizes[i]});
        }
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

        m_vertexCount = m_vertices.size() / m_vertexStride;

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

        // 上传实例数据
        glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);

        size_t matrixDataSize = m_instanceMatrices.size() * sizeof(glm::mat4);
        size_t colorDataSize = m_instanceColors.size() * sizeof(glm::vec3);
        size_t totalInstanceSize = matrixDataSize + colorDataSize;

        glBufferData(GL_ARRAY_BUFFER, totalInstanceSize, nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, matrixDataSize, m_instanceMatrices.data());
        glBufferSubData(GL_ARRAY_BUFFER, matrixDataSize, colorDataSize, m_instanceColors.data());

        // 设置实例矩阵属性 (location 3, 4, 5, 6)
        // 一个mat4占用4个location
        size_t mat4Stride = sizeof(glm::mat4) / sizeof(glm::vec4);
        for (size_t i = 0; i < 4; ++i)
        {
            glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
            glEnableVertexAttribArray(3 + i);
            glVertexAttribDivisor(3 + i, 1); // 每个实例更新一次
        }

        // 设置实例颜色属性 (location 7)
        size_t colorOffset = matrixDataSize;
        glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)colorOffset);
        glEnableVertexAttribArray(7);
        glVertexAttribDivisor(7, 1); // 每个实例更新一次

        glBindVertexArray(0);

        Core::Logger::GetInstance().Info("Instanced mesh created successfully - VAO: " +
                                         std::to_string(m_vao) + ", VBO: " + std::to_string(m_vbo) +
                                         ", Instance VBO: " + std::to_string(m_instanceVBO));
    }

    void InstancedMesh::Draw() const
    {
        if (m_instances.empty())
        {
            Core::Logger::GetInstance().Warning("InstancedMesh::Draw() called with no instances!");
            return;
        }

        glBindVertexArray(m_vao);
        glDrawArraysInstanced(GL_TRIANGLES, 0, m_vertexCount, m_instances.size());
        glBindVertexArray(0);

        // 计算实际绘制的三角形数量
        size_t triangleCount = (m_vertexCount / 3) * m_instances.size();
        Core::Logger::GetInstance().LogDrawCall(triangleCount);
    }

} // namespace Renderer
