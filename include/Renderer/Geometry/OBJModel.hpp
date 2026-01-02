#pragma once

#include "Renderer/Resources/OBJLoader.hpp"
#include "Renderer/Resources/Texture.hpp"
#include "Renderer/Data/MeshData.hpp"
#include <string>
#include <vector>

namespace Renderer
{

    /**
     * @class OBJModel
     * @brief OBJ 模型数据工具类
     *
     * 纯静态工具类，提供 OBJ 文件的解析和数据提取功能。
     * 不管理 GPU 资源，不维护实例状态。
     *
     * 使用方式：
     * - 使用 MeshDataFactory::CreateOBJBuffers() 创建已上传到 GPU 的缓冲区
     * - 或使用 OBJModel::GetMaterialVertexData() 获取原始顶点数据
     *
     * @note 2026-01-02 重构：从实例类改为纯静态工具类
     */
    class OBJModel
    {
    public:
        // 删除默认构造函数（纯静态类）
        OBJModel() = delete;

        /**
         * @brief 材质顶点数据结构
         *
         * 包含单个材质的所有顶点数据和材质信息
         */
        struct MaterialVertexData {
            std::vector<float> vertices;        // 顶点数据（位置+法线+UV）
            std::vector<unsigned int> indices;   // 索引数据
            OBJMaterial material;              // 材质信息
            std::string texturePath;           // 纹理路径
        };

        /**
         * @brief 从 OBJ 文件获取按材质分离的顶点数据
         * @param objPath OBJ 文件路径
         * @return std::vector<MaterialVertexData> 每个材质的顶点数据列表
         *
         * @note 这是推荐的方法，用于实例化渲染
         */
        static std::vector<MaterialVertexData> GetMaterialVertexData(const std::string& objPath);

        /**
         * @brief 从 OBJ 文件获取合并的 MeshData
         * @param objPath OBJ 文件路径
         * @return MeshData 包含所有材质的合并数据
         *
         * @note 此方法会合并所有材质，适用于不需要材质分离的场景
         */
        static MeshData GetMeshData(const std::string& objPath);

        /**
         * @brief 获取 OBJ 文件的材质信息
         * @param objPath OBJ 文件路径
         * @return std::vector<OBJMaterial> 材质列表
         */
        static std::vector<OBJMaterial> GetMaterials(const std::string& objPath);

        /**
         * @brief 检查 OBJ 文件是否包含材质
         * @param objPath OBJ 文件路径
         * @return bool 是否包含材质
         */
        static bool HasMaterials(const std::string& objPath);

        /**
         * @brief 获取顶点布局
         * @param offsets 输出：各属性在顶点中的偏移量
         * @param sizes 输出：各属性的大小
         *
         * 布局：位置(3), 法线(3), UV(2)
         */
        static void GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes);
    };

} // namespace Renderer
