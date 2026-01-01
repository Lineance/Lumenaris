/**
 * 新架构示例程序 - 演示方案C的职责分离设计
 *
 * 架构设计：
 * - IMesh: 网格数据接口（Cube, SimpleMesh）
 * - InstanceData: 实例数据容器（矩阵、颜色）
 * - InstancedRenderer: 渲染逻辑执行者
 *
 * 使用流程：
 * 1. 创建网格（SimpleMesh）
 * 2. 准备实例数据（InstanceData）
 * 3. 创建渲染器并设置（InstancedRenderer）
 * 4. 执行渲染（Render）
 */

#include "Core/Window.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp"
#include "Core/Logger.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/SimpleMesh.hpp"
#include "Renderer/InstancedRenderer.hpp"
#include "Renderer/InstanceData.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>

// 摄像机参数
glm::vec3 cameraPos = glm::vec3(0.0f, 10.0f, 30.0f);
float cameraSpeed = 10.0f;

int main()
{
    // 初始化日志系统
    Core::LogRotationConfig rotationConfig;
    rotationConfig.type = Core::RotationType::SIZE;
    rotationConfig.maxFileSize = 5 * 1024 * 1024; // 5MB
    rotationConfig.maxFiles = 3;

    Core::Logger::GetInstance().Initialize("logs/new_architecture.log", true, Core::LogLevel::DEBUG,
                                           true, rotationConfig);

    try
    {
        Core::Logger::GetInstance().Info("=== New Architecture Example ===");
        Core::Logger::GetInstance().Info("Demonstrating Responsibility Separation Design");
        Core::Logger::GetInstance().Info("Architecture:");
        Core::Logger::GetInstance().Info("  - IMesh: Mesh data (SimpleMesh)");
        Core::Logger::GetInstance().Info("  - InstanceData: Instance data (matrices, colors)");
        Core::Logger::GetInstance().Info("  - InstancedRenderer: Rendering logic");

        Core::Logger::GetInstance().Info("Creating application window...");
        Core::Window window(1920, 1080, "New Architecture - Responsibility Separation");
        window.Init();

        Core::Logger::GetInstance().Info("Initializing input controllers...");
        Core::MouseController mouseController;
        mouseController.Initialize(glfwGetCurrentContext());

        Core::KeyboardController keyboardController;
        keyboardController.Initialize(glfwGetCurrentContext());

        // Keyboard controls
        keyboardController.RegisterKeyCallback(GLFW_KEY_ESCAPE, []()
                                               {
                                                   Core::Logger::GetInstance().Info("Exit requested");
                                                   exit(0);
                                               });
        keyboardController.RegisterKeyCallback(GLFW_KEY_TAB, [&mouseController]()
                                               {
                                                   mouseController.ToggleMouseCapture();
                                               });

        Core::Logger::GetInstance().Info("Loading instanced shader program...");
        Renderer::Shader instancedShader;
        instancedShader.Load("assets/shader/instanced.vert", "assets/shader/instanced.frag");

        // ==========================================
        // 步骤 1: 创建网格（SimpleMesh）
        // ==========================================
        Core::Logger::GetInstance().Info("Step 1: Creating SimpleMesh from Cube template...");
        Renderer::SimpleMesh cubeMesh = Renderer::SimpleMesh::CreateFromCube();
        cubeMesh.Create();

        // ==========================================
        // 步骤 2: 准备实例数据（InstanceData）
        // ==========================================
        Core::Logger::GetInstance().Info("Step 2: Preparing InstanceData...");

        Renderer::InstanceData instances;

        // 创建 10x10 的立方体地面阵列
        int gridSize = 10;
        float spacing = 2.0f;
        float startX = -((gridSize - 1) * spacing) / 2.0f;
        float startZ = -((gridSize - 1) * spacing) / 2.0f;

        for (int x = 0; x < gridSize; ++x)
        {
            for (int z = 0; z < gridSize; ++z)
            {
                glm::vec3 position(startX + x * spacing, -1.0f, startZ + z * spacing);
                glm::vec3 rotation(0.0f, 0.0f, 0.0f);
                glm::vec3 scale(1.5f, 0.5f, 1.5f);

                // 生成方格颜色
                bool isWhite = (x + z) % 2 == 0;
                glm::vec3 color = isWhite ? glm::vec3(0.9f, 0.9f, 0.9f) : glm::vec3(0.3f, 0.3f, 0.3f);

                instances.Add(position, rotation, scale, color);
            }
        }

        Core::Logger::GetInstance().Info("Prepared " + std::to_string(instances.GetCount()) + " instances");

        // ==========================================
        // 步骤 3: 创建渲染器并设置（InstancedRenderer）
        // ==========================================
        Core::Logger::GetInstance().Info("Step 3: Creating and configuring InstancedRenderer...");

        Renderer::InstancedRenderer renderer;
        renderer.SetMesh(cubeMesh);
        renderer.SetInstances(instances);
        renderer.Initialize();

        Core::Logger::GetInstance().Info("InstancedRenderer initialized with " +
                                         std::to_string(renderer.GetInstanceCount()) + " instances");

        // ==========================================
        // 步骤 4: 执行渲染（Render Loop）
        // ==========================================
        Core::Logger::GetInstance().Info("Step 4: Starting render loop...");

        Core::Logger::GetInstance().Info("Enabling OpenGL depth testing");
        glEnable(GL_DEPTH_TEST);

        // 设置背景色
        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);

        // 预设光照参数
        glm::vec3 lightPos = glm::vec3(10.0f, 20.0f, 10.0f);

        float lastTime = static_cast<float>(glfwGetTime());
        double fps_lastTime = glfwGetTime();
        int fps_frameCount = 0;
        int totalFrameCount = 0;

        Core::Logger::GetInstance().Info("Controls: WASD=Move, Q/E=Up/Down, Mouse=Look Around");
        Core::Logger::GetInstance().Info("TAB=Toggle Mouse Capture, ESC=Exit");
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
                moveDirection -= glm::vec3(0.0f, 1.0f, 0.0f);
            if (keyboardController.IsKeyPressed(GLFW_KEY_E))
                moveDirection += glm::vec3(0.0f, 1.0f, 0.0f);

            if (glm::length(moveDirection) > 0.0f)
            {
                moveDirection = glm::normalize(moveDirection);
                cameraPos += moveDirection * moveSpeed;
            }

            // 动态计算窗口宽高比
            float aspectRatio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
            glm::mat4 projection = glm::perspective(glm::radians(mouseController.GetFOV()),
                                                    aspectRatio, 0.1f, 200.0f);
            glm::mat4 view = glm::lookAt(cameraPos,
                                         cameraPos + mouseController.GetCameraFront(),
                                         glm::vec3(0.0f, 1.0f, 0.0f));

            // 设置渲染上下文
            size_t totalDrawCalls = 1;
            Core::LogContext renderContext;
            renderContext.renderPass = "NewArchitecture";
            renderContext.batchIndex = 0;
            renderContext.drawCallCount = static_cast<int>(totalDrawCalls);
            renderContext.currentShader = "Instanced Shader";
            Core::Logger::GetInstance().SetContext(renderContext);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 设置通用着色器参数
            instancedShader.Use();
            instancedShader.SetMat4("projection", projection);
            instancedShader.SetMat4("view", view);
            instancedShader.SetVec3("lightPos", lightPos);
            instancedShader.SetVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
            instancedShader.SetVec3("viewPos", cameraPos);
            instancedShader.SetFloat("ambientStrength", 0.3f);
            instancedShader.SetFloat("specularStrength", 0.5f);
            instancedShader.SetFloat("shininess", 32.0f);
            instancedShader.SetBool("useInstanceColor", true);
            instancedShader.SetBool("useTexture", false);

            // ==========================================
            // 执行渲染！
            // ==========================================
            renderer.Render();

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

    Core::Logger::GetInstance().Shutdown();
    return 0;
}
