#include "Core/MouseController.hpp"
#include "Core/Logger.hpp"
#include <iostream>

namespace Core
{

    // 静态成员初始化
    MouseController *MouseController::s_instance = nullptr;

    MouseController::MouseController()
        : m_yaw(-90.0f), m_pitch(0.0f), m_fov(45.0f),
          m_firstMouse(true), m_mouseCaptured(true), m_lastX(400.0f), m_lastY(300.0f),
          m_mouseSensitivity(0.1f), m_scrollSensitivity(1.0f),
          m_cameraFront(0.0f, 0.0f, -1.0f), m_cameraUp(0.0f, 1.0f, 0.0f)
    {
    }

    void MouseController::Initialize(GLFWwindow *window)
    {
        if (!window)
        {
            Core::Logger::GetInstance().Error("MouseController initialization failed: invalid GLFW window pointer");
            return;
        }

        Core::Logger::GetInstance().Info("Initializing MouseController...");

        // 设置静态实例指针
        s_instance = this;

        // 设置GLFW回调函数
        SetMouseCallback(window);
        SetScrollCallback(window);

        // 隐藏并捕捉光标
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        Core::Logger::GetInstance().Info("MouseController initialized successfully - cursor captured");
    }

    void MouseController::SetMouseCallback(GLFWwindow *window)
    {
        glfwSetCursorPosCallback(window, MouseController::MouseCallback);
    }

    void MouseController::SetScrollCallback(GLFWwindow *window)
    {
        glfwSetScrollCallback(window, MouseController::ScrollCallback);
    }

    void MouseController::MouseCallback(GLFWwindow *window, double xpos, double ypos)
    {
        if (!s_instance || !s_instance->m_mouseCaptured)
            return;

        if (s_instance->m_firstMouse)
        {
            s_instance->m_lastX = static_cast<float>(xpos);
            s_instance->m_lastY = static_cast<float>(ypos);
            s_instance->m_firstMouse = false;
        }

        float xoffset = static_cast<float>(xpos) - s_instance->m_lastX;
        float yoffset = s_instance->m_lastY - static_cast<float>(ypos); // 反转Y轴

        s_instance->m_lastX = static_cast<float>(xpos);
        s_instance->m_lastY = static_cast<float>(ypos);

        xoffset *= s_instance->m_mouseSensitivity;
        yoffset *= s_instance->m_mouseSensitivity;

        s_instance->m_yaw += xoffset;
        s_instance->m_pitch += yoffset;

        // 限制俯仰角
        if (s_instance->m_pitch > 89.0f)
        {
            s_instance->m_pitch = 89.0f;
        }
        if (s_instance->m_pitch < -89.0f)
        {
            s_instance->m_pitch = -89.0f;
        }

        s_instance->UpdateCameraVectors();
    }

    void MouseController::ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
    {
        if (!s_instance)
            return;

        float oldFov = s_instance->m_fov;
        s_instance->m_fov -= static_cast<float>(yoffset) * s_instance->m_scrollSensitivity;

        bool clamped = false;
        if (s_instance->m_fov < 1.0f)
        {
            s_instance->m_fov = 1.0f;
            clamped = true;
        }
        if (s_instance->m_fov > 45.0f)
        {
            s_instance->m_fov = 45.0f;
            clamped = true;
        }

        // FOV clamped or changed - no debug logging
    }

    void MouseController::UpdateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front.y = sin(glm::radians(m_pitch));
        front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_cameraFront = glm::normalize(front);
    }

    void MouseController::ToggleMouseCapture()
    {
        SetMouseCapture(!m_mouseCaptured);
    }

    void MouseController::SetMouseCapture(bool captured)
    {
        if (m_mouseCaptured == captured)
            return;

        m_mouseCaptured = captured;
        Core::Logger::GetInstance().Info("Mouse capture " + std::string(captured ? "enabled" : "disabled"));

        GLFWwindow *window = glfwGetCurrentContext();
        if (window)
        {
            if (captured)
            {
                // 隐藏并捕捉光标
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                m_firstMouse = true; // 重置鼠标状态以避免跳跃
            }
            else
            {
                // 显示并释放光标
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
        else
        {
            Core::Logger::GetInstance().Warning("Failed to change mouse capture mode: no current GLFW context");
        }
    }

} // namespace Core