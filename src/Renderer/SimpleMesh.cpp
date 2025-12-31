#include "Renderer/SimpleMesh.hpp"
#include "Renderer/OBJModel.hpp"
#include "Renderer/Cube.hpp"
#include "Core/Logger.hpp"
#include <glad/glad.h>

namespace Renderer
{

    SimpleMesh::~SimpleMesh()
    {
        // 清理 OpenGL 资源
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

        // 注意：不删除纹理，因为 SimpleMesh 不拥有纹理的所有权
    }

    // 拷贝构造函数（深拷贝）
    SimpleMesh::SimpleMesh(const SimpleMesh& other)
        : m_vertices(other.m_vertices),
          m_indices(other.m_indices),
          m_vertexStride(other.m_vertexStride),
          m_vertexCount(other.m_vertexCount),
          m_indexCount(other.m_indexCount),
          m_hasIndices(other.m_hasIndices),
          m_texture(other.m_texture),
          m_materialColor(other.m_materialColor),
          m_vertexAttributes(other.m_vertexAttributes),
          m_vao(0),  // 新对象需要重新创建 OpenGL 资源
          m_vbo(0),
          m_ebo(0)
    {
        // 深拷贝：只在源对象已创建时才创建新的 OpenGL 资源
        if (other.m_vao != 0)
        {
            Create();
        }
    }

    // 拷贝赋值运算符（深拷贝）
    SimpleMesh& SimpleMesh::operator=(const SimpleMesh& other)
    {
        if (this != &other)
        {
            // 清理旧资源
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

            // 拷贝数据
            m_vertices = other.m_vertices;
            m_indices = other.m_indices;
            m_vertexStride = other.m_vertexStride;
            m_vertexCount = other.m_vertexCount;
            m_indexCount = other.m_indexCount;
            m_hasIndices = other.m_hasIndices;
            m_texture = other.m_texture;
            m_materialColor = other.m_materialColor;
            m_vertexAttributes = other.m_vertexAttributes;

            // 深拷贝：只在源对象已创建时才创建新的 OpenGL 资源
            if (other.m_vao != 0)
            {
                Create();
            }
        }
        return *this;
    }

    void SimpleMesh::SetVertexData(const std::vector<float>& vertices, size_t stride)
    {
        m_vertices = vertices;
        m_vertexStride = stride;
        m_vertexCount = vertices.size() / stride;
    }

    void SimpleMesh::SetVertexLayout(const std::vector<size_t>& offsets, const std::vector<int>& sizes)
    {
        m_vertexAttributes.clear();
        for (size_t i = 0; i < offsets.size() && i < sizes.size(); ++i)
        {
            m_vertexAttributes.push_back({offsets[i], sizes[i]});
        }
    }

    void SimpleMesh::SetIndexData(const std::vector<unsigned int>& indices)
    {
        m_indices = indices;
        m_indexCount = indices.size();
        m_hasIndices = true;
    }

    void SimpleMesh::SetTexture(Texture* texture)
    {
        m_texture = texture;
    }

    void SimpleMesh::Create()
    {
        if (m_vertices.empty())
        {
            Core::Logger::GetInstance().Error("SimpleMesh::Create() called with no vertices!");
            return;
        }

        Core::Logger::GetInstance().Info("Creating SimpleMesh with " +
                                         std::to_string(m_vertexCount) + " vertices...");

        // 创建 VAO 和 VBO
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

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

        // 如果有索引数据，创建 EBO
        if (m_hasIndices && !m_indices.empty())
        {
            glGenBuffers(1, &m_ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
            Core::Logger::GetInstance().Info("Created EBO with " + std::to_string(m_indices.size()) + " indices");
        }

        glBindVertexArray(0);

        Core::Logger::GetInstance().Info("SimpleMesh created successfully - VAO: " +
                                         std::to_string(m_vao) + ", VBO: " + std::to_string(m_vbo) +
                                         (m_hasIndices ? ", EBO: " + std::to_string(m_ebo) : ""));
    }

    void SimpleMesh::Draw() const
    {
        if (m_vao == 0)
        {
            Core::Logger::GetInstance().Error("SimpleMesh::Draw() called before Create()!");
            return;
        }

        // 绑定纹理（如果有）
        if (m_texture != nullptr)
        {
            m_texture->Bind(GL_TEXTURE0);
        }

        glBindVertexArray(m_vao);

        if (m_hasIndices)
        {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_INT, 0);
        }
        else
        {
            glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertexCount));
        }

        glBindVertexArray(0);

        // 解绑纹理
        if (m_texture != nullptr)
        {
            Texture::UnbindStatic();
        }

        size_t triangleCount = (m_hasIndices ? m_indexCount : m_vertexCount) / 3;
        Core::Logger::GetInstance().LogDrawCall(triangleCount);
    }

    // 静态方法：从 Cube 创建 SimpleMesh
    SimpleMesh SimpleMesh::CreateFromCube()
    {
        SimpleMesh mesh;

        // 从 Cube 类获取顶点数据
        std::vector<float> cubeVertices = Cube::GetVertexData();
        std::vector<size_t> offsets;
        std::vector<int> sizes;
        Cube::GetVertexLayout(offsets, sizes);

        mesh.SetVertexData(cubeVertices, 8);
        mesh.SetVertexLayout(offsets, sizes);

        Core::Logger::GetInstance().Info("Created SimpleMesh from Cube template");

        return mesh;
    }

    // 静态方法：从 OBJ 材质数据创建 SimpleMesh
    SimpleMesh SimpleMesh::CreateFromMaterialData(const OBJModel::MaterialVertexData& materialData)
    {
        SimpleMesh mesh;

        // 设置顶点数据
        mesh.SetVertexData(materialData.vertices, 8);
        mesh.SetVertexLayout({0, 3, 6}, {3, 3, 2});

        // 设置索引数据
        if (!materialData.indices.empty())
        {
            mesh.SetIndexData(materialData.indices);
        }

        // 设置材质颜色
        mesh.SetMaterialColor(materialData.material.diffuse);

        // 加载并设置纹理
        if (!materialData.texturePath.empty())
        {
            Texture* texture = new Texture();
            if (texture->LoadFromFile(materialData.texturePath))
            {
                mesh.SetTexture(texture);
                Core::Logger::GetInstance().Info("Loaded texture: " + materialData.texturePath);
            }
            else
            {
                delete texture;
                texture = nullptr;
                Core::Logger::GetInstance().Warning("Failed to load texture: " + materialData.texturePath);
            }
        }

        Core::Logger::GetInstance().Info("Created SimpleMesh for material " +
                                         materialData.material.name +
                                         " with " + std::to_string(materialData.indices.size()) + " indices");

        return mesh;
    }

} // namespace Renderer
