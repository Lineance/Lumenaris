#pragma once
#include "Renderer/Mesh.hpp"
#include "Core/GLM.hpp"

namespace Renderer
{

    class Cube : public IMesh
    {
    public:
        // 保持原始接口不变
        void Create(); // 保持public，手动调用
        void Draw() const override;

        // 新增：实例配置接口
        void SetPosition(const glm::vec3 &pos) { m_position = pos; }
        void SetColor(const glm::vec3 &color) { m_color = color; }
        void SetScale(float scale) { m_scale = scale; }
        void SetRotation(const glm::vec3 &rotation) { m_rotation = rotation; }

        // 获取状态
        const glm::vec3 &GetColor() const { return m_color; }
        glm::mat4 GetModelMatrix() const;

    private:
        unsigned int m_vao = 0, m_vbo = 0;

        // 新增：实例数据
        glm::vec3 m_position = glm::vec3(0.0f);
        glm::vec3 m_color = glm::vec3(1.0f);
        float m_scale = 1.0f;
        glm::vec3 m_rotation = glm::vec3(0.0f); // 欧拉角（度数）
    };

} // namespace Renderer