#pragma once
#include "Core/GLM.hpp"
#include <vector>

namespace Renderer
{

    /**
     * @class Torus
     * @brief 圆环几何数据工具类
     *
     * 纯静态工具类，提供圆环的几何数据生成。
     * 不管理 GPU 资源，不维护实例状态。
     *
     * 使用方式：
     * - 使用 MeshDataFactory::CreateTorusBuffer() 创建已上传到 GPU 的缓冲区
     * - 或使用 Torus::GetVertexData() 获取原始顶点数据
     */
    class Torus
    {
    public:
        // 删除默认构造函数（纯静态类）
        Torus() = delete;

        /**
         * @brief 获取圆环的顶点数据
         * @param majorRadius 主半径（环的中心到管中心的距离，默认 1.0）
         * @param minorRadius 次半径（管的半径，默认 0.3）
         * @param majorSegments 主分段数（环绕环的分段数，默认 32）
         * @param minorSegments 次分段数（环绕管的分段数，默认 16）
         * @return std::vector<float> 顶点数据数组
         *
         * 顶点布局：每8个float为一个顶点 [x, y, z, nx, ny, nz, u, v]
         */
        static std::vector<float> GetVertexData(
            float majorRadius = 1.0f,
            float minorRadius = 0.3f,
            int majorSegments = 32,
            int minorSegments = 16
        );

        /**
         * @brief 获取圆环的索引数据
         * @param majorSegments 主分段数
         * @param minorSegments 次分段数
         * @return std::vector<unsigned int> 索引数据数组
         */
        static std::vector<unsigned int> GetIndexData(
            int majorSegments = 32,
            int minorSegments = 16
        );

        /**
         * @brief 获取圆环的顶点布局
         * @param offsets 输出：各属性在顶点中的偏移量
         * @param sizes 输出：各属性的大小
         *
         * 布局：位置(3), 法线(3), UV(2)
         */
        static void GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes);
    };

} // namespace Renderer
