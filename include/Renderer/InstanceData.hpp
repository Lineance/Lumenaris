#pragma once
#include "Core/GLM.hpp"
#include <vector>

namespace Renderer
{

    /**
     * @class InstanceData
     * @brief 实例数据容器 - 存储多个实例的变换和颜色信息
     *
     * 职责：
     * - 管理实例的模型矩阵（位置、旋转、缩放）
     * - 管理实例的颜色属性
     * - 提供批量添加和清除实例的接口
     *
     * 设计说明：
     * - 纯数据容器，不涉及渲染逻辑
     * - 可以独立于渲染器进行操作
     * - 支持实例的动态增删
     *
     * 示例：
     * @code
     * InstanceData instances;
     * instances.Add(glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(1), glm::vec3(1,1,1));
     * instances.Add(glm::vec3(5,0,0), glm::vec3(0,90,0), glm::vec3(1), glm::vec3(1,0,0));
     * size_t count = instances.GetCount(); // 2
     * @endcode
     */
    class InstanceData
    {
    public:
        InstanceData() = default;

        // 添加单个实例
        void Add(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale, const glm::vec3& color);

        // 批量添加实例
        void AddBatch(const std::vector<glm::mat4>& matrices, const std::vector<glm::vec3>& colors);

        // 清除所有实例
        void Clear();

        // 获取实例数量
        size_t GetCount() const { return m_modelMatrices.size(); }

        // 数据访问
        const std::vector<glm::mat4>& GetModelMatrices() const { return m_modelMatrices; }
        const std::vector<glm::vec3>& GetColors() const { return m_colors; }
        std::vector<glm::mat4>& GetModelMatrices() { return m_modelMatrices; }
        std::vector<glm::vec3>& GetColors() { return m_colors; }

        // 判断是否为空
        bool IsEmpty() const { return m_modelMatrices.empty(); }

    private:
        std::vector<glm::mat4> m_modelMatrices; // 实例的模型矩阵
        std::vector<glm::vec3> m_colors;        // 实例的颜色
    };

} // namespace Renderer
