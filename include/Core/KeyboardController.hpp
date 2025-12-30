#pragma once
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <functional>
#include <atomic>
#include <thread>
#include <mutex>

namespace Core
{

    class KeyboardController
    {
    public:
        // 构造函数/析构函数
        KeyboardController();
        ~KeyboardController();

        // 初始化方法
        void Initialize(GLFWwindow *window);

        // 状态获取方法
        bool IsKeyPressed(int key) const;
        bool IsKeyJustPressed(int key) const;
        bool IsKeyJustReleased(int key) const;

        // 按键注册方法
        void RegisterKeyCallback(int key, std::function<void()> callback,
                                 bool repeat = false, float repeatDelay = 0.1f);
        void UnregisterKeyCallback(int key);

        // 配置方法
        void SetKeyRepeatEnabled(bool enabled) { m_keyRepeatEnabled = enabled; }
        void SetKeyRepeatDelay(float delay) { m_keyRepeatDelay = delay; }

        // 更新方法（每帧调用）
        void Update(float deltaTime);

        // 静态回调函数（供GLFW调用）
        static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    private:
        // 按键状态结构
        struct KeyState
        {
            bool current = false;
            bool previous = false;
            bool pressedThisFrame = false;
            bool releasedThisFrame = false;
            float repeatTimer = 0.0f;
            bool repeatEnabled = false;
            float repeatDelay = 0.1f;
            std::function<void()> callback = nullptr;
        };

        // 键盘状态
        std::unordered_map<int, KeyState> m_keyStates;
        mutable std::mutex m_keyMutex;

        // 控制参数
        std::atomic<bool> m_initialized{false};
        std::atomic<bool> m_keyRepeatEnabled{true};
        std::atomic<float> m_keyRepeatDelay{0.1f};

        // 窗口引用
        GLFWwindow *m_window = nullptr;

        // 静态实例指针（用于回调函数访问实例成员）
        static KeyboardController *s_instance;

        // 内部方法
        void ProcessKeyEvent(int key, int action);
        void UpdateKeyState(int key, bool state, float deltaTime);
    };

} // namespace Core