#pragma once
#include "Renderer/Geometry/Mesh.hpp"
#include "Core/GLM.hpp"
#include <vector>

namespace Renderer
{

    /**
     * @class Plane
     * @brief 平面几何体类
     *
     * 特性：
     * - 简单的四边形平面
     * - 支持自定义宽度和高度
     * - 可配置分段数（用于细分）
     * - 正确的法线和UV坐标
     * - 静态方法支持工厂模式
     */
    class Plane : public IMesh
    {
    public:
        /**
         * @brief 构造函数
         * @param width 平面宽度（默认1.0）
         * @param height 平面高度（默认1.0）
         * @param widthSegments 宽度方向的分段数（默认1）
         * @param heightSegments 高度方向的分段数（默认1）
         */
        explicit Plane(float width = 1.0f, float height = 1.0f,
                      int widthSegments = 1, int heightSegments = 1);

        // IMesh 接口实现
        void Create() override;
        void Draw() const override;

        // 变换配置
        void SetPosition(const glm::vec3& pos) { m_position = pos; }
        void SetScale(float scale) { m_scale = scale; }
        void SetRotation(const glm::vec3& rotation) { m_rotation = rotation; }
        void SetColor(const glm::vec3& color) { m_color = color; }

        // 参数配置
        void SetWidth(float width) { m_width = width; }
        void SetHeight(float height) { m_height = height; }
        void SetWidthSegments(int segments) { m_widthSegments = segments; }
        void SetHeightSegments(int segments) { m_heightSegments = segments; }

        // 获取状态
        const glm::vec3& GetColor() const { return m_color; }
        float GetWidth() const { return m_width; }
        float GetHeight() const { return m_height; }
        glm::mat4 GetModelMatrix() const;

        // 静态方法：获取平面的标准顶点数据（用于工厂模式）
        // ⭐ 支持参数化，避免硬编码
        static std::vector<float> GetVertexData(
            float width = 1.0f,
            float height = 1.0f,
            int widthSegments = 1,
            int heightSegments = 1
        );

        static std::vector<unsigned int> GetIndexData(
            int widthSegments = 1,
            int heightSegments = 1
        );

        static void GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes);

        // IMesh 接口扩展
        unsigned int GetVAO() const override { return m_vao; }
        size_t GetVertexCount() const override { return m_vertexCount; }
        size_t GetIndexCount() const override { return m_indexCount; }
        bool HasIndices() const override { return m_ebo != 0; }
        bool HasTexture() const override { return false; }

    private:
        unsigned int m_vao = 0, m_vbo = 0, m_ebo = 0;

        // 平面参数
        float m_width;
        float m_height;
        int m_widthSegments;
        int m_heightSegments;

        // 变换数据
        glm::vec3 m_position = glm::vec3(0.0f);
        glm::vec3 m_color = glm::vec3(1.0f);
        float m_scale = 1.0f;
        glm::vec3 m_rotation = glm::vec3(0.0f);

        // 缓存数据
        int m_vertexCount = 0;
        int m_indexCount = 0;
    };

} // namespace Renderer
