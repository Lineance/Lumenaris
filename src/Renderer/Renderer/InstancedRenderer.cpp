#include "Renderer/Renderer/InstancedRenderer.hpp"
#include "Renderer/Data/MeshBuffer.hpp"
#include "Renderer/Factory/MeshDataFactory.hpp"
#include "Renderer/Geometry/OBJModel.hpp"
#include "Core/Logger.hpp"
#include <glad/glad.h>

namespace Renderer
{

    InstancedRenderer::InstancedRenderer() = default;

    InstancedRenderer::~InstancedRenderer()
    {
        // 清理 OpenGL 资源
        if (m_instanceVBO)
        {
            glDeleteBuffers(1, &m_instanceVBO);
            m_instanceVBO = 0;
        }

        // 注意：网格缓冲区和纹理由 shared_ptr 自动管理，无需手动删除
    }

    void InstancedRenderer::SetMesh(std::shared_ptr<MeshBuffer> meshBuffer)
    {
        m_meshBuffer = meshBuffer;
        m_materialColor = meshBuffer->GetMaterialColor();
        if (meshBuffer->HasTexture())
        {
            m_texture = meshBuffer->GetTexture();  // 共享纹理的 shared_ptr
        }
    }

    void InstancedRenderer::SetInstances(const std::shared_ptr<InstanceData>& data)
    {
        m_instances = data;
        m_instanceCount = data ? data->GetCount() : 0;
    }

    void InstancedRenderer::Initialize()
    {
        // 网格缓冲区必须已经上传到 GPU
        if (!m_meshBuffer || m_meshBuffer->GetVAO() == 0)
        {
            Core::Logger::GetInstance().Error("InstancedRenderer::Initialize() - MeshBuffer not uploaded to GPU! Call meshBuffer.UploadToGPU() first.");
            return;
        }

        if (!m_instances || m_instances->IsEmpty())
        {
            Core::Logger::GetInstance().Error("InstancedRenderer::Initialize() - No instances set!");
            return;
        }

        // 创建实例化 VBO
        glGenBuffers(1, &m_instanceVBO);

        // 上传实例数据
        UploadInstanceData();

        // 绑定网格缓冲区的 VAO 来设置实例化属性
        GLuint meshVAO = m_meshBuffer->GetVAO();
        glBindVertexArray(meshVAO);

        // 设置实例化属性
        SetupInstanceAttributes();

        glBindVertexArray(0);

        Core::Logger::GetInstance().Info("InstancedRenderer::Initialize() - Initialized with " +
                                         std::to_string(m_instanceCount) + " instances");
    }

