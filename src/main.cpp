#include "Core/Window.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Cube.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

// 摄像机位置和移动参数
Core::Vec3 cameraPos = Core::Vec3(0.0f, 0.0f, 3.0f);
float cameraSpeed = 5.0f; // 单位：米/秒

int main()
{
    try
    {
        Core::Window window(800, 600, "DarkRoom Engine - Multiple Cubes");
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
                                               { std::cout << "空格键按下，执行跳跃动作" << std::endl; }, true, 0.5f); // 允许重复，每0.5秒触发一次

        keyboardController.RegisterKeyCallback(GLFW_KEY_R, []()
                                               { std::cout << "R键按下，重新加载场景" << std::endl; });

        Renderer::Shader shader;
        shader.Load("assets/shader/basic.vert", "assets/shader/basic.frag");

        // =================================================================
        // 创建多个立方体（必须手动调用Create()，与原始代码一致）
        std::vector<Renderer::Cube> cubes;
        const int cubeCount = 10;

        for (int i = 0; i < cubeCount; ++i)
        {
            Renderer::Cube cube;
            cube.Create(); // ✅ 手动调用，与原始代码行为一致

            // 设置位置：沿X轴一字排开，间距2个单位，中心对称分布
            cube.SetPosition(glm::vec3(i * 2.0f - cubeCount, 0.0f, 0.0f));

            // 设置颜色：随索引变化产生渐变效果
            cube.SetColor(glm::vec3(
                0.5f + i * 0.05f, // R分量递增
                0.3f + i * 0.07f, // G分量递增
                0.8f - i * 0.03f  // B分量递减
                ));

            cubes.push_back(std::move(cube));
        }
        // =================================================================

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

                // 在控制台输出
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

            // =================================================================
            // 渲染所有立方体
            shader.Use();
            shader.SetMat4("projection", projection);
            shader.SetMat4("view", view);
            shader.SetVec3("viewPos", cameraPos); // 片段着色器需要摄像机位置
            shader.SetVec3("lightPos", Core::Vec3(0.0f, 5.0f, 5.0f));

            glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 遍历绘制每个立方体
            for (const auto &cube : cubes)
            {
                shader.SetMat4("model", cube.GetModelMatrix());
                shader.SetVec3("objectColor", cube.GetColor());

                cube.Draw();
            }
            // =================================================================

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