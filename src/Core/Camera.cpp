#include "Core/Camera.hpp"
#include "Core/Logger.hpp"
#include "Core/GLM.hpp"
#include <cmath>

namespace Core
{

    // ========================================
    // 构造函数
    // ========================================

    Camera::Camera(
        const glm::vec3 &position,
        const glm::vec3 &up,
        float yaw,
        float pitch)
        : m_front(glm::vec3(0.0f, 0.0f, -1.0f)),
          m_movementSpeed(15.0f),
          m_mouseSensitivity(0.1f),
          m_zoom(45.0f),
          m_projectionType(ProjectionType::PERSPECTIVE)
    {
        m_position = position;
        m_worldUp = up;
        m_yaw = yaw;
        m_pitch = pitch;
        UpdateCameraVectors();

        Core::Logger::GetInstance().Info("Camera initialized at position (" +
                                         std::to_string(m_position.x) + ", " +
                                         std::to_string(m_position.y) + ", " +
                                         std::to_string(m_position.z) + ")");
    }

    // ========================================
    // 核心功能：矩阵获取
    // ========================================

    glm::mat4 Camera::GetViewMatrix() const
    {
        // 使用GLM的lookAt函数生成视图矩阵
        // 参数：摄像机位置、目标位置（位置+前向向量）、上向量
        return glm::lookAt(m_position, m_position + m_front, m_up);
    }

    glm::mat4 Camera::GetProjectionMatrix(float aspect, float nearPlane, float farPlane) const
    {
        if (m_projectionType == ProjectionType::PERSPECTIVE)
        {
            // 透视投影：符合人眼的近大远小效果
            return glm::perspective(glm::radians(m_zoom), aspect, nearPlane, farPlane);
        }
        else
        {
            // 正交投影：没有透视效果，物体大小与距离无关
            float halfWidth = (m_zoom / 2.0f) * aspect;
            float halfHeight = m_zoom / 2.0f;
            return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane, farPlane);
        }
    }

    // ========================================
    // 摄像机移动控制
    // ========================================

    void Camera::ProcessKeyboard(MovementDirection direction, float deltaTime)
    {
        // 计算移动速度（使用deltaTime确保不同帧率下速度一致）
        float velocity = m_movementSpeed * deltaTime;

        switch (direction)
        {
        case MovementDirection::FORWARD:
            m_position += m_front * velocity;
            break;
        case MovementDirection::BACKWARD:
            m_position -= m_front * velocity;
            break;
        case MovementDirection::LEFT:
            m_position -= m_right * velocity;
            break;
        case MovementDirection::RIGHT:
            m_position += m_right * velocity;
            break;
        case MovementDirection::UP:
            m_position += m_worldUp * velocity;
            break;
        case MovementDirection::DOWN:
            m_position -= m_worldUp * velocity;
            break;
        }
    }

    void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
    {
        // 应用鼠标灵敏度
        xoffset *= m_mouseSensitivity;
        yoffset *= m_mouseSensitivity;

        // 更新欧拉角
        m_yaw += xoffset;
        m_pitch += yoffset;

        // 限制俯仰角，防止万向节死锁（Gimbal Lock）
        if (constrainPitch)
        {
            if (m_pitch > 89.0f)
            {
                m_pitch = 89.0f;
            }
            if (m_pitch < -89.0f)
            {
                m_pitch = -89.0f;
            }
        }

        // 更新摄像机方向向量
        UpdateCameraVectors();
    }

    void Camera::ProcessMouseScroll(float yoffset)
    {
        // 滚轮向上减小FOV（放大），滚轮向下增大FOV（缩小）
        m_zoom -= yoffset;

        // 限制FOV范围
        if (m_zoom < 1.0f)
        {
            m_zoom = 1.0f;
        }
        if (m_zoom > 120.0f)
        {
            m_zoom = 120.0f;
        }
    }

    // ========================================
    // 属性设置
    // ========================================

    void Camera::Reset(
        const glm::vec3 &position,
        const glm::vec3 &up,
        float yaw,
        float pitch)
    {
        m_position = position;
        m_worldUp = up;
        m_yaw = yaw;
        m_pitch = pitch;
        m_zoom = 45.0f;
        m_front = glm::vec3(0.0f, 0.0f, -1.0f);
        UpdateCameraVectors();

        Core::Logger::GetInstance().Info("Camera reset to initial position");
    }

    void Camera::LookAt(const glm::vec3 &target)
    {
        // 计算从摄像机到目标的方向向量
        glm::vec3 direction = glm::normalize(target - m_position);

        // 从方向向量计算欧拉角
        m_pitch = glm::degrees(std::asin(direction.y));
        m_yaw = glm::degrees(std::atan2(direction.z, direction.x));

        // 更新方向向量
        UpdateCameraVectors();

        Core::Logger::GetInstance().Info("Camera looking at target (" +
                                         std::to_string(target.x) + ", " +
                                         std::to_string(target.y) + ", " +
                                         std::to_string(target.z) + ")");
    }

    // ========================================
    // 私有方法
    // ========================================

    void Camera::UpdateCameraVectors()
    {
        // 根据欧拉角计算前向向量
        // 使用球坐标系转换为笛卡尔坐标系
        glm::vec3 front;
        front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front.y = sin(glm::radians(m_pitch));
        front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_front = glm::normalize(front);

        // 重新计算右向量和上向量
        // 叉乘得到垂直于前向量的向量
        m_right = glm::normalize(glm::cross(m_front, m_worldUp));
        m_up = glm::normalize(glm::cross(m_right, m_front));
    }

} // namespace Core
