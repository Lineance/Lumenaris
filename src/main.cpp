#include "Core/Window.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp" // 新增包含
#include "Renderer/Shader.hpp"
#include "Renderer/Geometry.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

// 摄像机位置和移动参数
Core::Vec3 cameraPos = Core::Vec3(0.0f, 0.0f, 3.0f);
float cameraSpeed = 5.0f; // 单位：米/秒

int main()
{
    try
    {
        Core::Window window(800, 600, "DarkRoom Engine - Input Control");
        window.Init();

        // 初始化鼠标控制器
        Core::MouseController mouseController;
        mouseController.Initialize(glfwGetCurrentContext());

        // 初始化键盘控制器
        Core::KeyboardController keyboardController;
        keyboardController.Initialize(glfwGetCurrentContext());

        // 注册键盘回调函数
        keyboardController.RegisterKeyCallback(GLFW_KEY_ESCAPE, [&window]()
                                               {
            std::cout << "ESC键按下，退出程序" << std::endl;
            window.SetWindowShouldClose(); });

        keyboardController.RegisterKeyCallback(GLFW_KEY_SPACE, []()
                                               {
                                                   std::cout << "空格键按下，执行跳跃动作" << std::endl;
                                                   // 这里可以添加跳跃逻辑
                                               },
                                               true, 0.5f); // 允许重复，每0.5秒触发一次

        keyboardController.RegisterKeyCallback(GLFW_KEY_R, []()
                                               {
                                                   std::cout << "R键按下，重新加载场景" << std::endl;
                                                   // 这里可以添加重新加载逻辑
                                               });

        Renderer::Shader shader;
        shader.Load("assets/shader/basic.vert", "assets/shader/basic.frag");

        auto cube = Renderer::GeometryFactory::Create("CUBE");
        if (!cube)
            throw std::runtime_error("Failed to create cube geometry");
        cube->Create();

        glEnable(GL_DEPTH_TEST);

        // 计时变量
        float lastTime = static_cast<float>(glfwGetTime());

        double fps_lastTime = glfwGetTime();
        int fps_frameCount = 0;
        double fps_fpsUpdateInterval = 0.5; // 每0.5秒更新一次FPS显示

        while (!window.ShouldClose())
        {

            double fps_currentTime = glfwGetTime();
            fps_frameCount++;

            // 如果距离上次更新已超过设定的时间间隔
            if (fps_currentTime - fps_lastTime >= fps_fpsUpdateInterval)
            {
                // 计算FPS
                double fps = fps_frameCount / (fps_currentTime - fps_lastTime);

                // 方法1: 在控制台输出
                std::cout << "FPS: " << fps << std::endl;

                // 重置计数器和时间
                fps_frameCount = 0;
                fps_lastTime = fps_currentTime;
            }

            // 计算时间增量
            float currentTime = static_cast<float>(glfwGetTime());
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            // 更新键盘状态
            keyboardController.Update(deltaTime);

            // 摄像机移动控制（基于键盘状态查询）
            float moveSpeed = cameraSpeed * deltaTime;
            Core::Vec3 moveDirection(0.0f);

            if (keyboardController.IsKeyPressed(GLFW_KEY_W))
                moveDirection += mouseController.GetCameraFront();
            if (keyboardController.IsKeyPressed(GLFW_KEY_S))
                moveDirection -= mouseController.GetCameraFront();
            if (keyboardController.IsKeyPressed(GLFW_KEY_A))
                moveDirection -= glm::normalize(glm::cross(mouseController.GetCameraFront(),
                                                           Core::Vec3(0.0f, 1.0f, 0.0f)));
            if (keyboardController.IsKeyPressed(GLFW_KEY_D))
                moveDirection += glm::normalize(glm::cross(mouseController.GetCameraFront(),
                                                           Core::Vec3(0.0f, 1.0f, 0.0f)));
            if (keyboardController.IsKeyPressed(GLFW_KEY_Q))
                moveDirection -= Core::Vec3(0.0f, 1.0f, 0.0f); // 下降
            if (keyboardController.IsKeyPressed(GLFW_KEY_E))
                moveDirection += Core::Vec3(0.0f, 1.0f, 0.0f); // 上升

            // 标准化移动方向并应用速度
            if (glm::length(moveDirection) > 0.0f)
            {
                moveDirection = glm::normalize(moveDirection);
                cameraPos += moveDirection * moveSpeed;
            }

            // 更新投影和视图矩阵
            Core::Mat4 projection = glm::perspective(glm::radians(mouseController.GetFOV()),
                                                     800.0f / 600.0f, 0.1f, 100.0f);
            Core::Mat4 view = glm::lookAt(cameraPos,
                                          cameraPos + mouseController.GetCameraFront(),
                                          Core::Vec3(0.0f, 1.0f, 0.0f));

            shader.Use();
            shader.SetMat4("projection", projection);
            shader.SetMat4("view", view);
            shader.SetMat4("model", Core::Mat4(1.0f));
            shader.SetVec3("objectColor", Core::Vec3(0.5f, 0.5f, 0.5f));
            shader.SetVec3("lightPos", Core::Vec3(0, 0, 5));

            glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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