#include "Core/Window.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

void processInput(const Core::Window &window)
{
    // 通过 GLFW 直接获取按键
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        window.SetWindowShouldClose();
    }
}

int main()
{
    try
    {
        Core::Window window(800, 600, "DarkRoom Engine");
        window.Init();

        while (!window.ShouldClose())
        {
            processInput(window);

            glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            window.SwapBuffers();
            window.PollEvents();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}