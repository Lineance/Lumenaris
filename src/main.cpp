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
    // Initialize random seed for consistent cloud rotations
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    try
    {
        Core::Window window(800, 600, "Cloud Gallery - OBJ Model Loader");
        window.Init();

        Core::MouseController mouseController;
        mouseController.Initialize(glfwGetCurrentContext());

        Core::KeyboardController keyboardController;
        keyboardController.Initialize(glfwGetCurrentContext());

        // Load bunny.obj model
        std::vector<Renderer::OBJModel> cloudModels;
        std::string bunnyPath = "assets/models/bunny.obj";

        try {
            // Check if bunny.obj file exists
            if (!fs::exists(bunnyPath)) {
                std::cerr << "Bunny model file not found: " << bunnyPath << std::endl;
            } else {
                std::cout << "Loading bunny model from: " << bunnyPath << std::endl;

                std::cout << "Loading bunny.obj..." << std::endl;

                cloudModels.emplace_back(bunnyPath);
                if (cloudModels.back().LoadFromFile(bunnyPath)) {
                    cloudModels.back().Create();
                    std::cout << "✓ Bunny loaded successfully - Vertices: "
                              << cloudModels.back().GetVertexCount()
                              << ", Materials: " << cloudModels.back().GetMaterialCount()
                              << std::endl;
                } else {
                    std::cerr << "✗ Failed to load bunny.obj" << std::endl;
                }

                std::cout << "Total models loaded: " << cloudModels.size() << std::endl;

                // Position bunny model
                if (!cloudModels.empty()) {
                    // Position bunny at origin with appropriate scale and rotation
                    cloudModels[0].SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
                    cloudModels[0].SetScale(2.0f); // Make bunny larger to see details
                    cloudModels[0].SetColor(glm::vec3(0.8f, 0.6f, 0.4f)); // Warm brown color
                    // Rotate bunny to face the camera
                    cloudModels[0].SetRotation(glm::vec3(0.0f, 180.0f, 0.0f));
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error loading cloud models: " << e.what() << std::endl;
        }


        // Keyboard controls
        keyboardController.RegisterKeyCallback(GLFW_KEY_ESCAPE, []()
                                               { exit(0); });

        Renderer::Shader shader;
        // 测试不同的着色器
        shader.Load("assets/shader/basic.vert", "assets/shader/basic.frag"); // 卡通渲染



        glEnable(GL_DEPTH_TEST);

        float lastTime = static_cast<float>(glfwGetTime());
        double fps_lastTime = glfwGetTime();
        int fps_frameCount = 0;

        // Initial parameters
        std::cout << "[Controls] WASD: Move Q/E: Up/Down ESC: Exit\n";
        std::cout << "[Models] Loaded Stanford Bunny\n";

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

            glm::mat4 projection = glm::perspective(glm::radians(mouseController.GetFOV()),
                                                     800.0f / 600.0f, 0.1f, 100.0f);
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
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


            // Render all cloud models
            for (const auto& cloudModel : cloudModels) {
                shader.SetMat4("model", cloudModel.GetModelMatrix());
                shader.SetVec3("objectColor", cloudModel.GetColor());
                cloudModel.Draw();
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