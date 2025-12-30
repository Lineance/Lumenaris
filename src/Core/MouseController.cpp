#include "Core/MouseController.hpp"
#include <iostream>

namespace Core
{

    // 静态成员初始化
    MouseController *MouseController::s_instance = nullptr;

    MouseController::MouseController()
        : m_yaw(-90.0f), m_pitch(0.0f), m_fov(45.0f),
          m_firstMouse(true), m_lastX(400.0f), m_lastY(300.0f),
          m_mouseSensitivity(0.1f), m_scrollSensitivity(1.0f),
          m_cameraFront(0.0f, 0.0f, -1.0f), m_cameraUp(0.0f, 1.0f, 0.0f)
    {
    }

    void MouseController::Initialize(GLFWwindow *window)
    {
        if (!window)
        {
            std::cerr << "错误: 无效的GLFW窗口指针" << std::endl;
            return;
        }

        // 设置静态实例指针
        s_instance = this;

        // 设置GLFW回调函数
        SetMouseCallback(window);
        SetScrollCallback(window);

        // 隐藏并捕捉光标
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        std::cout << "鼠标控制器初始化完成" << std::endl;
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
        if (!s_instance)
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
            s_instance->m_pitch = 89.0f;
        if (s_instance->m_pitch < -89.0f)
            s_instance->m_pitch = -89.0f;

        s_instance->UpdateCameraVectors();
    }

    void MouseController::ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
    {
        if (!s_instance)
            return;

        s_instance->m_fov -= static_cast<float>(yoffset) * s_instance->m_scrollSensitivity;
        if (s_instance->m_fov < 1.0f)
            s_instance->m_fov = 1.0f;
        if (s_instance->m_fov > 45.0f)
            s_instance->m_fov = 45.0f;
    }

    void MouseController::UpdateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front.y = sin(glm::radians(m_pitch));
        front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_cameraFront = glm::normalize(front);
    }

} // namespace Core