#include "Core/KeyboardController.hpp"
#include <iostream>
#include <algorithm>

namespace Core
{

    // 静态成员初始化
    KeyboardController *KeyboardController::s_instance = nullptr;

    KeyboardController::KeyboardController()
    {
        s_instance = this;

        // 预初始化常用按键状态
        std::vector<int> commonKeys = {
            GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
            GLFW_KEY_SPACE, GLFW_KEY_ESCAPE, GLFW_KEY_ENTER,
            GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL};

        for (int key : commonKeys)
        {
            m_keyStates[key] = KeyState();
        }
    }

    KeyboardController::~KeyboardController()
    {
        m_initialized = false;
        s_instance = nullptr;
    }

    void KeyboardController::Initialize(GLFWwindow *window)
    {
        if (!window)
        {
            std::cerr << "错误: 无效的GLFW窗口指针" << std::endl;
            return;
        }

        m_window = window;

        // 设置GLFW键盘回调函数
        glfwSetKeyCallback(window, KeyboardController::KeyCallback);

        m_initialized = true;
        std::cout << "键盘控制器初始化完成" << std::endl;
    }

    void KeyboardController::KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        if (!s_instance)
            return;

        s_instance->ProcessKeyEvent(key, action);
    }

    void KeyboardController::ProcessKeyEvent(int key, int action)
    {
        std::lock_guard<std::mutex> lock(m_keyMutex);

        auto it = m_keyStates.find(key);
        if (it == m_keyStates.end())
        {
            // 如果键不存在，创建新状态
            m_keyStates[key] = KeyState();
            it = m_keyStates.find(key);
        }

        KeyState &state = it->second;

        switch (action)
        {
        case GLFW_PRESS:
            state.current = true;
            state.pressedThisFrame = true;
            state.repeatTimer = 0.0f;

            // 触发回调函数
            if (state.callback)
            {
                state.callback();
            }
            break;

        case GLFW_RELEASE:
            state.current = false;
            state.releasedThisFrame = true;
            state.repeatTimer = 0.0f;
            break;

        case GLFW_REPEAT:
            if (m_keyRepeatEnabled && state.repeatEnabled)
            {
                state.repeatTimer = 0.0f; // 重置计时器，GLFW会自动处理重复
                if (state.callback)
                {
                    state.callback();
                }
            }
            break;
        }
    }

    bool KeyboardController::IsKeyPressed(int key) const
    {
        std::lock_guard<std::mutex> lock(m_keyMutex);

        auto it = m_keyStates.find(key);
        if (it != m_keyStates.end())
        {
            return it->second.current;
        }
        return false;
    }

    bool KeyboardController::IsKeyJustPressed(int key) const
    {
        std::lock_guard<std::mutex> lock(m_keyMutex);

        auto it = m_keyStates.find(key);
        if (it != m_keyStates.end())
        {
            return it->second.pressedThisFrame;
        }
        return false;
    }

    bool KeyboardController::IsKeyJustReleased(int key) const
    {
        std::lock_guard<std::mutex> lock(m_keyMutex);

        auto it = m_keyStates.find(key);
        if (it != m_keyStates.end())
        {
            return it->second.releasedThisFrame;
        }
        return false;
    }

    void KeyboardController::RegisterKeyCallback(int key, std::function<void()> callback,
                                                 bool repeat, float repeatDelay)
    {
        std::lock_guard<std::mutex> lock(m_keyMutex);

        auto it = m_keyStates.find(key);
        if (it == m_keyStates.end())
        {
            m_keyStates[key] = KeyState();
            it = m_keyStates.find(key);
        }

        KeyState &state = it->second;
        state.callback = callback;
        state.repeatEnabled = repeat;
        state.repeatDelay = repeatDelay;
    }

    void KeyboardController::UnregisterKeyCallback(int key)
    {
        std::lock_guard<std::mutex> lock(m_keyMutex);

        auto it = m_keyStates.find(key);
        if (it != m_keyStates.end())
        {
            it->second.callback = nullptr;
            it->second.repeatEnabled = false;
        }
    }

    void KeyboardController::Update(float deltaTime)
    {
        std::lock_guard<std::mutex> lock(m_keyMutex);

        // 更新所有按键状态
        for (auto &pair : m_keyStates)
        {
            KeyState &state = pair.second;

            // 保存上一帧状态
            state.previous = state.current;

            // 重置帧标记
            state.pressedThisFrame = false;
            state.releasedThisFrame = false;

            // 更新重复计时器
            if (state.current && state.repeatEnabled && m_keyRepeatEnabled)
            {
                state.repeatTimer += deltaTime;

                // 检查是否需要触发重复回调
                if (state.repeatTimer >= state.repeatDelay)
                {
                    state.repeatTimer = 0.0f;
                    if (state.callback)
                    {
                        state.callback();
                    }
                }
            }
        }

        // 补充查询GLFW当前状态（确保没有遗漏回调）
        for (auto &pair : m_keyStates)
        {
            int key = pair.first;
            KeyState &state = pair.second;

            // 如果GLFW报告按键状态与内部状态不一致，更新内部状态
            int glfwState = glfwGetKey(m_window, key);
            bool isPressed = (glfwState == GLFW_PRESS);

            if (isPressed != state.current)
            {
                state.current = isPressed;
                if (isPressed)
                {
                    state.pressedThisFrame = true;
                    if (state.callback)
                    {
                        state.callback();
                    }
                }
                else
                {
                    state.releasedThisFrame = true;
                }
            }
        }
    }

} // namespace Core