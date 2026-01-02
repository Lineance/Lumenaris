#include "Renderer/Renderer/InstancedRenderer.hpp"
#include "Renderer/Data/MeshBuffer.hpp"
#include "Renderer/Factory/MeshDataFactory.hpp"
#include "Renderer/Geometry/OBJModel.hpp"
#include "Core/Logger.hpp"
#include <glad/glad.h>
#include <cstring>  // for std::memcpy

namespace Renderer
{

    InstancedRenderer::InstancedRenderer() = default;

    // 移动构造函数（转移所有权）
    InstancedRenderer::InstancedRenderer(InstancedRenderer &&other) noexcept
        : m_meshBuffer(std::move(other.m_meshBuffer)),
          m_instances(std::move(other.m_instances)),
          m_instanceCount(other.m_instanceCount),
          m_instanceVBO(other.m_instanceVBO),
          m_texture(std::move(other.m_texture)),
          m_materialColor(other.m_materialColor)
    {
        // 将源对象的OpenGL资源ID置零，避免析构时重复释放
        other.m_instanceVBO = 0;
        other.m_instanceCount = 0;
        other.m_materialColor = glm::vec3(1.0f);
    }

    // 移动赋值运算符
    InstancedRenderer &InstancedRenderer::operator=(InstancedRenderer &&other) noexcept
    {
        if (this != &other)
        {
            // 1. 释放当前对象的OpenGL资源（instanceVBO）
            if (m_instanceVBO)
            {
                glDeleteBuffers(1, &m_instanceVBO);
                m_instanceVBO = 0;
            }

            // 2. 转移所有资源
            m_meshBuffer = std::move(other.m_meshBuffer);
            m_instances = std::move(other.m_instances);
            m_instanceCount = other.m_instanceCount;
            m_instanceVBO = other.m_instanceVBO;
            m_texture = std::move(other.m_texture);
            m_materialColor = other.m_materialColor;

            // 3. 将源对象置为有效但空的状态
            other.m_instanceVBO = 0;
            other.m_instanceCount = 0;
            other.m_materialColor = glm::vec3(1.0f);
        }
        return *this;
    }

    InstancedRenderer::~InstancedRenderer()
    {
        // ✅ 修复：只清理实例化VBO，VAO由MeshBuffer管理
        if (m_instanceVBO)
        {
            glDeleteBuffers(1, &m_instanceVBO);
            m_instanceVBO = 0;
        }

        // 注意：网格缓冲区（含VAO）和纹理由 shared_ptr 自动管理，无需手动删除
    }

    void InstancedRenderer::SetMesh(std::shared_ptr<MeshBuffer> meshBuffer)
    {
        m_meshBuffer = meshBuffer;
        m_materialColor = meshBuffer->GetMaterialColor();
        if (meshBuffer->HasTexture())
        {
            m_texture = meshBuffer->GetTexture(); // 共享纹理的 shared_ptr
        }
    }

    void InstancedRenderer::SetInstances(const std::shared_ptr<InstanceData> &data)
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

        // ✅ 修复：检查是否已经初始化，避免VBO泄漏
        if (m_instanceVBO != 0)
        {
            Core::Logger::GetInstance().Warning("InstancedRenderer::Initialize() - Already initialized, cleaning up old instance VBO.");
            glDeleteBuffers(1, &m_instanceVBO);
            m_instanceVBO = 0;
        }

        // ✅ 修复：不再创建独立VAO，直接使用MeshBuffer的VAO
        // 创建实例化 VBO（用于存储实例矩阵和颜色）
        glGenBuffers(1, &m_instanceVBO);

        // 上传实例数据
        UploadInstanceData();

        // ✅ 修复：直接在MeshBuffer的VAO上配置实例属性
        GLuint meshVAO = m_meshBuffer->GetVAO();
        glBindVertexArray(meshVAO);

