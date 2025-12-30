#include "Core/Window.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Cube.hpp"
#include "Renderer/Sphere.hpp"
#include "Renderer/Geometry.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <atomic>

// 摄像机参数
Core::Vec3 cameraPos = Core::Vec3(0.0f, 0.0f, 3.0f);
float cameraSpeed = 5.0f;


int main()
{
    try
    {
        Core::Window window(800, 600, "1-bit Grain Engine - Realtime Control");
        window.Init();

        Core::MouseController mouseController;
        mouseController.Initialize(glfwGetCurrentContext());

        Core::KeyboardController keyboardController;
        keyboardController.Initialize(glfwGetCurrentContext());

        // Register geometries
        Renderer::GeometryFactory::Register("cube", []()
                                            { return std::make_unique<Renderer::Cube>(); });
        Renderer::GeometryFactory::Register("sphere", []()
                                            { return std::make_unique<Renderer::Sphere>(1.0f, 20, 20); });

        // Keyboard controls
        keyboardController.RegisterKeyCallback(GLFW_KEY_ESCAPE, []()
                                               { exit(0); });


        Renderer::Shader shader;
        shader.Load("assets/shader/basic.vert", "assets/shader/basic.frag");

        // 创建球体
        std::vector<Renderer::Sphere> spheres;
        const int sphereCount = 10;
        for (int i = 0; i < sphereCount; ++i)
        {
            Renderer::Sphere sphere(0.8f + i * 0.1f, 16, 16); // 半径递增，不同分辨率
            sphere.Create();
            sphere.SetPosition(glm::vec3(i * 10.0f - sphereCount, 0.0f, 0.0f));
            sphere.SetColor(glm::vec3(0.8f + i * 0.02f, 0.6f + i * 0.04f, 0.4f + i * 0.06f));
            spheres.push_back(std::move(sphere));
        }

        // 创建光源可视化立方体
        Renderer::Cube lightCube;
        lightCube.Create();
        lightCube.SetColor(glm::vec3(1.0f, 1.0f, 0.8f)); // 暖白色光颜色
        lightCube.SetScale(0.3f); // 小立方体

        glEnable(GL_DEPTH_TEST);

        float lastTime = static_cast<float>(glfwGetTime());
        double fps_lastTime = glfwGetTime();
        int fps_frameCount = 0;

        // Initial parameters
        std::cout << "[Controls] WASD: Move Q/E: Up/Down ESC: Exit\n";

        while (!window.ShouldClose())
        {
            double fps_currentTime = glfwGetTime();
            fps_frameCount++;

            if (fps_currentTime - fps_lastTime >= 0.5)
            {
                double fps = fps_frameCount / (fps_currentTime - fps_lastTime);

                std::cout << "\r[FPS: " << std::fixed << std::setprecision(1) << fps << "]   ";
                std::cout.flush();

                fps_frameCount = 0;
                fps_lastTime = fps_currentTime;
            }

            float currentTime = static_cast<float>(glfwGetTime());
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            keyboardController.Update(deltaTime);

            float moveSpeed = cameraSpeed * deltaTime;
            Core::Vec3 moveDirection(0.0f);
            if (keyboardController.IsKeyPressed(GLFW_KEY_W))
                moveDirection += mouseController.GetCameraFront();
            if (keyboardController.IsKeyPressed(GLFW_KEY_S))
                moveDirection -= mouseController.GetCameraFront();
            if (keyboardController.IsKeyPressed(GLFW_KEY_A))
                moveDirection -= glm::normalize(glm::cross(mouseController.GetCameraFront(), Core::Vec3(0.0f, 1.0f, 0.0f)));
            if (keyboardController.IsKeyPressed(GLFW_KEY_D))
                moveDirection += glm::normalize(glm::cross(mouseController.GetCameraFront(), Core::Vec3(0.0f, 1.0f, 0.0f)));
            if (keyboardController.IsKeyPressed(GLFW_KEY_Q))
                moveDirection -= Core::Vec3(0.0f, 1.0f, 0.0f); // 向下飞行
            if (keyboardController.IsKeyPressed(GLFW_KEY_E))
                moveDirection += Core::Vec3(0.0f, 1.0f, 0.0f); // 向上飞行
            if (glm::length(moveDirection) > 0.0f)
            {
                moveDirection = glm::normalize(moveDirection);
                cameraPos += moveDirection * moveSpeed;
            }

            Core::Mat4 projection = glm::perspective(glm::radians(mouseController.GetFOV()),
                                                     800.0f / 600.0f, 0.1f, 100.0f);
            Core::Mat4 view = glm::lookAt(cameraPos,
                                          cameraPos + mouseController.GetCameraFront(),
                                          Core::Vec3(0.0f, 1.0f, 0.0f));

            // 渲染
            shader.Use();
            shader.SetMat4("projection", projection);
            shader.SetMat4("view", view);

            // Light position and visualization
            Core::Vec3 lightPos = Core::Vec3(5.0f, 5.0f, 5.0f);
            shader.SetVec3("lightPos", lightPos);

            // Update light cube position
            lightCube.SetPosition(lightPos);

            shader.SetVec3("lightColor", Core::Vec3(1.0f, 1.0f, 1.0f));
            shader.SetVec3("viewPos", cameraPos);
            shader.SetFloat("shininess", 32.0f);


            glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Render spheres
            for (const auto &sphere : spheres)
            {
                shader.SetMat4("model", sphere.GetModelMatrix());
                shader.SetVec3("objectColor", sphere.GetColor());
                sphere.Draw();
            }

            // Render light cube
            shader.SetMat4("model", lightCube.GetModelMatrix());
            shader.SetVec3("objectColor", lightCube.GetColor());
            lightCube.Draw();

            window.SwapBuffers();
            window.PollEvents();
        }

        std::cout << "\n\n[Exit] Program terminated successfully." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}