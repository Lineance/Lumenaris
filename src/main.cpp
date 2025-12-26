#include "Core/Window.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Geometry.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>

int main()
{
    try
    {
        Core::Window window(800, 600, "DarkRoom Engine");
        window.Init();

        Renderer::Shader shader;
        shader.Load("assets/shader/basic.vert", "assets/shader/basic.frag");

        auto cube = Renderer::GeometryFactory::Create("CUBE");
        if (!cube)
            throw std::runtime_error("Failed to create cube geometry");
        cube->Create();

        Core::Mat4 projection = glm::perspective(glm::radians(45.0f),
                                                 800.0f / 600.0f, 0.1f, 100.0f);
        Core::Mat4 view = glm::lookAt(Core::Vec3(0, 5, 10), Core::Vec3(0, 0, 0),
                                      Core::Vec3(0, 1, 0));

        while (!window.ShouldClose())
        {
            if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                window.SetWindowShouldClose();
            }

            shader.Use();
            shader.SetMat4("projection", projection);
            shader.SetMat4("view", view);
            shader.SetMat4("model", Core::Mat4(1.0f));
            shader.SetVec3("objectColor", Core::Vec3(0.5f, 0.5f, 0.5f));
            shader.SetVec3("lightPos", Core::Vec3(0, 0, 5));

            glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            cube->Draw();

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