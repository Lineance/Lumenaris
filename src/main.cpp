#include "Core/Window.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp"
#include "Core/Logger.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/OBJModel.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <ctime>

namespace fs = std::filesystem;

// 摄像机参数
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
float cameraSpeed = 5.0f;

int main()
{
    // 初始化日志系统（启用异步写入和按大小轮转）
    Core::LogRotationConfig rotationConfig;
    rotationConfig.type = Core::RotationType::SIZE;
    rotationConfig.maxFileSize = 5 * 1024 * 1024; // 5MB
    rotationConfig.maxFiles = 3;

    Core::Logger::GetInstance().Initialize("logs/car_gallery.log", true, Core::LogLevel::DEBUG,
                                           true, rotationConfig);

    try
    {
        Core::Logger::GetInstance().Info("Starting 3D Car Gallery application");
        Core::Logger::GetInstance().Info("Application version: OBJ Model Viewer");
        Core::Logger::GetInstance().Info("Window resolution: 1920x1080");

        Core::Logger::GetInstance().Info("Creating application window...");
        Core::Window window(1920, 1080, "3D Car Gallery - OBJ Model Viewer");
        window.Init();

        Core::Logger::GetInstance().Info("Initializing input controllers...");
        Core::MouseController mouseController;
        mouseController.Initialize(glfwGetCurrentContext());

        Core::KeyboardController keyboardController;
        keyboardController.Initialize(glfwGetCurrentContext());

        // Load sportsCar.obj model
        std::vector<Renderer::OBJModel> cloudModels;
        std::string carPath = "assets/models/cars/sportsCar.obj";

        try
        {
            // Check if sportsCar.obj file exists
            if (!fs::exists(carPath))
            {
                Core::Logger::GetInstance().Error("Sports car model file not found: " + carPath);
            }
            else
            {
                Core::Logger::GetInstance().Info("Loading sports car model from: " + carPath);

                cloudModels.emplace_back(carPath);
                if (cloudModels.back().LoadFromFile(carPath))
                {
                    cloudModels.back().Create();
                    Core::Logger::GetInstance().Info("Sports car loaded successfully - Vertices: " +
                                                     std::to_string(cloudModels.back().GetVertexCount()) +
                                                     ", Materials: " + std::to_string(cloudModels.back().GetMaterialCount()));
                }
                else
                {
                    Core::Logger::GetInstance().Error("Failed to load sportsCar.obj");
                }

                Core::Logger::GetInstance().Info("Total models loaded: " + std::to_string(cloudModels.size()));

                // Position sports car model
                if (!cloudModels.empty())
                {
                    // Position sports car at origin with appropriate scale and rotation
                    cloudModels[0].SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
                    cloudModels[0].SetScale(0.5f);                        // Scale down sports car to fit view
                    cloudModels[0].SetColor(glm::vec3(0.8f, 0.8f, 0.8f)); // Default white color (will be overridden by materials)
                    // Rotate sports car to face the camera
                    cloudModels[0].SetRotation(glm::vec3(0.0f, 90.0f, 0.0f));
                }
            }
        }
        catch (const std::exception &e)
        {
            Core::Logger::GetInstance().Error("Error loading cloud models: " + std::string(e.what()));
        }

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

        Core::Logger::GetInstance().Info("Loading shader program...");
        Renderer::Shader shader;

        shader.Load("assets/shader/basic.vert", "assets/shader/basic.frag");

        Core::Logger::GetInstance().Info("Enabling OpenGL depth testing");
        glEnable(GL_DEPTH_TEST); // 关键启用OpenGL渲染深度参数

        // 设置背景色
        glClearColor(0.5f, 0.7f, 1.0f, 1.0f); // 淡蓝色天空

        // 预设光照参数
        glm::vec3 lightPos = glm::vec3(5.0f, 5.0f, 5.0f);

        float lastTime = static_cast<float>(glfwGetTime());
        double fps_lastTime = glfwGetTime();
        int fps_frameCount = 0;
        int totalFrameCount = 0;

        // Initial parameters
        Core::Logger::GetInstance().Info("Controls: WASD: Move Q/E: Up/Down TAB: Toggle Mouse Capture ESC: Exit");
        Core::Logger::GetInstance().Info("Models: Loaded Sports Car");
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

            // TODO 这里的操作逻辑到底是什么
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
                                                    aspectRatio, 0.1f, 100.0f);
            glm::mat4 view = glm::lookAt(cameraPos,
                                         cameraPos + mouseController.GetCameraFront(),
                                         glm::vec3(0.0f, 1.0f, 0.0f));

            // 设置渲染上下文
            Core::LogContext renderContext;
            renderContext.renderPass = "Forward";
            renderContext.batchIndex = 0;
            renderContext.drawCallCount = cloudModels.size();
            renderContext.currentShader = "Phong";
            Core::Logger::GetInstance().SetContext(renderContext);

            // 渲染
            shader.Use();
            shader.SetMat4("projection", projection);
            shader.SetMat4("view", view);
            shader.SetVec3("lightPos", lightPos);
            shader.SetVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
            shader.SetVec3("viewPos", cameraPos);
            shader.SetFloat("shininess", 32.0f);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Render all cloud models
            for (auto &cloudModel : cloudModels)
            {
                shader.SetMat4("model", cloudModel.GetModelMatrix());

                const auto& materials = cloudModel.GetMaterials();
                if (!materials.empty())
                {
                    // 对每个材质单独渲染
                    for (size_t matIdx = 0; matIdx < materials.size(); ++matIdx)
                    {
                        const auto& material = materials[matIdx];

                        // 使用材质的漫反射颜色
                        shader.SetVec3("objectColor", material.diffuse);

                        // 设置材质的光泽度
                        shader.SetFloat("shininess", material.shininess > 0.0f ? material.shininess : 32.0f);

                        // 检查是否有纹理
                        cloudModel.SetCurrentMaterialIndex(static_cast<int>(matIdx));
                        if (cloudModel.HasTexture())
                        {
                            shader.SetBool("useTexture", true);
                        }
                        else
                        {
                            shader.SetBool("useTexture", false);
                        }

                        // 渲染使用此材质的面
                        cloudModel.DrawWithMaterial(static_cast<int>(matIdx));
                    }
                }
                else
                {
                    // 如果没有材质信息，使用默认颜色
                    shader.SetVec3("objectColor", cloudModel.GetColor());
                    shader.SetBool("useTexture", false);
                    shader.SetFloat("shininess", 32.0f);
                    cloudModel.Draw();
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

    Core::Logger::GetInstance().Info("Shutting down logger system...");
    Core::Logger::GetInstance().Shutdown();
    return 0;
}