#include "Renderer/InstanceData.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Renderer
{

    void InstanceData::Add(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale, const glm::vec3& color)
    {
        // 计算模型矩阵（平移 -> 旋转 -> 缩放）
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);

        m_modelMatrices.push_back(model);
        m_colors.push_back(color);
    }

    void InstanceData::AddBatch(const std::vector<glm::mat4>& matrices, const std::vector<glm::vec3>& colors)
    {
        size_t newSize = matrices.size();
        size_t currentSize = m_modelMatrices.size();

        // 预留容量，避免多次重新分配
        if (m_modelMatrices.capacity() < currentSize + newSize)
        {
            // 预留额外 20% 的空间，减少后续重新分配
            size_t reserveSize = currentSize + newSize + (newSize / 5);
            m_modelMatrices.reserve(reserveSize);
            m_colors.reserve(reserveSize);
        }

        if (matrices.size() != colors.size())
        {
            // 如果大小不匹配，只添加最小数量的数据
            size_t minSize = std::min(matrices.size(), colors.size());
            m_modelMatrices.insert(m_modelMatrices.end(), matrices.begin(), matrices.begin() + minSize);
            m_colors.insert(m_colors.end(), colors.begin(), colors.begin() + minSize);
        }
        else
        {
            m_modelMatrices.insert(m_modelMatrices.end(), matrices.begin(), matrices.end());
            m_colors.insert(m_colors.end(), colors.begin(), colors.end());
        }
    }

    void InstanceData::Clear()
    {
        m_modelMatrices.clear();
        m_colors.clear();
    }

} // namespace Renderer
