#include "Renderer/Factory/MeshDataFactory.hpp"
#include "Renderer/Geometry/Cube.hpp"
#include "Renderer/Geometry/Sphere.hpp"
#include "Renderer/Geometry/Torus.hpp"
#include "Renderer/Geometry/Plane.hpp"
#include "Renderer/Geometry/OBJModel.hpp"
#include "Core/Logger.hpp"
#include <glad/glad.h>

namespace Renderer
{

    // ============================================================
    // MeshDataFactory 实现
    // ============================================================

    MeshData MeshDataFactory::CreateCubeData()
    {
        // 使用 Cube 类的静态方法获取顶点数据
        std::vector<float> vertices = Cube::GetVertexData();

        std::vector<size_t> offsets;
        std::vector<int> sizes;
        Cube::GetVertexLayout(offsets, sizes);

        MeshData data;
        // ✅ 性能优化：使用移动语义避免数据拷贝
        data.SetVertices(std::move(vertices), 8);  // stride = 8 (pos:3 + normal:3 + uv:2)
        data.SetVertexLayout(offsets, sizes);
        data.SetMaterialColor(glm::vec3(1.0f));

        Core::Logger::GetInstance().Debug("MeshDataFactory::CreateCubeData() - Created cube data: " +
                                          std::to_string(data.GetVertexCount()) + " vertices");

        return data;
    }

    MeshData MeshDataFactory::CreateSphereData(int stacks, int slices, float radius)
    {
        // ✅ 性能优化：预分配内存，避免多次重新分配
        size_t totalVertices = (stacks + 1) * (slices + 1);
        size_t vertexDataSize = totalVertices * 8;  // 8 floats per vertex (pos:3 + normal:3 + uv:2)
        size_t totalIndices = stacks * slices * 6;  // 每个网格单元 2 个三角形 = 6 个索引

        std::vector<float> vertices;
        vertices.reserve(vertexDataSize);

        std::vector<unsigned int> indices;
        indices.reserve(totalIndices);

        // 参数化球体生成
        float stackStep = glm::pi<float>() / stacks;
        float sliceStep = 2.0f * glm::pi<float>() / slices;

        for (int i = 0; i <= stacks; ++i)
        {
            float stackAngle = glm::pi<float>() / 2 - i * stackStep;
            float xy = radius * cosf(stackAngle);
            float z = radius * sinf(stackAngle);

            for (int j = 0; j <= slices; ++j)
            {
                float sliceAngle = j * sliceStep;

                // 顶点位置
                glm::vec3 pos(
                    xy * cosf(sliceAngle),
                    xy * sinf(sliceAngle),
                    z
                );

                // 顶点法线（对于单位球，法线 = 归一化的位置）
                glm::vec3 normal = glm::normalize(pos);

                // UV 坐标
                glm::vec2 uv(
                    static_cast<float>(j) / slices,
                    static_cast<float>(i) / stacks
                );

                // 添加到顶点数组：pos(3) + normal(3) + uv(2) = 8 floats
                vertices.push_back(pos.x);
                vertices.push_back(pos.y);
                vertices.push_back(pos.z);
                vertices.push_back(normal.x);
                vertices.push_back(normal.y);
                vertices.push_back(normal.z);
                vertices.push_back(uv.x);
                vertices.push_back(uv.y);
            }
        }

        // 生成索引
        for (int i = 0; i < stacks; ++i)
        {
            for (int j = 0; j < slices; ++j)
            {
                unsigned int p0 = i * (slices + 1) + j;
                unsigned int p1 = p0 + slices + 1;

                // 第一个三角形
                indices.push_back(p0);
                indices.push_back(p1);
                indices.push_back(p0 + 1);

                // 第二个三角形
                indices.push_back(p0 + 1);
                indices.push_back(p1);
                indices.push_back(p1 + 1);
            }
        }

        MeshData data;
        // ✅ 性能优化：使用移动语义避免数据拷贝
        data.SetVertices(std::move(vertices), 8);  // stride = 8
        data.SetIndices(std::move(indices));
        data.SetVertexLayout({0, 3, 6}, {3, 3, 2});  // pos, normal, uv
        data.SetMaterialColor(glm::vec3(1.0f));

        Core::Logger::GetInstance().Debug("MeshDataFactory::CreateSphereData() - Created sphere data: " +
                                          std::to_string(data.GetVertexCount()) + " vertices, " +
                                          std::to_string(data.GetIndexCount()) + " indices");

        return data;
    }

    MeshData MeshDataFactory::CreateTorusData(float majorRadius, float minorRadius,
                                               int majorSegments, int minorSegments)
    {
        // 使用 Torus 类的静态方法获取顶点数据
        std::vector<float> vertices = Renderer::Torus::GetVertexData();
        std::vector<unsigned int> indices = Renderer::Torus::GetIndexData();

        MeshData data;
        data.SetVertices(std::move(vertices), 8);
        data.SetIndices(std::move(indices));
        data.SetVertexLayout({0, 3, 6}, {3, 3, 2});
        data.SetMaterialColor(glm::vec3(1.0f));

        Core::Logger::GetInstance().Debug("MeshDataFactory::CreateTorusData() - Created torus data: " +
                                          std::to_string(data.GetVertexCount()) + " vertices, " +
                                          std::to_string(data.GetIndexCount()) + " indices");

        return data;
    }

