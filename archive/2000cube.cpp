#include "Core/Window.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp"
#include "Core/Logger.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/InstancedMesh.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>

// 摄像机参数
glm::vec3 cameraPos = glm::vec3(0.0f, 10.0f, 25.0f);
float cameraSpeed = 10.0f;

int main()
{
    // 初始化日志系统（启用异步写入和按大小轮转）
    Core::LogRotationConfig rotationConfig;
    rotationConfig.type = Core::RotationType::SIZE;
    rotationConfig.maxFileSize = 5 * 1024 * 1024; // 5MB
    rotationConfig.maxFiles = 3;

    Core::Logger::GetInstance().Initialize("logs/instanced_rendering.log", true, Core::LogLevel::DEBUG,
                                           true, rotationConfig);

    try
    {
        Core::Logger::GetInstance().Info("Starting Instanced Rendering Test Application");
        Core::Logger::GetInstance().Info("Application version: Instanced Rendering Demo");
        Core::Logger::GetInstance().Info("Window resolution: 1920x1080");

        Core::Logger::GetInstance().Info("Creating application window...");
        Core::Window window(1920, 1080, "Instanced Rendering Test - 20x20x5 Cubes");
        window.Init();

        Core::Logger::GetInstance().Info("Initializing input controllers...");
        Core::MouseController mouseController;
        mouseController.Initialize(glfwGetCurrentContext());

        Core::KeyboardController keyboardController;
        keyboardController.Initialize(glfwGetCurrentContext());

        // Keyboard controls
        Core::Logger::GetInstance().Info("Registering keyboard controls...");
        keyboardController.RegisterKeyCallback(GLFW_KEY_ESCAPE, []()
                                               {
                                                   Core::Logger::GetInstance().Info("Application exit requested by user (ESC key)");
                                                   exit(0);
                                               });
        keyboardController.RegisterKeyCallback(GLFW_KEY_TAB, [&mouseController]()
                                               {
                                                   mouseController.ToggleMouseCapture();
                                               });

        Core::Logger::GetInstance().Info("Loading instanced shader program...");
        Renderer::Shader instancedShader;
        instancedShader.Load("assets/shader/instanced.vert", "assets/shader/instanced.frag");

        // 创建实例化网格
        Core::Logger::GetInstance().Info("Creating instanced mesh...");
        Renderer::InstancedMesh instancedCubes;

        // 准备立方体顶点数据
        std::vector<float> cubeVertices;
        cubeVertices.insert(cubeVertices.end(), {
            // 前面
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

            // 后面
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,

            // 左面
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

            // 右面
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

            // 下面
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

            // 上面
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f
        });

        instancedCubes.SetVertices(cubeVertices.data(), cubeVertices.size());
        instancedCubes.SetVertexLayout(8, {0, 3, 6}, {3, 3, 2});

        // 创建 20x20x5 的立方体阵列 (2000个实例)
        int gridSizeX = 20;
        int gridSizeZ = 20;
        int gridSizeY = 5;
        float spacing = 1.5f;
        float startX = -((gridSizeX - 1) * spacing) / 2.0f;
        float startZ = -((gridSizeZ - 1) * spacing) / 2.0f;

        Core::Logger::GetInstance().Info("Generating " + std::to_string(gridSizeX * gridSizeZ * gridSizeY) + " instances...");

        for (int x = 0; x < gridSizeX; ++x)
        {
            for (int z = 0; z < gridSizeZ; ++z)
            {
                for (int y = 0; y < gridSizeY; ++y)
                {
                    glm::vec3 position(
                        startX + x * spacing,
                        y * spacing,
                        startZ + z * spacing
                    );

                    // 每个立方体有独特的旋转
                    glm::vec3 rotation(
                        x * 3.0f,
                        y * 5.0f + (x % 3) * 15.0f,
                        z * 3.0f
                    );

                    glm::vec3 scale(0.7f, 0.7f, 0.7f);

                    // 生成彩虹渐变颜色
                    float t = static_cast<float>(x) / (gridSizeX - 1);
                    float t2 = static_cast<float>(z) / (gridSizeZ - 1);
                    float t3 = static_cast<float>(y) / (gridSizeY - 1);
                    glm::vec3 color(
                        0.3f + 0.7f * std::abs(std::sin(t * 3.14159f * 2.0f)),
                        0.3f + 0.7f * std::abs(std::sin(t2 * 3.14159f * 2.0f)),
                        0.3f + 0.7f * std::abs(std::cos(t3 * 3.14159f * 2.0f))
                    );

                    instancedCubes.AddInstance(position, rotation, scale, color);
                }
            }
        }

        instancedCubes.Create();
        Core::Logger::GetInstance().Info("Instanced mesh created with " +
                                         std::to_string(instancedCubes.GetInstanceCount()) + " instances");
        Core::Logger::GetInstance().Info("Performance: Single draw call for all instances!");

        Core::Logger::GetInstance().Info("Enabling OpenGL depth testing");
        glEnable(GL_DEPTH_TEST);

        // 设置背景色 - 深色背景以突出彩色立方体
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);

        // 预设光照参数
        glm::vec3 lightPos = glm::vec3(10.0f, 20.0f, 10.0f);

        float lastTime = static_cast<float>(glfwGetTime());
        double fps_lastTime = glfwGetTime();
        int fps_frameCount = 0;
        int totalFrameCount = 0;

        // Initial parameters
        Core::Logger::GetInstance().Info("Controls: WASD=Move, Q/E=Up/Down, Mouse=Look Around");
        Core::Logger::GetInstance().Info("TAB=Toggle Mouse Capture, ESC=Exit");
        Core::Logger::GetInstance().Info("Instances: " + std::to_string(instancedCubes.GetInstanceCount()) + " cubes");
        Core::Logger::GetInstance().Info("Starting render loop...");

        while (!window.ShouldClose())
        {
            double fps_currentTime = glfwGetTime();
            fps_frameCount++;
            totalFrameCount++;

            if (fps_currentTime - fps_lastTime >= 0.5)
            {
                double fps = fps_frameCount / (fps_currentTime - fps_lastTime);
                Core::Logger::GetInstance().SetFPS(static_cast<int>(fps));

                // 每0.5秒更新一次统计摘要
                Core::Logger::GetInstance().LogStatisticsSummary();

                fps_frameCount = 0;
                fps_lastTime = fps_currentTime;
            }

            float currentTime = static_cast<float>(glfwGetTime());
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            keyboardController.Update(deltaTime);

            float moveSpeed = cameraSpeed * deltaTime;
            glm::vec3 moveDirection(0.0f);

            // 摄像机控制
            if (keyboardController.IsKeyPressed(GLFW_KEY_W))
                moveDirection += mouseController.GetCameraFront();
            if (keyboardController.IsKeyPressed(GLFW_KEY_S))
                moveDirection -= mouseController.GetCameraFront();
            if (keyboardController.IsKeyPressed(GLFW_KEY_A))
                moveDirection -= glm::normalize(glm::cross(mouseController.GetCameraFront(), glm::vec3(0.0f, 1.0f, 0.0f)));
            if (keyboardController.IsKeyPressed(GLFW_KEY_D))
                moveDirection += glm::normalize(glm::cross(mouseController.GetCameraFront(), glm::vec3(0.0f, 1.0f, 0.0f)));
            if (keyboardController.IsKeyPressed(GLFW_KEY_Q))
                moveDirection -= glm::vec3(0.0f, 1.0f, 0.0f); // 向下飞行
            if (keyboardController.IsKeyPressed(GLFW_KEY_E))
                moveDirection += glm::vec3(0.0f, 1.0f, 0.0f); // 向上飞行

            if (glm::length(moveDirection) > 0.0f)
            {
                moveDirection = glm::normalize(moveDirection);
                cameraPos += moveDirection * moveSpeed;
            }

            // 动态计算窗口宽高比以适应窗口大小变化
            float aspectRatio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
            glm::mat4 projection = glm::perspective(glm::radians(mouseController.GetFOV()),
                                                    aspectRatio, 0.1f, 200.0f);
            glm::mat4 view = glm::lookAt(cameraPos,
                                         cameraPos + mouseController.GetCameraFront(),
                                         glm::vec3(0.0f, 1.0f, 0.0f));

            // 设置渲染上下文
            Core::LogContext renderContext;
            renderContext.renderPass = "Instanced";
            renderContext.batchIndex = 0;
            renderContext.drawCallCount = 1; // 实例化渲染只有1次绘制调用！
            renderContext.currentShader = "Instanced";
            Core::Logger::GetInstance().SetContext(renderContext);

            // 渲染实例化网格
            instancedShader.Use();
            instancedShader.SetMat4("projection", projection);
            instancedShader.SetMat4("view", view);
            instancedShader.SetVec3("lightPos", lightPos);
            instancedShader.SetVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
            instancedShader.SetVec3("viewPos", cameraPos);
            instancedShader.SetFloat("ambientStrength", 0.3f);
            instancedShader.SetFloat("specularStrength", 0.6f);
            instancedShader.SetFloat("shininess", 64.0f);
            instancedShader.SetBool("useInstanceColor", true);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 单次绘制调用渲染所有实例！
            instancedCubes.Draw();

            window.SwapBuffers();
            window.PollEvents();
        }

        Core::Logger::GetInstance().Info("Render loop ended, cleaning up resources...");
        Core::Logger::GetInstance().Info("Final statistics - Total frames rendered: " + std::to_string(totalFrameCount));
        Core::Logger::GetInstance().Info("Program terminated successfully");
    }
    catch (const std::exception &e)
    {
        Core::Logger::GetInstance().Error("Fatal error occurred: " + std::string(e.what()));
        Core::Logger::GetInstance().Error("Application will terminate with error code -1");
        Core::Logger::GetInstance().Shutdown();
        return -1;
    }

    Core::Logger::GetInstance().Info("Shutting down logger system...");
    Core::Logger::GetInstance().Shutdown();
    return 0;
}
