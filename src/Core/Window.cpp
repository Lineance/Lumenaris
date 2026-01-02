#include "Core/Window.hpp"
#include "Core/Logger.hpp"
#include <iostream>

namespace Core
{

    // 静态回调函数，处理窗口尺寸变化
    static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
    {
        glViewport(0, 0, width, height); // 调整OpenGL视口

        // 更新Window实例的成员变量
        Window *win = static_cast<Window *>(glfwGetWindowUserPointer(window));
        if (win)
        {
            win->SetSize(width, height);
        }
        else
        {
            Core::Logger::GetInstance().Warning("Window resize callback received null window pointer");
        }
    }

    // 构造函数：仅初始化成员变量，不执行实际创建操作，延迟初始化窗口
    Window::Window(int width, int height, const std::string &title)
        : m_width(width), m_height(height), m_title(title)
    {
    }

    Window::~Window()
    {
        if (m_window)
        {
            glfwDestroyWindow(m_window); // 销毁窗口
        }
        glfwTerminate(); // 终止GLFW
        Core::Logger::GetInstance().Info("GLFW terminated");
    }

    void Window::Init() // 创建窗口
    {
        Core::Logger::GetInstance().Info("Initializing window system...");

        // 初始化GLFW
        if (!glfwInit())
        {
            Core::Logger::GetInstance().Error("Failed to initialize GLFW");
            throw std::runtime_error("Failed to initialize GLFW");
        }
        Core::Logger::GetInstance().Info("GLFW initialized successfully");

        // 配置OpenGL上下文版本和模式
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // TODO 针对macOS的兼容性配置

        m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
        // 创建一个窗口, 两个nullptr分别指的是窗口模式和不共享资源

        if (!m_window)
        {
            Core::Logger::GetInstance().Error("Failed to create GLFW window: " + m_title);
            throw std::runtime_error("Failed to create GLFW window");
        }
        Core::Logger::GetInstance().Info("GLFW window created successfully: " + m_title +
                                         " (" + std::to_string(m_width) + "x" + std::to_string(m_height) + ")");

        glfwMakeContextCurrent(m_window);                                    // 设置为当前OpenGL上下文
        glfwSetWindowUserPointer(m_window, this);                            // 设置用户指针以便回调函数访问
        glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback); // 设置窗口回调

        // 初始化GLAD，加载OpenGL函数指针
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            Core::Logger::GetInstance().Error("Failed to initialize GLAD");
            throw std::runtime_error("Failed to initialize GLAD");
        }
        Core::Logger::GetInstance().Info("GLAD initialized successfully - OpenGL functions loaded");

        // 加载成功后，GLAD 会在进行你所有调用，故glClearColor可以直接调用
        Core::Logger::GetInstance().Info("Window system initialization completed");
    }

    // 对于glfw的封装
    void Window::PollEvents() const { glfwPollEvents(); }                                   // 处理事件
    void Window::SwapBuffers() const { glfwSwapBuffers(m_window); }                         // 交换缓冲区
    bool Window::ShouldClose() const { return glfwWindowShouldClose(m_window); }            // 检查关闭条件
    void Window::SetWindowShouldClose() const { glfwSetWindowShouldClose(m_window, true); } // 请求关闭窗口

    // 设置窗口标题
    void Window::SetTitle(const std::string &title)
    {
        m_title = title;
        glfwSetWindowTitle(m_window, title.c_str());
    }

} // namespace Core