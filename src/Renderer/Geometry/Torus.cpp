#include "Renderer/Geometry/Torus.hpp"
#include <cmath>

#define PI 3.14159265358979323846f

namespace Renderer
{

    std::vector<float> Torus::GetVertexData(
        float majorRadius,
        float minorRadius,
        int majorSegments,
        int minorSegments)
    {
        std::vector<float> vertices;
        vertices.reserve((majorSegments + 1) * (minorSegments + 1) * 8);

        for (int i = 0; i <= majorSegments; ++i)
        {
            float u = static_cast<float>(i) / majorSegments;
            float theta = u * 2.0f * PI; // 主角度 [0, 2PI]

            float cosTheta = cosf(theta);
            float sinTheta = sinf(theta);

            for (int j = 0; j <= minorSegments; ++j)
            {
                float v = static_cast<float>(j) / minorSegments;
                float phi = v * 2.0f * PI; // 次角度 [0, 2PI]

                float cosPhi = cosf(phi);
                float sinPhi = sinf(phi);

                // 位置
                float x = (majorRadius + minorRadius * cosPhi) * cosTheta;
                float y = minorRadius * sinPhi;
                float z = (majorRadius + minorRadius * cosPhi) * sinTheta;

                // 法线
                float nx = cosPhi * cosTheta;
                float ny = sinPhi;
                float nz = cosPhi * sinTheta;

                // UV坐标
                float uCoord = u;
                float vCoord = v;

                // 添加顶点数据: 位置(3) + 法线(3) + UV(2) = 8 floats
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                vertices.push_back(nx);
                vertices.push_back(ny);
                vertices.push_back(nz);
                vertices.push_back(uCoord);
                vertices.push_back(vCoord);
            }
        }

        return vertices;
    }

    std::vector<unsigned int> Torus::GetIndexData(
        int majorSegments,
        int minorSegments)
    {
        std::vector<unsigned int> indices;
        indices.reserve(majorSegments * minorSegments * 6);

        for (int i = 0; i < majorSegments; ++i)
        {
            for (int j = 0; j < minorSegments; ++j)
            {
                int first = i * (minorSegments + 1) + j;
                int second = first + minorSegments + 1;

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

    void Torus::GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes)
    {
        offsets = {0, 3, 6};  // 位置、法线、UV 的偏移
        sizes = {3, 3, 2};     // 位置(3), 法线(3), UV(2)
    }

} // namespace Renderer
