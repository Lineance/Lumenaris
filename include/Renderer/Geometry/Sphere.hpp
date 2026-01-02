#pragma once
#include "Core/GLM.hpp"
#include <vector>

namespace Renderer
{

    /**
     * @class Sphere
     * @brief 球体几何数据工具类
     *
     * 纯静态工具类，提供球体的几何数据生成。
     * 不管理 GPU 资源，不维护实例状态。
     *
     * 使用方式：
     * - 使用 MeshDataFactory::CreateSphereBuffer() 创建已上传到 GPU 的缓冲区
     * - 或使用 Sphere::GetVertexData() 获取原始顶点数据
     */
    class Sphere
    {
    public:
        // 删除默认构造函数（纯静态类）
        Sphere() = delete;

        /**
         * @brief 获取球体的顶点数据
         * @param radius 球体半径（默认 1.0）
         * @param stacks 纬度线数量（默认 20）
         * @param slices 经度线数量（默认 20）
         * @return std::vector<float> 顶点数据数组
         *
         * 顶点布局：每8个float为一个顶点 [x, y, z, nx, ny, nz, u, v]
         */
        static std::vector<float> GetVertexData(float radius = 1.0f, int stacks = 20, int slices = 20);

        /**
         * @brief 获取球体的索引数据
         * @param stacks 纬度线数量
         * @param slices 经度线数量
         * @return std::vector<unsigned int> 索引数据数组
         */
        static std::vector<unsigned int> GetIndexData(int stacks = 20, int slices = 20);

        /**
         * @brief 获取球体的顶点布局
         * @param offsets 输出：各属性在顶点中的偏移量
         * @param sizes 输出：各属性的大小
         *
         * 布局：位置(3), 法线(3), UV(2)
         */
        static void GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes);
    };

} // namespace Renderer
