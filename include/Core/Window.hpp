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
    };

} // namespace Core