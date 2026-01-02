#pragma once
#include "Renderer/Data/MeshBuffer.hpp"
#include "Renderer/Resources/Texture.hpp"
#include "Renderer/Data/InstanceData.hpp"
#include "Renderer/Core/IRenderer.hpp"
#include "Core/GLM.hpp"
#include <vector>
#include <memory>
#include <utility>
#include <tuple>
#include <glad/glad.h>

namespace Renderer
{

    /**
     * @class InstancedRenderer
     * @brief 实例化渲染器 - 负责批量渲染多个相同几何体
     *
     * 设计方案：职责完全分离
     * - ✅ MeshBuffer: 负责 GPU 资源（VAO/VBO/EBO）
     * - ✅ InstanceData: 负责实例数据（矩阵、颜色）
     * - ✅ InstancedRenderer: 负责渲染逻辑
     *
     * 职责：
     * 1. 持有 MeshBuffer 引用（网格模板）
     * 2. 持有 InstanceData（实例数据）
     * 3. 管理 OpenGL 实例化缓冲区（instanceVBO）
     * 4. 执行实例化渲染
     *
     * 使用方式：
     * @code
     * // 1. 创建网格缓冲区（已上传到 GPU）
     * MeshBuffer cubeBuffer = MeshBufferFactory::CreateCubeBuffer();
     *
     * // 2. 准备实例数据
     * auto instances = std::make_shared<InstanceData>();
     * for (int i = 0; i < 100; ++i) {
     *     instances->Add(pos, rot, scale, color);
     * }
     *
     * // 3. 创建渲染器并渲染
     * InstancedRenderer renderer;
     * renderer.SetMesh(std::make_shared<MeshBuffer>(std::move(cubeBuffer)));
     * renderer.SetInstances(instances);
     * renderer.Initialize();
     * renderer.Render();  // 一次调用渲染100个实例
     * @endcode
     *
     * 优点：
     * - ✅ 职责清晰分离，符合单一职责原则
     * - ✅ 可以复用同一个网格渲染不同的实例集合
     * - ✅ 网格和实例数据可以独立管理
     * - ✅ 支持动态更新实例数据
     */
    class InstancedRenderer : public IRenderer
    {
    public:
        InstancedRenderer();
        ~InstancedRenderer();

        // IRenderer 接口实现
        void Initialize() override;
        void Render() const override;
        std::string GetName() const override { return "InstancedRenderer"; }

        // 设置网格模板（使用 shared_ptr 管理所有权）
        void SetMesh(std::shared_ptr<MeshBuffer> meshBuffer);

        // 设置实例数据（使用 shared_ptr 避免拷贝）
        void SetInstances(const std::shared_ptr<InstanceData>& data);

        // 设置材质颜色
        void SetMaterialColor(const glm::vec3& color) { m_materialColor = color; }
        const glm::vec3& GetMaterialColor() const { return m_materialColor; }

        // 设置纹理（使用 shared_ptr 管理所有权）
        void SetTexture(std::shared_ptr<Texture> texture) { m_texture = texture; }
        bool HasTexture() const { return m_texture != nullptr; }
        const std::shared_ptr<Texture>& GetTexture() const { return m_texture; }

        // 获取信息
        size_t GetInstanceCount() const { return m_instanceCount; }
        const std::shared_ptr<MeshBuffer>& GetMesh() const { return m_meshBuffer; }
        const std::shared_ptr<InstanceData>& GetInstances() const { return m_instances; }

        // 更新实例数据到GPU（用于动画）
        void UpdateInstanceData();

        // 静态辅助方法：为 Cube 创建实例化渲染器
        static InstancedRenderer CreateForCube(const std::shared_ptr<InstanceData>& instances);

        // 静态辅助方法：为 OBJ 模型创建实例化渲染器（返回多个渲染器，每个材质一个）
        // 同时返回 meshBuffer 和 instanceData 的 shared_ptr 以保持生命周期
        static std::tuple<std::vector<InstancedRenderer>,
                          std::vector<std::shared_ptr<MeshBuffer>>,
                          std::shared_ptr<InstanceData>>
        CreateForOBJ(const std::string& objPath, const std::shared_ptr<InstanceData>& instances);

        // ✅ 性能优化（2026-01-02）：批量渲染方法
        // 按纹理分组渲染多个渲染器，减少OpenGL状态切换
        // 修复前：每个渲染器独立绑定/解绑纹理和VAO（168次状态切换/帧）
        // 修复后：相同纹理的渲染器批量渲染（状态切换减少60-70%）
        static void RenderBatch(const std::vector<InstancedRenderer*>& renderers);
        static void RenderBatch(const std::vector<std::unique_ptr<InstancedRenderer>>& renderers);

        // 禁用拷贝（防止OpenGL资源双重释放）
        InstancedRenderer(const InstancedRenderer&) = delete;
        InstancedRenderer& operator=(const InstancedRenderer&) = delete;

        // 移动构造（转移OpenGL资源所有权）
        InstancedRenderer(InstancedRenderer&& other) noexcept;

        // 移动赋值（释放当前资源 + 转移所有权）
        InstancedRenderer& operator=(InstancedRenderer&& other) noexcept;

    private:
        // 网格和实例数据
        std::shared_ptr<MeshBuffer> m_meshBuffer;     // 网格缓冲区（使用 shared_ptr 管理所有权）
        std::shared_ptr<InstanceData> m_instances;    // 实例数据（使用 shared_ptr 避免拷贝）
        size_t m_instanceCount = 0;                   // 实例数量

        // OpenGL 对象
        // ✅ 修复：删除独立的VAO，直接使用MeshBuffer的VAO
        // 避免双重所有权导致的资源重复释放/泄漏问题
        GLuint m_instanceVBO = 0;                     // 实例化 VBO（存储矩阵和颜色）

        // 材质和纹理
        std::shared_ptr<Texture> m_texture;           // 纹理（使用 shared_ptr 管理所有权）
        glm::vec3 m_materialColor = glm::vec3(1.0f);

        // 内部方法
        void UploadInstanceData();
        std::vector<float> PrepareInstanceBuffer() const;  // 辅助方法：准备缓冲区数据
    };

} // namespace Renderer