    MeshData MeshDataFactory::CreatePlaneData(float width, float height,
                                               int widthSegments, int heightSegments)
    {
        // 使用 Plane 类的静态方法获取顶点数据
        std::vector<float> vertices = Renderer::Plane::GetVertexData();
        std::vector<unsigned int> indices = Renderer::Plane::GetIndexData();

        MeshData data;
        data.SetVertices(std::move(vertices), 8);
        data.SetIndices(std::move(indices));
        data.SetVertexLayout({0, 3, 6}, {3, 3, 2});
        data.SetMaterialColor(glm::vec3(1.0f));

        Core::Logger::GetInstance().Debug("MeshDataFactory::CreatePlaneData() - Created plane data: " +
                                          std::to_string(data.GetVertexCount()) + " vertices, " +
                                          std::to_string(data.GetIndexCount()) + " indices");

        return data;
    }

    std::vector<MeshData> MeshDataFactory::CreateOBJData(const std::string& objPath)
    {
        // 使用 OBJModel 的静态方法获取按材质分离的顶点数据
        std::vector<OBJModel::MaterialVertexData> materialDataList =
            OBJModel::GetMaterialVertexData(objPath);

        std::vector<MeshData> dataList;
        dataList.reserve(materialDataList.size());

        for (auto& materialData : materialDataList)  // ✅ 改为非 const 引用以支持移动
        {
            MeshData data;

            // OBJ 的顶点格式：pos(3) + normal(3) + uv(2) = 8 floats
            // ✅ 性能优化：使用移动语义避免数据拷贝
            data.SetVertices(std::move(materialData.vertices), 8);

            if (!materialData.indices.empty())
            {
                data.SetIndices(std::move(materialData.indices));
            }

            // OBJ 的标准布局：位置、法线、UV
            data.SetVertexLayout({0, 3, 6}, {3, 3, 2});

            // 使用材质的漫反射颜色
            data.SetMaterialColor(materialData.material.diffuse);

            dataList.push_back(std::move(data));
        }

        Core::Logger::GetInstance().Info("MeshDataFactory::CreateOBJData() - Created " +
                                         std::to_string(dataList.size()) + " mesh data from " + objPath);

        return dataList;
    }

    MeshData MeshDataFactory::ExtractFromCube(const Cube& cube)
    {
        // Cube 已经有静态方法提供顶点数据
        return CreateCubeData();
    }

    MeshData MeshDataFactory::ExtractFromSphere(const Sphere& sphere)
    {
        // 需要从 Sphere 对象获取参数
        // 暂时使用默认参数
        return CreateSphereData(32, 32, sphere.GetRadius());
    }

    // ============================================================
    // MeshBufferFactory 实现
    // ============================================================

    MeshBuffer MeshBufferFactory::CreateCubeBuffer()
    {
        MeshData data = MeshDataFactory::CreateCubeData();
        return CreateFromMeshData(data);
    }

    MeshBuffer MeshBufferFactory::CreateSphereBuffer(int stacks, int slices, float radius)
    {
        MeshData data = MeshDataFactory::CreateSphereData(stacks, slices, radius);
        return CreateFromMeshData(data);
    }

    MeshBuffer MeshBufferFactory::CreateTorusBuffer(float majorRadius, float minorRadius,
                                                     int majorSegments, int minorSegments)
    {
        MeshData data = MeshDataFactory::CreateTorusData(majorRadius, minorRadius, majorSegments, minorSegments);
        return CreateFromMeshData(data);
    }

    MeshBuffer MeshBufferFactory::CreatePlaneBuffer(float width, float height,
                                                     int widthSegments, int heightSegments)
    {
        MeshData data = MeshDataFactory::CreatePlaneData(width, height, widthSegments, heightSegments);
        return CreateFromMeshData(data);
    }

    std::vector<MeshBuffer> MeshBufferFactory::CreateOBJBuffers(const std::string& objPath)
    {
        std::vector<MeshData> dataList = MeshDataFactory::CreateOBJData(objPath);
        return CreateFromMeshDataList(dataList);
    }

    MeshBuffer MeshBufferFactory::CreateFromMeshData(const MeshData& data)
    {
        MeshBuffer buffer;
        buffer.UploadToGPU(data);
        return buffer;
    }

    MeshBuffer MeshBufferFactory::CreateFromMeshData(MeshData&& data)
    {
        MeshBuffer buffer;
        // ✅ 性能优化：使用移动语义避免数据拷贝
        buffer.UploadToGPU(std::move(data));
        return buffer;
    }

    std::vector<MeshBuffer> MeshBufferFactory::CreateFromMeshDataList(const std::vector<MeshData>& dataList)
    {
        std::vector<MeshBuffer> buffers;
        buffers.reserve(dataList.size());

        for (const auto& data : dataList)
        {
            buffers.push_back(CreateFromMeshData(data));
        }

        Core::Logger::GetInstance().Info("MeshBufferFactory::CreateFromMeshDataList() - Created " +
                                         std::to_string(buffers.size()) + " mesh buffers");

        return buffers;
    }

} // namespace Renderer
