#include "Renderer/InstancedRenderer.hpp"
#include "Renderer/SimpleMesh.hpp"
#include "Renderer/OBJModel.hpp"
#include "Renderer/Cube.hpp"
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

        // 注意：不删除纹理和网格，因为不拥有它们的所有权
    }

    void InstancedRenderer::SetMesh(std::shared_ptr<SimpleMesh> mesh)
    {
        m_mesh = mesh;
        m_materialColor = mesh->GetMaterialColor();
        if (mesh->HasTexture())
        {
            m_texture = mesh->GetTexture();
        }
    }

    void InstancedRenderer::SetInstances(const std::shared_ptr<InstanceData>& data)
    {
        m_instances = data;
        m_instanceCount = data ? data->GetCount() : 0;
    }

    void InstancedRenderer::Initialize()
    {
        // 网格必须已经被创建
        if (!m_mesh || m_mesh->GetVAO() == 0)
        {
            Core::Logger::GetInstance().Error("InstancedRenderer::Initialize() - Mesh not created! Call mesh.Create() first.");
            return;
        }

        if (!m_instances || m_instances->IsEmpty())
        {
            Core::Logger::GetInstance().Error("InstancedRenderer::Initialize() - No instances set!");
            return;
        }

        Core::Logger::GetInstance().Info("Initializing InstancedRenderer for " +
                                         std::to_string(m_instanceCount) + " instances...");

        // 创建实例化 VBO
        glGenBuffers(1, &m_instanceVBO);

        // 上传实例数据
        UploadInstanceData();

        // 绑定网格的 VAO 来设置实例化属性
        GLuint meshVAO = m_mesh->GetVAO();
        glBindVertexArray(meshVAO);

        // 设置实例化属性
        SetupInstanceAttributes();

        glBindVertexArray(0);

        Core::Logger::GetInstance().Info("InstancedRenderer initialized - Instance VBO: " +
                                         std::to_string(m_instanceVBO));
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

        size_t matrixDataSize = matrices.size() * sizeof(glm::mat4);
        size_t colorDataSize = colors.size() * sizeof(glm::vec3);
        size_t totalSize = matrixDataSize + colorDataSize;

        glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, totalSize, nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, matrixDataSize, matrices.data());
        glBufferSubData(GL_ARRAY_BUFFER, matrixDataSize, colorDataSize, colors.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        Core::Logger::GetInstance().Info("Uploaded instance data: " +
                                         std::to_string(matrices.size()) + " matrices, " +
                                         std::to_string(colors.size()) + " colors");
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

    void InstancedRenderer::Render() const
    {
        if (!m_mesh || m_mesh->GetVAO() == 0)
        {
            Core::Logger::GetInstance().Warning("InstancedRenderer::Render() - Mesh not created!");
            return;
        }

        if (!m_instances || m_instances->IsEmpty())
        {
            Core::Logger::GetInstance().Warning("InstancedRenderer::Render() - No instances to render!");
            return;
        }

        // 绑定纹理（如果有）
        if (m_texture != nullptr)
        {
            m_texture->Bind(GL_TEXTURE0);
        }

        // 绑定网格的 VAO
        GLuint meshVAO = m_mesh->GetVAO();
        glBindVertexArray(meshVAO);

        // 执行实例化渲染
        if (m_mesh->HasIndices())
        {
            glDrawElementsInstanced(GL_TRIANGLES,
                                    static_cast<GLsizei>(m_mesh->GetIndexCount()),
                                    GL_UNSIGNED_INT,
                                    0,
                                    static_cast<GLsizei>(m_instanceCount));
        }
        else
        {
            glDrawArraysInstanced(GL_TRIANGLES,
                                  0,
                                  static_cast<GLsizei>(m_mesh->GetVertexCount()),
                                  static_cast<GLsizei>(m_instanceCount));
        }

        glBindVertexArray(0);

        // 解绑纹理
        if (m_texture != nullptr)
        {
            Texture::UnbindStatic();
        }

        // 记录绘制调用
#if ENABLE_RENDER_STATS
        size_t triangleCount = ((m_mesh->HasIndices() ? m_mesh->GetIndexCount() : m_mesh->GetVertexCount()) / 3) * m_instanceCount;
        Core::Logger::GetInstance().LogDrawCall(triangleCount);
#endif
    }

    // 静态方法：为 Cube 创建实例化渲染器
    InstancedRenderer InstancedRenderer::CreateForCube(const std::shared_ptr<InstanceData>& instances)
    {
        InstancedRenderer renderer;
        renderer.SetInstances(instances);

        Core::Logger::GetInstance().Info("Created InstancedRenderer for Cube template");

        return renderer;
    }

    // 静态方法：为 OBJ 模型创建实例化渲染器（返回多个渲染器，每个材质一个）
    std::tuple<std::vector<InstancedRenderer>, std::vector<std::shared_ptr<SimpleMesh>>, std::shared_ptr<InstanceData>>
    InstancedRenderer::CreateForOBJ(const std::string& objPath, const std::shared_ptr<InstanceData>& instances)
    {
        std::vector<InstancedRenderer> renderers;
        std::vector<std::shared_ptr<SimpleMesh>> meshPointers;

        // 使用 OBJModel 的静态方法获取按材质分离的顶点数据
        std::vector<OBJModel::MaterialVertexData> materialDataList =
            OBJModel::GetMaterialVertexData(objPath);

        if (materialDataList.empty())
        {
            Core::Logger::GetInstance().Error("Failed to get material vertex data from: " + objPath);
            return std::make_tuple(std::move(renderers), std::move(meshPointers), nullptr);
        }

        Core::Logger::GetInstance().Info("Creating InstancedRenderers from OBJ: " + objPath +
                                         " with " + std::to_string(materialDataList.size()) + " materials");

        meshPointers.reserve(materialDataList.size());

        // 为每个材质创建一个 SimpleMesh 和 InstancedRenderer
        for (const auto& materialData : materialDataList)
        {
            // 创建 SimpleMesh 作为网格模板（直接构造，避免拷贝）
            auto mesh = std::make_shared<SimpleMesh>(SimpleMesh::CreateFromMaterialData(materialData));
            mesh->Create();  // 创建 OpenGL 对象

            // 保存 shared_ptr
            meshPointers.push_back(mesh);

            // 创建 InstancedRenderer
            InstancedRenderer renderer;
            renderer.SetMesh(mesh);  // 传递 shared_ptr
            renderer.SetInstances(instances);  // 共享同一个 InstanceData shared_ptr（零拷贝）
            renderer.Initialize();

            renderers.push_back(std::move(renderer));

            Core::Logger::GetInstance().Info("Created InstancedRenderer for material " +
                                             materialData.material.name +
                                             " with " + std::to_string(materialData.indices.size()) + " indices");
        }

        return std::make_tuple(std::move(renderers), std::move(meshPointers), instances);
    }

} // namespace Renderer