        // 设置实例属性（从instanceVBO）
        glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);

        // 设置实例矩阵属性 (location 3, 4, 5, 6)
        for (size_t i = 0; i < 4; ++i)
        {
            glEnableVertexAttribArray(3 + i);
            glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(i * sizeof(glm::vec4)));
            glVertexAttribDivisor(3 + i, 1); // 每个实例更新一次
        }

        // 设置实例颜色属性 (location 7)
        size_t matrixDataSize = m_instances->GetModelMatrices().size() * sizeof(glm::mat4);
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)matrixDataSize);
        glVertexAttribDivisor(7, 1); // 每个实例更新一次

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        Core::Logger::GetInstance().Info("InstancedRenderer::Initialize() - Initialized with " +
                                         std::to_string(m_instanceCount) + " instances" +
                                         ", MeshBuffer VAO: " + std::to_string(meshVAO) +
                                         ", instanceVBO: " + std::to_string(m_instanceVBO) +
                                         ", instancesPtr: " + std::to_string(reinterpret_cast<uintptr_t>(m_instances.get())));
    }

    void InstancedRenderer::UploadInstanceData()
    {
        if (!m_instanceVBO)
        {
            Core::Logger::GetInstance().Error("InstancedRenderer::UploadInstanceData() - Instance VBO not created!");
            return;
        }

        // 准备缓冲区数据
        std::vector<float> buffer = PrepareInstanceBuffer();

        // 单次传输到 GPU
        glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
        glBufferData(GL_ARRAY_BUFFER,
                     buffer.size() * sizeof(float),
                     buffer.data(),
                     GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    std::vector<float> InstancedRenderer::PrepareInstanceBuffer() const
    {
        const auto &matrices = m_instances->GetModelMatrices();
        const auto &colors = m_instances->GetColors();

        // 计算总数据大小：每个矩阵 16 个 float，每个颜色 3 个 float
        size_t matrixFloatCount = matrices.size() * 16;
        size_t colorFloatCount = colors.size() * 3;
        size_t totalFloatCount = matrixFloatCount + colorFloatCount;

        // ✅ 性能优化（2026-01-02）：单次分配，避免多次 resize() 和零填充开销
        std::vector<float> buffer;
        buffer.resize(totalFloatCount);  // 只分配一次

        // ✅ 性能优化：批量复制矩阵（编译器自动优化为 AVX/AVX2/AVX-512）
        std::memcpy(buffer.data(), matrices.data(), matrices.size() * sizeof(glm::mat4));

        // ✅ 性能优化：批量复制颜色（glm::vec3 内存布局连续，无 padding）
        // sizeof(glm::vec3) = 12 bytes = 3 floats，memcpy 安全且可向量化
        std::memcpy(buffer.data() + matrixFloatCount,
                    colors.data(),
                    colors.size() * sizeof(glm::vec3));

        return buffer;
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

        // ✅ 性能优化（2026-01-02）：脏标记机制
        // 只在数据实际变化时更新 GPU，避免每帧冗余传输
        if (!m_instances->IsDirty())
        {
            return;  // 数据未变化，跳过 GPU 更新
        }

        // 准备缓冲区数据
        std::vector<float> buffer = PrepareInstanceBuffer();

        // 更新 GPU 缓冲区数据（使用 glBufferSubData）
        glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER,
                        0,
                        buffer.size() * sizeof(float),
                        buffer.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // ❌ BUG 修复（2026-01-02）：不要在这里清除脏标记！
        // 当多个 renderer 共享同一个 instanceData 时（例如多材质 OBJ 模型），
        // 每个 renderer 都有自己的 instanceVBO，都需要从同一个 instanceData 更新数据。
        // 如果在第一个 renderer 更新后就清除脏标记，其余的 renderer 将无法更新。
        // 脏标记由调用者在所有 renderer 更新完毕后统一清除。
    }

    void InstancedRenderer::Render() const
    {
        // ✅ 修复：检查MeshBuffer的VAO（而不是已删除的m_vao）
        if (!m_meshBuffer || m_meshBuffer->GetVAO() == 0)
        {
            return; // 静默失败，避免每帧日志
        }

        if (!m_instances || m_instances->IsEmpty())
        {
            return; // 静默失败，避免每帧日志
        }

        // 绑定纹理（如果有）
        // ⭐ 使用纹理单元1（TextureUnit::MATERIAL_DIFFUSE），为ImGui预留单元0
        if (m_texture)
        {
            m_texture->Bind(GL_TEXTURE1);
        }

        // ✅ 修复：绑定MeshBuffer的VAO（而不是独立的m_vao）
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
    InstancedRenderer InstancedRenderer::CreateForCube(const std::shared_ptr<InstanceData> &instances)
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

    // ✅ 性能优化（2026-01-02）：批量渲染方法实现
    void InstancedRenderer::RenderBatch(const std::vector<InstancedRenderer*>& renderers)
    {
        if (renderers.empty())
        {
            return;
        }

        // ✅ 按纹理分组（使用原始指针作为key，避免shared_ptr拷贝）
        // 使用 std::map 保持纹理顺序稳定
        std::map<Texture*, std::vector<InstancedRenderer*>> batches;

        for (auto* renderer : renderers)
        {
            if (!renderer)
            {
                continue;  // 跳过空指针
            }

            // 获取纹理原始指针（nullptr表示无纹理）
            Texture* textureKey = renderer->GetTexture().get();
            batches[textureKey].push_back(renderer);
        }

        // ✅ 批量渲染每个纹理组
        for (const auto& [texturePtr, batch] : batches)
        {
            if (batch.empty())
            {
                continue;
            }

            // 绑定纹理（如果有）
            if (texturePtr)
            {
                texturePtr->Bind(GL_TEXTURE1);
            }

            // 批量渲染所有使用该纹理的渲染器
            for (auto* renderer : batch)
            {
                renderer->Render();
            }

            // 解绑纹理
            if (texturePtr)
            {
                Texture::UnbindStatic();
            }
        }
    }

    // ✅ 重载版本：支持 unique_ptr vector
    void InstancedRenderer::RenderBatch(const std::vector<std::unique_ptr<InstancedRenderer>>& renderers)
    {
        // 转换为原始指针 vector，调用重载版本
        std::vector<InstancedRenderer*> rawPointers;
        rawPointers.reserve(renderers.size());

        for (const auto& renderer : renderers)
        {
            rawPointers.push_back(renderer.get());
        }

        RenderBatch(rawPointers);
    }

    // ✅ 重载版本：支持值类型 vector (2026-01-02)
    void InstancedRenderer::RenderBatch(const std::vector<InstancedRenderer>& renderers)
    {
        // 转换为原始指针 vector，调用重载版本
        std::vector<InstancedRenderer*> rawPointers;
        rawPointers.reserve(renderers.size());

        for (size_t i = 0; i < renderers.size(); ++i)
        {
            // 注意：获取指向vector中元素的指针（const_cast因为Render()是const方法）
            rawPointers.push_back(const_cast<InstancedRenderer*>(&renderers[i]));
        }

        RenderBatch(rawPointers);
    }

    // 静态方法：为 OBJ 模型创建实例化渲染器（返回多个渲染器，每个材质一个）
    std::tuple<std::vector<InstancedRenderer>, std::vector<std::shared_ptr<MeshBuffer>>, std::shared_ptr<InstanceData>>
    InstancedRenderer::CreateForOBJ(const std::string &objPath, const std::shared_ptr<InstanceData> &instances)
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
            const std::string &texturePath = meshBufferPtr->GetData().GetTexturePath();

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
