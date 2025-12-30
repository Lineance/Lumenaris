#pragma once
#include "Renderer/Geometry.hpp"
#include "Renderer/OBJLoader.hpp"
#include <glm/glm.hpp>
#include <string>

namespace Renderer
{

    class OBJModel : public IGeometry
    {
    public:
        OBJModel();
        explicit OBJModel(const std::string& filepath);
        ~OBJModel();

        // 从IGeometry继承的接口
        void Create() override;
        void Draw() const override;

        // 加载OBJ文件
        bool LoadFromFile(const std::string& filepath);

        // 配置接口（与Cube保持一致）
        void SetPosition(const glm::vec3& pos) { m_position = pos; }
        void SetColor(const glm::vec3& color) { m_color = color; }
        void SetScale(float scale) { m_scale = scale; }
        void SetRotation(const glm::vec3& rotation) { m_rotation = rotation; }

        // 获取状态
        const glm::vec3& GetColor() const { return m_color; }
        const std::string& GetFilePath() const { return m_filepath; }
        glm::mat4 GetModelMatrix() const;

        // 获取顶点数量
        size_t GetVertexCount() const { return m_loader.GetVertices().size(); }

    private:
        // OpenGL缓冲区对象
        unsigned int m_vao = 0;
        unsigned int m_vbo = 0;
        unsigned int m_ebo = 0;

        // OBJ文件加载器
        OBJLoader m_loader;

        // 模型文件路径
        std::string m_filepath;

        // 变换参数
        glm::vec3 m_position = glm::vec3(0.0f);
        glm::vec3 m_color = glm::vec3(1.0f);
        float m_scale = 1.0f;
        glm::vec3 m_rotation = glm::vec3(0.0f); // 欧拉角（度数）

        // 清理OpenGL资源
        void Cleanup();
    };

} // namespace Renderer
