#include "Renderer/Data/MeshBuffer.hpp"
#include "Core/Logger.hpp"
#include <glad/glad.h>

namespace Renderer
{

    MeshBuffer::~MeshBuffer()
    {
        ReleaseGPU();
    }

    // ============================================================
    // 拷贝语义（已删除，防止误用）
    // ============================================================
    // 拷贝构造函数和拷贝赋值运算符已声明为 delete，无实现

    // ============================================================
    // 移动语义
    // ============================================================

    MeshBuffer::MeshBuffer(MeshBuffer&& other) noexcept
        : m_data(std::move(other.m_data)),
          m_vao(other.m_vao),
          m_vbo(other.m_vbo),
          m_ebo(other.m_ebo),
          m_texture(std::move(other.m_texture))
    {
        // 清空源对象
        other.m_vao = 0;
        other.m_vbo = 0;
        other.m_ebo = 0;
    }

    MeshBuffer& MeshBuffer::operator=(MeshBuffer&& other) noexcept
    {
        if (this != &other)
        {
            // 释放旧资源
            ReleaseGPU();

            // 转移资源
            m_data = std::move(other.m_data);
            m_vao = other.m_vao;
            m_vbo = other.m_vbo;
            m_ebo = other.m_ebo;
            m_texture = std::move(other.m_texture);

            // 清空源对象
            other.m_vao = 0;
            other.m_vbo = 0;
            other.m_ebo = 0;
        }
        return *this;
    }

    // ============================================================
    // GPU 操作
    // ============================================================

    void MeshBuffer::UploadToGPU(const MeshData& data)
    {
        // 保存数据副本（拷贝）
        m_data = data;

        // 如果已有 GPU 资源，先释放
        if (m_vao != 0)
        {
            ReleaseGPU();
        }

        // 创建新的 GPU 资源
        CreateVAO();
        UploadVertexData();

        if (m_data.HasIndices())
        {
            UploadIndexData();
        }

        SetupVertexAttributes();

        // 解绑
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        Core::Logger::GetInstance().Info("MeshBuffer::UploadToGPU() - Uploaded mesh to GPU: " +
                                         std::to_string(m_data.GetVertexCount()) + " vertices, " +
                                         std::to_string(m_data.GetIndexCount()) + " indices");
    }

    void MeshBuffer::UploadToGPU(MeshData&& data)
    {
        // ✅ 性能优化：使用移动语义避免数据拷贝
        m_data = std::move(data);

        // 如果已有 GPU 资源，先释放
        if (m_vao != 0)
        {
            ReleaseGPU();
        }

        // 创建新的 GPU 资源
        CreateVAO();
        UploadVertexData();

        if (m_data.HasIndices())
        {
            UploadIndexData();
        }

        SetupVertexAttributes();

        // 解绑
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        Core::Logger::GetInstance().Info("MeshBuffer::UploadToGPU() - Uploaded mesh to GPU (moved): " +
                                         std::to_string(m_data.GetVertexCount()) + " vertices, " +
                                         std::to_string(m_data.GetIndexCount()) + " indices");
    }

    void MeshBuffer::ReleaseGPU()
    {
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

        Core::Logger::GetInstance().Debug("MeshBuffer::ReleaseGPU() - Released GPU resources");
    }

    void MeshBuffer::BindBuffersToVAO() const
    {
        // 将 VBO 绑定到当前 VAO
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        // 如果有 EBO，也绑定到当前 VAO
        if (m_ebo != 0)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        }
    }

    // ============================================================
    // 内部方法
    // ============================================================

    void MeshBuffer::CreateVAO()
    {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        if (m_data.HasIndices())
        {
            glGenBuffers(1, &m_ebo);
        }
    }

    void MeshBuffer::UploadVertexData()
    {
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        const auto& vertices = m_data.GetVertices();
        glBufferData(GL_ARRAY_BUFFER,
                     m_data.GetVertexDataSizeBytes(),
                     vertices.data(),
                     GL_STATIC_DRAW);
    }

    void MeshBuffer::UploadIndexData()
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

        const auto& indices = m_data.GetIndices();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     m_data.GetIndexDataSizeBytes(),
                     indices.data(),
                     GL_STATIC_DRAW);
    }

    void MeshBuffer::SetupVertexAttributes()
    {
        const auto& offsets = m_data.GetAttributeOffsets();
        const auto& sizes = m_data.GetAttributeSizes();
        size_t stride = m_data.GetVertexStride();

        // ✅ 修复VAO僵尸属性污染：先禁用所有属性，确保干净状态
        // 这可以防止之前绑定的VAO属性污染当前VAO
        // 例如：ImGui可能启用了location 8，但当前网格只用0-2
        // 如果不禁用location 8，glDrawArrays会尝试读取未绑定的VBO，导致驱动崩溃
        GLint maxAttribs = 0;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
        for (GLint i = 0; i < maxAttribs; ++i)
        {
            glDisableVertexAttribArray(i);
        }

        // 设置顶点属性
        for (size_t i = 0; i < sizes.size(); ++i)
        {
            size_t offset = offsets[i];
            int size = sizes[i];

            glVertexAttribPointer(i, size, GL_FLOAT, GL_FALSE,
                                 stride * sizeof(float),
                                 (void*)(offset * sizeof(float)));
            glEnableVertexAttribArray(i);
        }
    }

} // namespace Renderer
