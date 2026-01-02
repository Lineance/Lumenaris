#pragma once
#include "Core/GLM.hpp"
#include <vector>

namespace Renderer
{

    /**
     * @class Cube
     * @brief 立方体几何数据工具类
     *
     * 纯静态工具类，提供立方体的几何数据生成。
     * 不管理 GPU 资源，不维护实例状态。
     *
     * 使用方式：
     * - 使用 MeshDataFactory::CreateCubeBuffer() 创建已上传到 GPU 的缓冲区
     * - 或使用 Cube::GetVertexData() 获取原始顶点数据
     */
    class Cube
    {
    public:
        // 删除默认构造函数（纯静态类）
        Cube() = delete;

        /**
         * @brief 获取立方体的顶点数据
         * @return std::vector<float> 顶点数据数组
         *
         * 顶点布局：每8个float为一个顶点 [x, y, z, nx, ny, nz, u, v]
         * 共6面 × 4顶点/面 × 8float/顶点 = 192 float
         */
        static std::vector<float> GetVertexData();

        /**
         * @brief 获取立方体的顶点布局
         * @param offsets 输出：各属性在顶点中的偏移量
         * @param sizes 输出：各属性的大小
         *
         * 布局：位置(3), 法线(3), UV(2)
         */
        static void GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes);
    };

} // namespace Renderer