    void InstancedRenderer::UploadInstanceData()
    {
        if (!m_instanceVBO)
        {
            Core::Logger::GetInstance().Error("InstancedRenderer::UploadInstanceData() - Instance VBO not created!");
            return;
        }

        const auto& matrices = m_instances->GetModelMatrices();
        const auto& colors = m_instances->GetColors();

        // 计算总数据大小：每个矩阵 16 个 float，每个颜色 3 个 float
        size_t matrixFloatCount = matrices.size() * 16;
        size_t colorFloatCount = colors.size() * 3;
        size_t totalFloatCount = matrixFloatCount + colorFloatCount;

        // 创建连续的缓冲区（使用 vector 避免手动内存管理）
        std::vector<float> buffer;
        buffer.reserve(totalFloatCount);

        // 将矩阵数据插入缓冲区（每个 mat4 是 16 个 float）
        const float* matrixData = reinterpret_cast<const float*>(matrices.data());
        buffer.insert(buffer.end(), matrixData, matrixData + matrixFloatCount);

        // 将颜色数据插入缓冲区（每个 vec3 是 3 个 float）
        const float* colorData = reinterpret_cast<const float*>(colors.data());
        buffer.insert(buffer.end(), colorData, colorData + colorFloatCount);

        // 单次传输到 GPU
        glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
        glBufferData(GL_ARRAY_BUFFER,
                     buffer.size() * sizeof(float),
                     buffer.data(),
                     GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void InstancedRenderer::SetupInstanceAttributes()
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);

        // 设置实例矩阵属性 (location 3, 4, 5, 6)
        for (size_t i = 0; i < 4; ++i)
        {
            glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
            glEnableVertexAttribArray(3 + i);
            glVertexAttribDivisor(3 + i, 1);  // 每个实例更新一次
        }

        // 设置实例颜色属性 (location 7)
        size_t matrixDataSize = m_instances->GetModelMatrices().size() * sizeof(glm::mat4);
        glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)matrixDataSize);
        glEnableVertexAttribArray(7);
        glVertexAttribDivisor(7, 1);  // 每个实例更新一次

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void InstancedRenderer::UpdateInstanceData()
    {
        if (!m_instanceVBO)
        {
            Core::Logger::GetInstance().Error("InstancedRenderer::UpdateInstanceData() - Instance VBO not created!");
            return;
        }

        if (!m_instances || m_instances->IsEmpty())
        {
            return;
        }

        const auto& matrices = m_instances->GetModelMatrices();
        const auto& colors = m_instances->GetColors();

        // 计算总数据大小：每个矩阵 16 个 float，每个颜色 3 个 float
        size_t matrixFloatCount = matrices.size() * 16;
        size_t colorFloatCount = colors.size() * 3;
        size_t totalFloatCount = matrixFloatCount + colorFloatCount;

        // 创建连续的缓冲区
        std::vector<float> buffer;
        buffer.reserve(totalFloatCount);

        // 将矩阵数据插入缓冲区（每个 mat4 是 16 个 float）
        const float* matrixData = reinterpret_cast<const float*>(matrices.data());
        buffer.insert(buffer.end(), matrixData, matrixData + matrixFloatCount);

        // 将颜色数据插入缓冲区（每个 vec3 是 3 个 float）
        const float* colorData = reinterpret_cast<const float*>(colors.data());
        buffer.insert(buffer.end(), colorData, colorData + colorFloatCount);

        // 更新 GPU 缓冲区数据（使用 glBufferSubData 而不是 glBufferData）
        glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER,
                       0,
                       buffer.size() * sizeof(float),
                       buffer.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void InstancedRenderer::Render() const
    {
        if (!m_meshBuffer || m_meshBuffer->GetVAO() == 0)
        {
            return;  // 静默失败，避免每帧日志
        }

        if (!m_instances || m_instances->IsEmpty())
        {
            return;  // 静默失败，避免每帧日志
        }

        // 绑定纹理（如果有）
        if (m_texture)
        {
            m_texture->Bind(GL_TEXTURE0);
        }

        // 绑定网格缓冲区的 VAO
        GLuint meshVAO = m_meshBuffer->GetVAO();
        glBindVertexArray(meshVAO);

        // 执行实例化渲染
        if (m_meshBuffer->HasIndices())
        {
            glDrawElementsInstanced(GL_TRIANGLES,
                                    static_cast<GLsizei>(m_meshBuffer->GetIndexCount()),
                                    GL_UNSIGNED_INT,
                                    0,
                                    static_cast<GLsizei>(m_instanceCount));
        }
        else
        {
            glDrawArraysInstanced(GL_TRIANGLES,
                                  0,
                                  static_cast<GLsizei>(m_meshBuffer->GetVertexCount()),
                                  static_cast<GLsizei>(m_instanceCount));
        }

        glBindVertexArray(0);

        // 解绑纹理
        if (m_texture)
        {
            Texture::UnbindStatic();
        }

        // 记录绘制调用
#if ENABLE_RENDER_STATS
        size_t triangleCount = ((m_meshBuffer->HasIndices() ? m_meshBuffer->GetIndexCount() : m_meshBuffer->GetVertexCount()) / 3) * m_instanceCount;
        Core::Logger::GetInstance().LogDrawCall(triangleCount);
#endif
    }

    // 静态方法：为 Cube 创建实例化渲染器
    InstancedRenderer InstancedRenderer::CreateForCube(const std::shared_ptr<InstanceData>& instances)
    {
        // 使用工厂创建 MeshBuffer（已上传到 GPU）
        MeshBuffer meshBuffer = MeshBufferFactory::CreateCubeBuffer();

        // 创建 shared_ptr
        auto meshBufferPtr = std::make_shared<MeshBuffer>(std::move(meshBuffer));

        // 创建渲染器
        InstancedRenderer renderer;
        renderer.SetMesh(meshBufferPtr);
        renderer.SetInstances(instances);
        renderer.Initialize();

        Core::Logger::GetInstance().Info("InstancedRenderer::CreateForCube() - Created renderer for " +
                                         std::to_string(instances->GetCount()) + " instances");

        return renderer;
    }

    // 静态方法：为 OBJ 模型创建实例化渲染器（返回多个渲染器，每个材质一个）
    std::tuple<std::vector<InstancedRenderer>, std::vector<std::shared_ptr<MeshBuffer>>, std::shared_ptr<InstanceData>>
    InstancedRenderer::CreateForOBJ(const std::string& objPath, const std::shared_ptr<InstanceData>& instances)
    {
        std::vector<InstancedRenderer> renderers;
        std::vector<std::shared_ptr<MeshBuffer>> meshBuffers;

        // 使用工厂创建 MeshBuffer 列表（已上传到 GPU）
        std::vector<MeshBuffer> buffers = MeshBufferFactory::CreateOBJBuffers(objPath);

        Core::Logger::GetInstance().Info("InstancedRenderer::CreateForOBJ() - Creating " +
                                         std::to_string(buffers.size()) + " renderers from " + objPath);

        renderers.reserve(buffers.size());
        meshBuffers.reserve(buffers.size());

        // 为每个材质创建渲染器
        for (size_t i = 0; i < buffers.size(); ++i)
        {
            auto meshBufferPtr = std::make_shared<MeshBuffer>(std::move(buffers[i]));

            // 从MeshData中获取纹理路径（无需重新加载OBJ！）
            const std::string& texturePath = meshBufferPtr->GetData().GetTexturePath();

            // 如果有纹理，加载纹理
            if (!texturePath.empty())
            {
                auto texture = std::make_shared<Texture>();
                if (texture->LoadFromFile(texturePath))
                {
                    meshBufferPtr->SetTexture(texture);
                }
            }

            meshBuffers.push_back(meshBufferPtr);

            // 创建渲染器并使用移动语义添加到 vector
            InstancedRenderer renderer;
            renderer.SetMesh(meshBufferPtr);
            renderer.SetInstances(instances);
            renderer.Initialize();

            renderers.push_back(std::move(renderer));
        }

        // 使用移动语义返回 tuple，避免拷贝
        return std::make_tuple(std::move(renderers), std::move(meshBuffers), instances);
    }

} // namespace Renderer
