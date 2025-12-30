#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

namespace Core
{

    class Window
    {
        GLFWwindow *m_window = nullptr;
        int m_width, m_height;
        std::string m_title;

    public:
        Window(int width, int height, const std::string &title);
        ~Window();

        void Init();
        void PollEvents() const;
        void SwapBuffers() const;
        bool ShouldClose() const;
        void SetWindowShouldClose() const;

        // 获取宽高
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }

        // 设置宽高（用于窗口大小变化回调）
        void SetSize(int width, int height) { m_width = width; m_height = height; }
    };

} // namespace Core