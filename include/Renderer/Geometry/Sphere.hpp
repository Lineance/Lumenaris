#pragma once
#include "Renderer/Geometry/Mesh.hpp"
#include "Core/GLM.hpp"

namespace Renderer
{

    class Sphere : public IMesh
    {
    public:
        // 构造函数，设置球体参数
        explicit Sphere(float radius = 1.0f, int stacks = 20, int slices = 20);

        // 保持原始接口不变
        void Create(); // 保持public，手动调用
        void Draw() const override;

        // 新增：实例配置接口
        void SetPosition(const glm::vec3 &pos) { m_position = pos; }
        void SetColor(const glm::vec3 &color) { m_color = color; }
        void SetScale(float scale) { m_scale = scale; }
        void SetRadius(float radius) { m_radius = radius; }
        void SetSegments(int stacks, int slices) { m_stacks = stacks; m_slices = slices; }

        // 获取状态
        const glm::vec3 &GetColor() const { return m_color; }
        float GetRadius() const { return m_radius; }
        glm::mat4 GetModelMatrix() const;

        // 静态方法：获取球体的顶点数据（用于实例化渲染）
        static std::vector<float> GetVertexData();
        static std::vector<unsigned int> GetIndexData();
        static void GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes);

        // IMesh接口实现
        unsigned int GetVAO() const override { return m_vao; }
        size_t GetVertexCount() const override { return m_vertexCount; }
        size_t GetIndexCount() const override { return m_indexCount; }
        bool HasIndices() const override { return m_ebo != 0; }
        bool HasTexture() const override { return false; }

    private:
        unsigned int m_vao = 0, m_vbo = 0, m_ebo = 0;

        // 球体参数
        float m_radius;
        int m_stacks;  // 纬度线数量
        int m_slices;  // 经度线数量

        // 新增：实例数据
        glm::vec3 m_position = glm::vec3(0.0f);
        glm::vec3 m_color = glm::vec3(1.0f);
        float m_scale = 1.0f;

        // 缓存顶点和索引数据
        int m_vertexCount = 0;
        int m_indexCount = 0;
    };

} // namespace Renderer
