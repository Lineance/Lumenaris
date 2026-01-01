#pragma once
#include "Renderer/Geometry/Mesh.hpp"
#include "Core/GLM.hpp"
#include <vector>

namespace Renderer
{

    /**
     * @class Torus
     * @brief 圆环体几何体类（甜甜圈形状）
     *
     * 特性：
     * - 支持自定义主半径、管半径和分段数
     * - 正确的法线和UV坐标
     * - 参数化曲面生成
     * - 静态方法支持工厂模式
     */
    class Torus : public IMesh
    {
    public:
        /**
         * @brief 构造函数
         * @param majorRadius 主半径（从中心到管中心的距离，默认1.0）
         * @param minorRadius 管半径（管的粗细，默认0.3）
         * @param majorSegments 主分段数（环的分段，默认32）
         * @param minorSegments 次分段数（管的分段，默认24）
         */
        explicit Torus(float majorRadius = 1.0f, float minorRadius = 0.3f,
                      int majorSegments = 32, int minorSegments = 24);

        // IMesh 接口实现
        void Create() override;
        void Draw() const override;

        // 变换配置
        void SetPosition(const glm::vec3& pos) { m_position = pos; }
        void SetScale(float scale) { m_scale = scale; }
        void SetRotation(const glm::vec3& rotation) { m_rotation = rotation; }
        void SetColor(const glm::vec3& color) { m_color = color; }

        // 参数配置
        void SetMajorRadius(float radius) { m_majorRadius = radius; }
        void SetMinorRadius(float radius) { m_minorRadius = radius; }
        void SetMajorSegments(int segments) { m_majorSegments = segments; }
        void SetMinorSegments(int segments) { m_minorSegments = segments; }

        // 获取状态
        const glm::vec3& GetColor() const { return m_color; }
        float GetMajorRadius() const { return m_majorRadius; }
        float GetMinorRadius() const { return m_minorRadius; }
        glm::mat4 GetModelMatrix() const;

        // 静态方法：获取圆环体的标准顶点数据（用于工厂模式）
        // ⭐ 支持参数化，避免硬编码
        static std::vector<float> GetVertexData(
            float majorRadius = 1.0f,
            float minorRadius = 0.3f,
            int majorSegments = 32,
            int minorSegments = 24
        );

        static std::vector<unsigned int> GetIndexData(
            int majorSegments = 32,
            int minorSegments = 24
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

        // 圆环体参数
        float m_majorRadius;
        float m_minorRadius;
        int m_majorSegments;
        int m_minorSegments;

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
