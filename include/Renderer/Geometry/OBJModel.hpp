#pragma once
#include "Renderer/Geometry/Mesh.hpp"
#include "Renderer/Resources/OBJLoader.hpp"
#include "Renderer/Resources/Texture.hpp"
#include "Core/GLM.hpp"
#include <string>
#include <vector>

namespace Renderer
{

    class OBJModel : public IMesh
    {
    public:
        OBJModel();
        explicit OBJModel(const std::string& filepath);
        ~OBJModel();

        // 从IGeometry继承的接口
        void Create() override;
        void Draw() const override;

        // IMesh 接口实现
        unsigned int GetVAO() const override { return m_vao; }
        size_t GetVertexCount() const override { return m_loader.GetVertices().size(); }
        size_t GetIndexCount() const override { return m_loader.GetIndices().size(); }
        bool HasIndices() const override { return m_loader.HasIndices(); }
        bool HasTexture() const override;

        // 按材质渲染
        void DrawWithMaterial(int materialIndex) const;

        // 加载OBJ文件
        bool LoadFromFile(const std::string& filepath);

        // 加载材质和纹理
        void LoadMaterialsAndTextures();

        // 配置接口（与Cube保持一致）
        void SetPosition(const glm::vec3& pos) { m_position = pos; }
        void SetColor(const glm::vec3& color) { m_color = color; }
        void SetScale(float scale) { m_scale = scale; }
        void SetRotation(const glm::vec3& rotation) { m_rotation = rotation; }

        // 获取状态
        const glm::vec3& GetColor() const { return m_color; }
        const std::string& GetFilePath() const { return m_filepath; }
        glm::mat4 GetModelMatrix() const;

        // 获取材质数量
        size_t GetMaterialCount() const { return m_loader.GetMaterials().size(); }

        // 获取材质数据
        const std::vector<OBJMaterial>& GetMaterials() const { return m_loader.GetMaterials(); }

        // 获取当前使用的材质索引
        int GetCurrentMaterialIndex() const { return m_currentMaterialIndex; }

        // 设置当前使用的材质索引
        void SetCurrentMaterialIndex(int index);

        // 获取当前材质
        const OBJMaterial* GetCurrentMaterial() const;

        // 静态方法：为实例化渲染提供按材质分离的顶点数据
        struct MaterialVertexData {
            std::vector<float> vertices;      // 扩展的顶点数据（位置+法线+UV）
            std::vector<unsigned int> indices; // 此材质的索引
            OBJMaterial material;             // 材质信息
            std::string texturePath;          // 纹理路径
        };

        static std::vector<MaterialVertexData> GetMaterialVertexData(const std::string& objPath);

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

        // 材质和纹理
        int m_currentMaterialIndex = 0;
        std::vector<Texture> m_textures; // 存储加载的纹理
        bool m_materialsLoaded = false;

        // 清理OpenGL资源
        void Cleanup();
    };

} // namespace Renderer
