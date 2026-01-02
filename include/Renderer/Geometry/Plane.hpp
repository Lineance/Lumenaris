#pragma once
#include "Core/GLM.hpp"
#include <vector>

namespace Renderer
{

    /**
     * @class Plane
     * @brief 平面几何数据工具类
     *
     * 纯静态工具类，提供平面的几何数据生成。
     * 不管理 GPU 资源，不维护实例状态。
     *
     * 使用方式：
     * - 使用 MeshDataFactory::CreatePlaneBuffer() 创建已上传到 GPU 的缓冲区
     * - 或使用 Plane::GetVertexData() 获取原始顶点数据
     */
    class Plane
    {
    public:
        // 删除默认构造函数（纯静态类）
        Plane() = delete;

        /**
         * @brief 获取平面的顶点数据
         * @param width 平面宽度（默认 1.0）
         * @param height 平面高度（默认 1.0）
         * @param widthSegments 宽度方向的分段数（默认 1）
         * @param heightSegments 高度方向的分段数（默认 1）
         * @return std::vector<float> 顶点数据数组
         *
         * 顶点布局：每8个float为一个顶点 [x, y, z, nx, ny, nz, u, v]
         */
        static std::vector<float> GetVertexData(
            float width = 1.0f,
            float height = 1.0f,
            int widthSegments = 1,
            int heightSegments = 1
        );

        /**
         * @brief 获取平面的索引数据
         * @param widthSegments 宽度方向的分段数
         * @param heightSegments 高度方向的分段数
         * @return std::vector<unsigned int> 索引数据数组
         */
        static std::vector<unsigned int> GetIndexData(
            int widthSegments = 1,
            int heightSegments = 1
        );

        /**
         * @brief 获取平面的顶点布局
         * @param offsets 输出：各属性在顶点中的偏移量
         * @param sizes 输出：各属性的大小
         *
         * 布局：位置(3), 法线(3), UV(2)
         */
        static void GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes);
    };

} // namespace Renderer
