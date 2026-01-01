#pragma once
#include "Renderer/MeshData.hpp"
#include "Renderer/MeshBuffer.hpp"  // 前向声明改为完整包含
#include <string>
#include <vector>

namespace Renderer
{

    /**
     * @class MeshDataFactory
     * @brief 网格数据工厂 - 创建各种几何体的 MeshData
     *
     * 职责：
     * - 从几何体（Cube/Sphere）生成 MeshData
     * - 从 OBJ 模型生成 MeshData
     * - 提供 CPU 纯数据，不涉及 GPU 资源
     */
    class MeshDataFactory
    {
    public:
        // ============================================================
        // 基础几何体
        // ============================================================

        /**
         * @brief 创建立方体数据
         * @return MeshData 包含立方体的顶点数据
         */
        static MeshData CreateCubeData();

        /**
         * @brief 创建球体数据
         * @param stacks 纬度分段数
         * @param slices 经度分段数
         * @param radius 半径
         * @return MeshData 包含球体的顶点和索引数据
         */
        static MeshData CreateSphereData(int stacks = 32, int slices = 32, float radius = 1.0f);

        // ============================================================
        // OBJ 模型
        // ============================================================

        /**
         * @brief 从 OBJ 文件创建网格数据
         * @param objPath OBJ 文件路径
         * @return std::vector<MeshData> 每个材质对应一个 MeshData
         *
         * @note
         * - OBJ 模型可能包含多个材质
         * - 每个材质需要单独的 MeshData
         * - 返回的顺序与材质在 OBJ 文件中的出现顺序一致
         */
        static std::vector<MeshData> CreateOBJData(const std::string& objPath);

        // ============================================================
        // 工具方法
        // ============================================================

        /**
         * @brief 从 Cube 对象提取 MeshData
         * @param cube Cube 对象引用
         * @return MeshData 包含 Cube 的顶点数据
         *
         * @note 这是一个静态辅助方法，用于从现有的 Cube 对象提取数据
         */
        static MeshData ExtractFromCube(const class Cube& cube);

        /**
         * @brief 从 Sphere 对象提取 MeshData
         * @param sphere Sphere 对象引用
         * @return MeshData 包含 Sphere 的顶点和索引数据
         */
        static MeshData ExtractFromSphere(const class Sphere& sphere);
    };

    /**
     * @class MeshBufferFactory
     * @brief 网格缓冲区工厂 - 创建已上传到 GPU 的 MeshBuffer
     *
     * 职责：
     * - 创建 MeshBuffer 并自动上传到 GPU
     * - 提供便捷的创建方法
     */
    class MeshBufferFactory
    {
    public:
        // ============================================================
        // 基础几何体（自动上传到 GPU）
        // ============================================================

        /**
         * @brief 创建立方体缓冲区（已上传到 GPU）
         * @return MeshBuffer 包含立方体的 GPU 资源
         */
        static MeshBuffer CreateCubeBuffer();

        /**
         * @brief 创建球体缓冲区（已上传到 GPU）
         * @param stacks 纬度分段数
         * @param slices 经度分段数
         * @param radius 半径
         * @return MeshBuffer 包含球体的 GPU 资源
         */
        static MeshBuffer CreateSphereBuffer(int stacks = 32, int slices = 32, float radius = 1.0f);

        // ============================================================
        // OBJ 模型（自动上传到 GPU）
        // ============================================================

        /**
         * @brief 从 OBJ 文件创建网格缓冲区（已上传到 GPU）
         * @param objPath OBJ 文件路径
         * @return std::vector<MeshBuffer> 每个材质对应一个 MeshBuffer
         *
         * @note
         * - 返回的 MeshBuffer 已经调用过 UploadToGPU()
         * - 可以直接传递给 InstancedRenderer 使用
         */
        static std::vector<MeshBuffer> CreateOBJBuffers(const std::string& objPath);

        // ============================================================
        // 从 MeshData 创建
        // ============================================================

        /**
         * @brief 从 MeshData 创建 MeshBuffer 并上传到 GPU（左值引用版本）
         * @param data 网格数据
         * @return MeshBuffer 已上传到 GPU
         */
        static MeshBuffer CreateFromMeshData(const MeshData& data);

        /**
         * @brief 从 MeshData 创建 MeshBuffer 并上传到 GPU（右值引用版本，移动语义）
         * @param data 网格数据
         * @return MeshBuffer 已上传到 GPU
         * @note 使用移动语义避免数据拷贝，性能更优
         */
        static MeshBuffer CreateFromMeshData(MeshData&& data);

        /**
         * @brief 批量创建 MeshBuffer 并上传到 GPU
         * @param dataList 网格数据列表
         * @return std::vector<MeshBuffer> 已上传到 GPU 的缓冲区列表
         */
        static std::vector<MeshBuffer> CreateFromMeshDataList(const std::vector<MeshData>& dataList);
    };

} // namespace Renderer
