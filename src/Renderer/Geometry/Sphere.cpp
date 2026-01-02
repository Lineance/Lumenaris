#include "Renderer/Geometry/Sphere.hpp"
#include <cmath>

#define PI 3.14159265358979323846f

namespace Renderer
{

    std::vector<float> Sphere::GetVertexData(float radius, int stacks, int slices)
    {
        std::vector<float> vertices;
        vertices.reserve((stacks + 1) * (slices + 1) * 8); // 每个顶点8个float

        // 生成顶点数据
        for (int stack = 0; stack <= stacks; ++stack)
        {
            float phi = PI * stack / stacks; // 纬度角度 [0, PI]
            float y = radius * cosf(phi);
            float r = radius * sinf(phi);

            for (int slice = 0; slice <= slices; ++slice)
            {
                float theta = 2.0f * PI * slice / slices; // 经度角度 [0, 2PI]

                // 位置
                float x = r * cosf(theta);
                float z = r * sinf(theta);

                // 法线 (归一化位置向量)
                float nx = x / radius;
                float ny = y / radius;
                float nz = z / radius;

                // UV坐标
                float u = static_cast<float>(slice) / slices;
                float v = static_cast<float>(stack) / stacks;

                // 添加顶点数据: 位置(3) + 法线(3) + UV(2) = 8 floats
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                vertices.push_back(nx);
                vertices.push_back(ny);
                vertices.push_back(nz);
                vertices.push_back(u);
                vertices.push_back(v);
            }
        }

        return vertices;
    }

    std::vector<unsigned int> Sphere::GetIndexData(int stacks, int slices)
    {
        std::vector<unsigned int> indices;
        indices.reserve(stacks * slices * 6); // 每个面6个索引

        // 生成索引数据
        for (int stack = 0; stack < stacks; ++stack)
        {
            for (int slice = 0; slice < slices; ++slice)
            {
                int first = stack * (slices + 1) + slice;
                int second = first + slices + 1;

                // 第一个三角形
                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                // 第二个三角形
                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }

        return indices;
    }

    void Sphere::GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes)
    {
        offsets = {0, 3, 6};  // 位置、法线、UV 的偏移
        sizes = {3, 3, 2};     // 位置(3), 法线(3), UV(2)
    }

} // namespace Renderer
