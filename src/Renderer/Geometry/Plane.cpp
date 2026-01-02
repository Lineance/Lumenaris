#include "Renderer/Geometry/Plane.hpp"
#include <vector>

namespace Renderer
{

    std::vector<float> Plane::GetVertexData(
        float width,
        float height,
        int widthSegments,
        int heightSegments)
    {
        std::vector<float> vertices;

        float halfWidth = width * 0.5f;
        float halfHeight = height * 0.5f;
        float widthStep = width / widthSegments;
        float heightStep = height / heightSegments;

        // 生成顶点数据
        for (int y = 0; y <= heightSegments; ++y)
        {
            float yPos = -halfHeight + y * heightStep;
            float v = static_cast<float>(y) / heightSegments;

            for (int x = 0; x <= widthSegments; ++x)
            {
                float xPos = -halfWidth + x * widthStep;
                float u = static_cast<float>(x) / widthSegments;

                // 位置（XY平面，Z=0）
                vertices.push_back(xPos);
                vertices.push_back(yPos);
                vertices.push_back(0.0f);

                // 法线（指向+Z方向）
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);

                // UV坐标
                vertices.push_back(u);
                vertices.push_back(v);
            }
        }

        return vertices;
    }

    std::vector<unsigned int> Plane::GetIndexData(
        int widthSegments,
        int heightSegments)
    {
        std::vector<unsigned int> indices;

        // 生成索引数据
        for (int y = 0; y < heightSegments; ++y)
        {
            for (int x = 0; x < widthSegments; ++x)
            {
                int topLeft = y * (widthSegments + 1) + x;
                int topRight = topLeft + 1;
                int bottomLeft = (y + 1) * (widthSegments + 1) + x;
                int bottomRight = bottomLeft + 1;

                // 第一个三角形
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                // 第二个三角形
                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        return indices;
    }

    void Plane::GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes)
    {
        offsets = {0, 3, 6};  // 位置、法线、UV 的偏移
        sizes = {3, 3, 2};     // 位置(3), 法线(3), UV(2)
    }

} // namespace Renderer
