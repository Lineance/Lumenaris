#pragma once
#include <GLFW/glfw3.h>
#include "Core/GLM.hpp"
#include <functional>

namespace Core
{

    class MouseController
    {
    public:
        // 构造函数/析构函数
        MouseController();
        ~MouseController() = default;

        // 初始化方法
        void Initialize(GLFWwindow *window);

        // 状态获取方法
        float GetYaw() const { return m_yaw; }
        float GetPitch() const { return m_pitch; }
        float GetFOV() const { return m_fov; }
        bool IsFirstMouse() const { return m_firstMouse; }

        // 摄像机控制方法
        glm::vec3 GetCameraFront() const { return m_cameraFront; }
        void UpdateCameraVectors();

        // 回调设置方法
        static void SetMouseCallback(GLFWwindow *window);
        static void SetScrollCallback(GLFWwindow *window);

        // 静态回调函数（供GLFW调用）
        static void MouseCallback(GLFWwindow *window, double xpos, double ypos);
        static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);

        // 灵敏度设置
        void SetMouseSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }
        void SetScrollSensitivity(float sensitivity) { m_scrollSensitivity = sensitivity; }

    private:
        // 鼠标状态
        float m_yaw;
        float m_pitch;
        float m_fov;

        // 鼠标控制参数
        bool m_firstMouse;
        float m_lastX;
        float m_lastY;
        float m_mouseSensitivity;
        float m_scrollSensitivity;

        // 摄像机方向
        glm::vec3 m_cameraFront;
        glm::vec3 m_cameraUp;

        // 静态实例指针（用于回调函数访问实例成员）
        static MouseController *s_instance;
    };

} // namespace Core