/**
 * @file test_car_renderbatch_simple.cpp
 * @brief 简化版测试 - 先用立方体验证 RenderBatch 功能
 */

#include "Core/Window.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp"
#include "Core/Logger.hpp"
#include "Renderer/Resources/Shader.hpp"
#include "Renderer/Renderer/InstancedRenderer.hpp"
#include "Renderer/Data/InstanceData.hpp"
#include "Renderer/Factory/MeshDataFactory.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>

// 摄像机参数
glm::vec3 cameraPos = glm::vec3(0.0f, 10.0f, 30.0f);
float cameraSpeed = 10.0f;

int main()
{
    // 初始化日志系统（同步模式）
    Core::LogRotationConfig rotationConfig;
    rotationConfig.type = Core::RotationType::SIZE;
    rotationConfig.maxFileSize = 5 * 1024 * 1024;
    rotationConfig.maxFiles = 3;

    Core::Logger::GetInstance().Initialize("logs/test_car_renderbatch_simple.log", true, Core::LogLevel::INFO,
                                           false, rotationConfig);

    try
    {
        std::cout << "=== Simple RenderBatch Test ===" << std::endl;
        Core::Logger::GetInstance().Info("=== Simple RenderBatch Test with Cubes ===");

        // 创建窗口
        std::cout << "Creating window..." << std::endl;
        Core::Window window(1920, 1080, "Simple RenderBatch Test");
        window.Init();

        // 初始化输入控制器
        Core::MouseController mouseController;
        mouseController.Initialize(glfwGetCurrentContext());

        Core::KeyboardController keyboardController;
        keyboardController.Initialize(glfwGetCurrentContext());

        // 注册键盘控制
        keyboardController.RegisterKeyCallback(GLFW_KEY_ESCAPE, []()
                                               {
                                                   std::cout << "Exit requested" << std::endl;
                                                   exit(0); });
        keyboardController.RegisterKeyCallback(GLFW_KEY_TAB, [&mouseController]()
                                               { mouseController.ToggleMouseCapture(); });

        // 加载着色器
        std::cout << "Loading shader..." << std::endl;
        Renderer::Shader instancedShader;
        instancedShader.Load("assets/shader/instanced.vert", "assets/shader/instanced.frag");

        // ==========================================
        // 创建多个立方体实例（圆形排列）
        // ==========================================
        std::cout << "Creating cube instances..." << std::endl;
        auto cubeInstances = std::make_shared<Renderer::InstanceData>();
        int cubeCount = 12;
        float radius = 10.0f;

        for (int i = 0; i < cubeCount; ++i)
        {
            float angle = (static_cast<float>(i) / cubeCount) * 3.14159f * 2.0f;
            float x = std::cos(angle) * radius;
            float z = std::sin(angle) * radius;
            float y = 0.0f;

            glm::vec3 position(x, y, z);
            glm::vec3 rotation(0.0f, -angle * 57.2958f + 90.0f, 0.0f);
            glm::vec3 scale(1.0f, 1.0f, 1.0f);

            // 彩虹色
            glm::vec3 color(
                static_cast<float>(i) / cubeCount,
                1.0f - static_cast<float>(i) / cubeCount,
                0.5f);

            cubeInstances->Add(position, rotation, scale, color);
        }

        std::cout << "Created " << cubeCount << " cube instances" << std::endl;

        // ==========================================
        // 创建多个立方体渲染器（模拟多材质）
        // ==========================================
        std::cout << "Creating cube renderers..." << std::endl;
        std::vector<Renderer::InstancedRenderer> cubeRenderers;

        // 创建3个渲染器，每个使用不同的材质颜色
        for (int i = 0; i < 3; ++i)
        {
            Renderer::MeshBuffer cubeBuffer = Renderer::MeshBufferFactory::CreateCubeBuffer();
            auto cubeBufferPtr = std::make_shared<Renderer::MeshBuffer>(std::move(cubeBuffer));

            Renderer::InstancedRenderer renderer;
            renderer.SetMesh(cubeBufferPtr);
            renderer.SetInstances(cubeInstances);

            // 设置不同的材质颜色
            glm::vec3 materialColor = (i == 0) ? glm::vec3(1.0f, 0.0f, 0.0f) : (i == 1) ? glm::vec3(0.0f, 1.0f, 0.0f)
                                                                                        : glm::vec3(0.0f, 0.0f, 1.0f);
            renderer.SetMaterialColor(materialColor);

            renderer.Initialize();
            cubeRenderers.push_back(std::move(renderer));
        }

        std::cout << "Created " << cubeRenderers.size() << " cube renderers" << std::endl;

        // OpenGL 设置
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);

        // 光照参数
        glm::vec3 lightPos = glm::vec3(10.0f, 20.0f, 10.0f);

        std::cout << "Starting render loop..." << std::endl;

        float lastTime = static_cast<float>(glfwGetTime());
        double fps_lastTime = glfwGetTime();
        int fps_frameCount = 0;
        int totalFrameCount = 0;

        while (!window.ShouldClose())
        {
            // FPS 计算
            double fps_currentTime = glfwGetTime();
            fps_frameCount++;
            totalFrameCount++;

            if (fps_currentTime - fps_lastTime >= 5.0)
            {
                double fps = fps_frameCount / (fps_currentTime - fps_lastTime);
                std::cout << "FPS: " << static_cast<int>(fps) << std::endl;
                fps_frameCount = 0;
                fps_lastTime = fps_currentTime;
            }

            float currentTime = static_cast<float>(glfwGetTime());
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            keyboardController.Update(deltaTime);

            // 摄像机控制
            float moveSpeed = cameraSpeed * deltaTime;
            glm::vec3 moveDirection(0.0f);

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

            // 计算视图和投影矩阵
            float aspectRatio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
            glm::mat4 projection = glm::perspective(glm::radians(mouseController.GetFOV()),
                                                    aspectRatio, 0.1f, 200.0f);
            glm::mat4 view = glm::lookAt(cameraPos,
                                         cameraPos + mouseController.GetCameraFront(),
                                         glm::vec3(0.0f, 1.0f, 0.0f));

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 设置着色器参数
            instancedShader.Use();
            instancedShader.SetMat4("projection", projection);
            instancedShader.SetMat4("view", view);
            instancedShader.SetVec3("lightPos", lightPos);
            instancedShader.SetVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
            instancedShader.SetVec3("viewPos", cameraPos);
            instancedShader.SetFloat("ambientStrength", 0.3f);
            instancedShader.SetFloat("specularStrength", 0.5f);
            instancedShader.SetFloat("shininess", 32.0f);

            // ==========================================
            // 使用 RenderBatch 批量渲染
            // ==========================================
            instancedShader.SetBool("useTexture", false);
            instancedShader.SetBool("useInstanceColor", true);

            Renderer::InstancedRenderer::RenderBatch(cubeRenderers);

            window.SwapBuffers();
            window.PollEvents();
        }

        std::cout << "Render loop ended. Total frames: " << totalFrameCount << std::endl;
        Core::Logger::GetInstance().Info("Program terminated successfully");
    }
    catch (const std::exception &e)
    {
        std::cout << "Fatal error: " << e.what() << std::endl;
        Core::Logger::GetInstance().Error("Fatal error occurred: " + std::string(e.what()));
        Core::Logger::GetInstance().Shutdown();
        return -1;
    }

    Core::Logger::GetInstance().Shutdown();
    return 0;
}
