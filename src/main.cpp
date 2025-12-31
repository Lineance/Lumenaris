#include "Core/Window.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp"
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

    try
    {
        Core::Window window(1920, 1080, "3D Car Gallery - OBJ Model Viewer");
        window.Init();

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
                std::cerr << "Sports car model file not found: " << carPath << std::endl;
            }
            else
            {
                std::cout << "Loading sports car model from: " << carPath << std::endl;

                std::cout << "Loading sportsCar.obj..." << std::endl;

                cloudModels.emplace_back(carPath);
                if (cloudModels.back().LoadFromFile(carPath))
                {
                    cloudModels.back().Create();
                    std::cout << "✓ Sports car loaded successfully - Vertices: "
                              << cloudModels.back().GetVertexCount()
                              << ", Materials: " << cloudModels.back().GetMaterialCount()
                              << std::endl;
                }
                else
                {
                    std::cerr << "✗ Failed to load sportsCar.obj" << std::endl;
                }

                std::cout << "Total models loaded: " << cloudModels.size() << std::endl;

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
            std::cerr << "Error loading cloud models: " << e.what() << std::endl;
        }

        // Keyboard controls
        keyboardController.RegisterKeyCallback(GLFW_KEY_ESCAPE, []()
                                               { exit(0); });
        keyboardController.RegisterKeyCallback(GLFW_KEY_TAB, [&mouseController]()
                                               { mouseController.ToggleMouseCapture(); });

        Renderer::Shader shader;

        shader.Load("assets/shader/basic.vert", "assets/shader/basic.frag"); 

        glEnable(GL_DEPTH_TEST); // 关键启用OpenGL渲染深度参数

        float lastTime = static_cast<float>(glfwGetTime());
        double fps_lastTime = glfwGetTime();
        int fps_frameCount = 0;

        // Initial parameters
        std::cout << "[Controls] WASD: Move Q/E: Up/Down TAB: Toggle Mouse Capture ESC: Exit\n";
        std::cout << "[Models] Loaded Sports Car\n";

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

            // 渲染
            shader.Use();
            shader.SetMat4("projection", projection);
            shader.SetMat4("view", view);

            // Light settings
            glm::vec3 lightPos = glm::vec3(5.0f, 5.0f, 5.0f);
            shader.SetVec3("lightPos", lightPos);
            shader.SetVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
            shader.SetVec3("viewPos", cameraPos);
            shader.SetFloat("shininess", 32.0f);

            // 设置蓝天背景色 (天空蓝色渐变)
            glClearColor(0.5f, 0.7f, 1.0f, 1.0f); // 淡蓝色天空

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // TODO 关键深度渲染代码，但是顺序需要讨论

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

        std::cout << "\n\n[Exit] Program terminated successfully." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}