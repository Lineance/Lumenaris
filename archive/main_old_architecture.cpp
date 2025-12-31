#include "Core/Window.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp"
#include "Core/Logger.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/InstancedRenderer.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <filesystem>

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

    Core::Logger::GetInstance().Initialize("logs/instanced_rendering.log", true, Core::LogLevel::DEBUG,
                                           true, rotationConfig);

    try
    {
        Core::Logger::GetInstance().Info("Starting Instanced Rendering Application");
        Core::Logger::GetInstance().Info("Application version: Cube & OBJ Instanced Rendering with Textures");
        Core::Logger::GetInstance().Info("Window resolution: 1920x1080");

        Core::Logger::GetInstance().Info("Creating application window...");
        Core::Window window(1920, 1080, "Instanced Rendering - Cube & OBJ with Textures");
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

        // ==========================================
        // 1. 创建立方体实例化网格
        // ==========================================
        Core::Logger::GetInstance().Info("Creating instanced cubes from Cube template...");
        Renderer::InstancedRenderer instancedCubes = Renderer::InstancedRenderer::CreateFromCube(0);

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

                instancedCubes.AddInstance(position, rotation, scale, color);
            }
        }

        instancedCubes.Create();
        Core::Logger::GetInstance().Info("Instanced cubes created: " + std::to_string(instancedCubes.GetInstanceCount()) + " instances");

        // ==========================================
        // 2. 创建 OBJ 模型实例化渲染（支持多材质和纹理）
        // ==========================================
        std::string carPath = "assets/models/cars/sportsCar.obj";
        std::vector<Renderer::InstancedRenderer> instancedCarMeshes;

        if (fs::exists(carPath))
        {
            Core::Logger::GetInstance().Info("Creating instanced cars from OBJ: " + carPath);

            // 创建多个实例化网格（每个材质一个）
            instancedCarMeshes = Renderer::InstancedRenderer::CreateFromOBJ(carPath, 0);

            if (!instancedCarMeshes.empty())
            {
                // 为每个材质网格添加相同的实例
                int carCount = 12;
                float radius = 15.0f;

                for (auto& mesh : instancedCarMeshes)
                {
                    for (int i = 0; i < carCount; ++i)
                    {
                        float angle = (static_cast<float>(i) / carCount) * 3.14159f * 2.0f;
                        float x = std::cos(angle) * radius;
                        float z = std::sin(angle) * radius;
                        float y = 0.0f;

                        glm::vec3 position(x, y, z);
                        glm::vec3 rotation(0.0f, -angle * 57.2958f + 90.0f, 0.0f);
                        glm::vec3 scale(0.5f, 0.5f, 0.5f);

                        // 使用白色，让纹理显示真实颜色
                        glm::vec3 color(1.0f, 1.0f, 1.0f);

                        mesh.AddInstance(position, rotation, scale, color);
                    }

                    mesh.Create();
                }

                Core::Logger::GetInstance().Info("Instanced car meshes created: " + std::to_string(instancedCarMeshes.size()) +
                                                 " materials, " + std::to_string(carCount) + " instances each");
            }
        }
        else
        {
            Core::Logger::GetInstance().Warning("Car OBJ file not found: " + carPath);
        }

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

        // Initial parameters
        Core::Logger::GetInstance().Info("Controls: WASD=Move, Q/E=Up/Down, Mouse=Look Around");
        Core::Logger::GetInstance().Info("TAB=Toggle Mouse Capture, ESC=Exit");
        Core::Logger::GetInstance().Info("Scene: " + std::to_string(instancedCubes.GetInstanceCount()) + " cubes, " +
                                         std::to_string(instancedCarMeshes.empty() ? 0 : instancedCarMeshes[0].GetInstanceCount()) + " cars (multi-material)");
        Core::Logger::GetInstance().Info("Performance: Each material group uses only 1 draw call!");
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
            size_t totalDrawCalls = (instancedCubes.GetInstanceCount() > 0 ? 1 : 0) +
                                   instancedCarMeshes.size();
            Core::LogContext renderContext;
            renderContext.renderPass = "Instanced";
            renderContext.batchIndex = 0;
            renderContext.drawCallCount = static_cast<int>(totalDrawCalls);
            renderContext.currentShader = "Instanced with Textures";
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

            // 渲染立方体地面（无纹理）
            if (instancedCubes.GetInstanceCount() > 0)
            {
                instancedShader.SetBool("useTexture", false);
                instancedShader.SetBool("useInstanceColor", true); // 立方体使用实例颜色
                instancedCubes.Draw();
            }

            // 渲染车模型（每个材质一个实例化网格，支持纹理）
            if (!instancedCarMeshes.empty())
            {
                for (const auto& carMesh : instancedCarMeshes)
                {
                    if (carMesh.GetInstanceCount() > 0)
                    {
                        // 如果此网格有纹理，启用纹理
                        instancedShader.SetBool("useTexture", carMesh.HasTexture());

                        // 设置材质颜色
                        instancedShader.SetVec3("objectColor", carMesh.GetMaterialColor());

                        // 设置材质光泽度
                        instancedShader.SetBool("useInstanceColor", false); // 使用材质颜色而不是实例颜色

                        carMesh.Draw();
                    }
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
