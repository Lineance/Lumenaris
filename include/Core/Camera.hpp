#pragma once

#include "Core/GLM.hpp"
#include <functional>

namespace Core
{

    /**
     * Camera 类 - 3D摄像机封装
     *
     * 功能：
     * - 管理3D摄像机位置、方向和移动
     * - 计算view矩阵和projection矩阵
     * - 支持WASD+QE六自由度移动控制
     * - 与MouseController配合实现视角旋转
     * - 支持不同的投影模式（透视投影、正交投影）
     * - 可配置的移动速度和鼠标灵敏度
     *
     * 使用方式：
     * 1. 创建Camera实例并设置位置
     * 2. 每帧调用ProcessKeyboard()处理键盘输入
     * 3. 与MouseController配合更新方向向量
     * 4. 使用GetViewMatrix()和GetProjectionMatrix()获取矩阵传给着色器
     */
    class Camera
    {
    public:
        // 构造函数
        Camera(
            const glm::vec3 &position = glm::vec3(0.0f, 0.0f, 0.0f),
            const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f),
            float yaw = -90.0f,
            float pitch = 0.0f);

        // 析构函数
        ~Camera() = default;

        // ========================================
        // 核心功能：矩阵获取
        // ========================================

        /**
         * 获取视图矩阵 (View Matrix)
         * 将世界坐标转换到摄像机坐标系
         */
        glm::mat4 GetViewMatrix() const;

        /**
         * 获取投影矩阵 (Projection Matrix)
         * @param aspect 窗口宽高比 (width / height)
         * @param nearPlane 近裁剪面距离
         * @param farPlane 远裁剪面距离
         */
        glm::mat4 GetProjectionMatrix(float aspect, float nearPlane = 0.1f, float farPlane = 100.0f) const;

        // ========================================
        // 摄像机移动控制
        // ========================================

        /**
         * 处理键盘输入，移动摄像机
         * @param direction 移动方向 (FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN)
         * @param deltaTime 时间间隔（用于不同帧率下的统一速度）
         */
        enum class MovementDirection
        {
            FORWARD,
            BACKWARD,
            LEFT,
            RIGHT,
            UP,
            DOWN
        };
        void ProcessKeyboard(MovementDirection direction, float deltaTime);

        /**
         * 处理鼠标移动，更新摄像机方向
         * @param xoffset 鼠标X轴偏移
         * @param yoffset 鼠标Y轴偏移
         * @param constrainPitch 是否限制俯仰角（默认true，防止万向节死锁）
         */
        void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

        /**
         * 处理鼠标滚轮，调整FOV（实现缩放效果）
         * @param yoffset 滚轮Y轴偏移
         */
        void ProcessMouseScroll(float yoffset);

        // ========================================
        // 属性访问和设置
        // ========================================

        // 位置相关
        const glm::vec3 &GetPosition() const { return m_position; }
        void SetPosition(const glm::vec3 &position) { m_position = position; }

        // 方向向量
        const glm::vec3 &GetFront() const { return m_front; }
        const glm::vec3 &GetUp() const { return m_up; }
        const glm::vec3 &GetRight() const { return m_right; }
        const glm::vec3 &GetWorldUp() const { return m_worldUp; }

        // 欧拉角
        float GetYaw() const { return m_yaw; }
        float GetPitch() const { return m_pitch; }
        void SetYaw(float yaw) { m_yaw = yaw; UpdateCameraVectors(); }
        void SetPitch(float pitch) { m_pitch = pitch; UpdateCameraVectors(); }

        // 视场角 (FOV)
        float GetFOV() const { return m_zoom; }
        void SetFOV(float fov)
        {
            m_zoom = fov;
            // 限制FOV范围
            if (m_zoom < 1.0f)
                m_zoom = 1.0f;
            if (m_zoom > 120.0f)
                m_zoom = 120.0f;
        }

        // 移动速度
        float GetMovementSpeed() const { return m_movementSpeed; }
        void SetMovementSpeed(float speed) { m_movementSpeed = speed; }

        // 鼠标灵敏度
        float GetMouseSensitivity() const { return m_mouseSensitivity; }
        void SetMouseSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }

        // 投影类型
        enum class ProjectionType
        {
            PERSPECTIVE, // 透视投影
            ORTHO        // 正交投影
        };
        ProjectionType GetProjectionType() const { return m_projectionType; }
        void SetProjectionType(ProjectionType type) { m_projectionType = type; }

        // ========================================
        // 实用工具方法
        // ========================================

        /**
         * 重置摄像机到初始状态
         */
        void Reset(
            const glm::vec3 &position = glm::vec3(0.0f, 0.0f, 0.0f),
            const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f),
            float yaw = -90.0f,
            float pitch = 0.0f);

        /**
         * 观察指定目标点
         * @param target 目标位置
         */
        void LookAt(const glm::vec3 &target);

    private:
        // ========================================
        // 成员变量
        // ========================================

        // 摄像机属性
        glm::vec3 m_position;
        glm::vec3 m_front;
        glm::vec3 m_up;
        glm::vec3 m_right;
        glm::vec3 m_worldUp;

        // 欧拉角
        float m_yaw;
        float m_pitch;

        // 摄像机选项
        float m_movementSpeed;
        float m_mouseSensitivity;
        float m_zoom;

        // 投影类型
        ProjectionType m_projectionType;

        // ========================================
        // 私有方法
        // ========================================

        /**
         * 根据更新的欧拉角更新摄像机方向向量
         */
        void UpdateCameraVectors();
    };

} // namespace Core
