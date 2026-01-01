#pragma once
#include "Renderer/MeshData.hpp"
#include "Renderer/Texture.hpp"
#include "Core/GLM.hpp"
#include <memory>
#include <glad/glad.h>

namespace Renderer
{

    /**
     * @class MeshBuffer
     * @brief GPU 资源包装器 - 管理网格的 OpenGL 缓冲区
     *
     * 设计原则：
     * - ✅ 持有 CPU 数据副本（支持深拷贝）
     * - ✅ 管理 GPU 资源（VAO/VBO/EBO）
     * - ✅ 不继承任何接口（不是可渲染对象）
     * - ✅ 不提供 Draw/Render 方法（由 Renderer 负责）
     * - ✅ 只提供 GetVAO() 访问接口
     *
     * 与 InstanceData 的对应关系：
     * - InstanceData: 纯 CPU 数据，GPU 资源由 InstancedRenderer 管理
     * - MeshBuffer: 同时管理 CPU 数据和 GPU 资源（因为 VAO 必须提前创建）
     *
     * 使用场景：
     * - 作为 InstancedRenderer 的网格模板
     * - 提供已上传到 GPU 的 VAO
     */
    class MeshBuffer
    {
    public:
        MeshBuffer() = default;
        ~MeshBuffer();

        // ============================================================
        // 拷贝语义（深拷贝）
        // ============================================================

        /**
         * @brief 拷贝构造函数（深拷贝）
         * @note 拷贝 CPU 数据，但不拷贝 GPU 资源
         *       新对象需要调用 UploadToGPU() 创建新的 GPU 资源
         */
        MeshBuffer(const MeshBuffer& other);

        /**
         * @brief 拷贝赋值运算符（深拷贝）
         */
        MeshBuffer& operator=(const MeshBuffer& other);

        // ============================================================
        // 移动语义（高效转移资源）
        // ============================================================

        /**
         * @brief 移动构造函数
         * @note 转移 GPU 资源所有权，源对象变为无效状态
         */
        MeshBuffer(MeshBuffer&& other) noexcept;

        /**
         * @brief 移动赋值运算符
         */
        MeshBuffer& operator=(MeshBuffer&& other) noexcept;

        // ============================================================
        // GPU 操作
        // ============================================================

        /**
         * @brief 上传数据到 GPU
         * @param data 网格数据
         * @note 创建或更新 VAO/VBO/EBO
         */
        void UploadToGPU(const MeshData& data);

        /**
         * @brief 释放 GPU 资源
         */
        void ReleaseGPU();

        // ============================================================
        // 访问接口
        // ============================================================

        /**
         * @brief 获取 VAO（InstancedRenderer 需要这个）
         * @return OpenGL VAO ID
         */
        unsigned int GetVAO() const { return m_vao; }

        /**
         * @brief 获取顶点数量
         */
        size_t GetVertexCount() const { return m_data.GetVertexCount(); }

        /**
         * @brief 获取索引数量
         */
        size_t GetIndexCount() const { return m_data.GetIndexCount(); }

        /**
         * @brief 是否有索引
         */
        bool HasIndices() const { return m_data.HasIndices(); }

        /**
         * @brief 获取材质颜色
         */
        const glm::vec3& GetMaterialColor() const { return m_data.GetMaterialColor(); }

        // ============================================================
        // 纹理管理
        // ============================================================

        /**
         * @brief 设置纹理（使用 shared_ptr 管理所有权）
         */
        void SetTexture(std::shared_ptr<Texture> texture) { m_texture = texture; }

        /**
         * @brief 获取纹理
         */
        std::shared_ptr<Texture> GetTexture() const { return m_texture; }

        /**
         * @brief 是否有纹理
         */
        bool HasTexture() const { return m_texture != nullptr; }

        // ============================================================
        // 数据访问
        // ============================================================

        /**
         * @brief 获取底层数据（只读）
         */
        const MeshData& GetData() const { return m_data; }

    private:
        // CPU 数据副本（支持深拷贝）
        MeshData m_data;

        // GPU 资源
        unsigned int m_vao = 0;
        unsigned int m_vbo = 0;
        unsigned int m_ebo = 0;

        // 纹理（使用 shared_ptr 管理所有权）
        std::shared_ptr<Texture> m_texture;

        // 内部方法
        void CreateVAO();
        void UploadVertexData();
        void UploadIndexData();
        void SetupVertexAttributes();
    };

} // namespace Renderer
