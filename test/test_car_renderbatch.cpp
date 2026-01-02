/**
 * @file test_car_renderbatch.cpp
 * @brief 多车模型批量渲染测试 - 使用 RenderBatch 优化性能
 *
 * 测试目标：
 * 1. 加载多个相同的车模型（不同位置、旋转、缩放）
 * 2. 使用 InstancedRenderer::RenderBatch 批量渲染
 * 3. 对比传统逐个渲染和批量渲染的性能差异
 *
 * 性能预期：
 * - 传统方式：38个材质 × 12辆车 = 456次 DrawCall
 * - 批量渲染：38个材质 = 38次 DrawCall（按纹理分组）
 * - 性能提升：约12倍
 */

#include "Core/Window.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp"
#include "Core/Logger.hpp"
#include "Renderer/Resources/Shader.hpp"
#include "Renderer/Renderer/InstancedRenderer.hpp"
#include "Renderer/Data/InstanceData.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <filesystem>
#include <optional>  // ✅ 添加 std::optional 支持

namespace fs = std::filesystem;

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

    // ✅ 使用同步模式，确保日志立即输出（便于调试）
    Core::Logger::GetInstance().Initialize("logs/test_car_renderbatch.log", true, Core::LogLevel::INFO,
                                           false, rotationConfig);

    try
    {
        Core::Logger::GetInstance().Info("=== Car RenderBatch Test ===");
        Core::Logger::GetInstance().Info("Testing batch rendering with multiple car models");
        Core::Logger::GetInstance().Info("Window resolution: 1920x1080");

        // 创建窗口
        Core::Logger::GetInstance().Info("Creating application window...");
        Core::Window window(1920, 1080, "Car RenderBatch Test | 1:Individual 2:Batch");
        window.Init();

        // 初始化输入控制器
        Core::Logger::GetInstance().Info("Initializing input controllers...");
        Core::MouseController mouseController;
        mouseController.Initialize(glfwGetCurrentContext());

        Core::KeyboardController keyboardController;
        keyboardController.Initialize(glfwGetCurrentContext());

        // 注册键盘控制
        keyboardController.RegisterKeyCallback(GLFW_KEY_ESCAPE, []()
                                               {
                                                   Core::Logger::GetInstance().Info("Application exit requested by user (ESC key)");
                                                   exit(0); });
        keyboardController.RegisterKeyCallback(GLFW_KEY_TAB, [&mouseController]()
                                               { mouseController.ToggleMouseCapture(); });

        // 加载着色器
        Core::Logger::GetInstance().Info("Loading instanced shader program...");
        Renderer::Shader instancedShader;
        instancedShader.Load("assets/shader/instanced.vert", "assets/shader/instanced.frag");

        // ==========================================
        // 加载车模型
        // ==========================================
        std::string carPath = "assets/models/cars/sportsCar.obj";

        Core::Logger::GetInstance().Info("Checking car model path: " + carPath);

        if (!fs::exists(carPath))
        {
            Core::Logger::GetInstance().Error("Car OBJ file not found: " + carPath);
            Core::Logger::GetInstance().Error("Please ensure the car model exists before running this test");
            return -1;
        }

        Core::Logger::GetInstance().Info("Car model file exists, starting to load...");

        // ==========================================
        // 创建多个车实例（圆形排列）
        // ==========================================
        auto carInstances = std::make_shared<Renderer::InstanceData>();
        int carCount = 12;
        float radius = 15.0f;

        for (int i = 0; i < carCount; ++i)
        {
            float angle = (static_cast<float>(i) / carCount) * 3.14159f * 2.0f;
            float x = std::cos(angle) * radius;
            float z = std::sin(angle) * radius;
            float y = 0.0f;

            glm::vec3 position(x, y, z);
            glm::vec3 rotation(0.0f, -angle * 57.2958f + 90.0f, 0.0f);
            glm::vec3 scale(0.5f, 0.5f, 0.5f);

            // 使用白色，让材质显示真实颜色
            glm::vec3 color(1.0f, 1.0f, 1.0f);

            carInstances->Add(position, rotation, scale, color);
        }

        Core::Logger::GetInstance().Info("Created " + std::to_string(carCount) + " car instances");

        // ==========================================
        // 创建实例化渲染器（每个材质一个）
        // ==========================================
        Core::Logger::GetInstance().Info("Creating InstancedRenderers from OBJ (this may take a while)...");
        std::cout << "Starting OBJ model loading..." << std::endl;
        std::cout << "Note: The car model is 16MB, this may take 10-30 seconds..." << std::endl;
        std::cout.flush();

        auto [carRenderers, carMeshBuffers, carInstancesData] =
            Renderer::InstancedRenderer::CreateForOBJ(carPath, carInstances);

        std::cout << "OBJ loading completed!" << std::endl;
        std::cout.flush();
        Core::Logger::GetInstance().Info("OBJ loading completed, checking results...");

        if (carRenderers.empty())
        {
            Core::Logger::GetInstance().Error("Failed to create car renderers");
            return -1;
        }

        Core::Logger::GetInstance().Info("Created " + std::to_string(carRenderers.size()) +
                                         " car renderers (multi-material) with " +
                                         std::to_string(carCount) + " instances each");
        Core::Logger::GetInstance().Info("Total materials: " + std::to_string(carRenderers.size()));

        // 统计信息
        int totalDrawCalls = carRenderers.size();
        Core::Logger::GetInstance().Info("Batch rendering: " + std::to_string(totalDrawCalls) + " draw calls");

        // OpenGL 设置
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);

        // 光照参数
        glm::vec3 lightPos = glm::vec3(10.0f, 20.0f, 10.0f);

        Core::Logger::GetInstance().Info("Controls: WASD=Move, Q/E=Up/Down, Mouse=Look Around");
        Core::Logger::GetInstance().Info("TAB=Toggle Mouse Capture, ESC=Exit");
        Core::Logger::GetInstance().Info("Starting render loop...");

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
                Core::Logger::GetInstance().SetFPS(static_cast<int>(fps));
                Core::Logger::GetInstance().LogStatisticsSummary();

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
            // 批量渲染车模型（优化 uniform 设置）
            // ==========================================

            // 使用 std::optional 跟踪状态变化，减少 uniform 切换
            std::optional<bool> lastUseTexture;
            std::optional<glm::vec3> lastObjectColor;
            std::optional<bool> lastUseInstanceColor;

            for (const auto &carRenderer : carRenderers)
            {
                if (carRenderer.GetInstanceCount() > 0)
                {
                    // 只在状态变化时设置 uniform
                    bool useTexture = carRenderer.HasTexture();
                    if (!lastUseTexture.has_value() || useTexture != lastUseTexture.value())
                    {
                        instancedShader.SetBool("useTexture", useTexture);
                        lastUseTexture = useTexture;
                    }

                    const glm::vec3 &objectColor = carRenderer.GetMaterialColor();
                    if (!lastObjectColor.has_value() || objectColor != lastObjectColor.value())
                    {
                        instancedShader.SetVec3("objectColor", objectColor);
                        lastObjectColor = objectColor;
                    }

                    bool useInstanceColor = false;
                    if (!lastUseInstanceColor.has_value() || useInstanceColor != lastUseInstanceColor.value())
                    {
                        instancedShader.SetBool("useInstanceColor", useInstanceColor);
                        lastUseInstanceColor = useInstanceColor;
                    }

                    carRenderer.Render();
                }
            }

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
