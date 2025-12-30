#include "Core/Window.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Cube.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <atomic>

// 摄像机参数
Core::Vec3 cameraPos = Core::Vec3(0.0f, 0.0f, 3.0f);
float cameraSpeed = 5.0f;

// ✅ 使用std::atomic保证线程安全
std::atomic<float> grainSize{0.5f};
std::atomic<float> grainIntensity{0.4f};
std::atomic<float> blurAmount{0.2f};
std::atomic<float> threshold{0.5f};
std::atomic<float> pixelBlockSize{4.0f}; // ✅ 默认4像素块

int main()
{
    try
    {
        Core::Window window(800, 600, "1-bit Grain Engine - Realtime Control");
        window.Init();

        Core::MouseController mouseController;
        mouseController.Initialize(glfwGetCurrentContext());

        Core::KeyboardController keyboardController;
        keyboardController.Initialize(glfwGetCurrentContext());

        // ✅ FIXED: 移除捕获，lambda内直接访问全局变量
        keyboardController.RegisterKeyCallback(GLFW_KEY_ESCAPE, []()
                                               {
                                                   exit(0); // 直接退出（简化处理）
                                               });

        // ✅ FIXED: 使用.load()和.store()操作atomic变量
        keyboardController.RegisterKeyCallback(GLFW_KEY_U, []()
                                               {
    pixelBlockSize.store(glm::min(50.0f, pixelBlockSize.load() + 1.0f));
    std::cout << "[Params] pixelBlockSize: " << pixelBlockSize.load() << " px" << std::endl; });
        keyboardController.RegisterKeyCallback(GLFW_KEY_J, []()
                                               {
    pixelBlockSize.store(glm::max(1.0f, pixelBlockSize.load() - 1.0f));
    std::cout << "[Params] pixelBlockSize: " << pixelBlockSize.load() << " px" << std::endl; });

        keyboardController.RegisterKeyCallback(GLFW_KEY_I, []()
                                               {
    grainIntensity.store(glm::min(1.0f, grainIntensity.load() + 0.05f));
    std::cout << "[Params] grainIntensity: " << grainIntensity.load() << std::endl; });
        keyboardController.RegisterKeyCallback(GLFW_KEY_K, []()
                                               {
    grainIntensity.store(glm::max(0.0f, grainIntensity.load() - 0.05f));
    std::cout << "[Params] grainIntensity: " << grainIntensity.load() << std::endl; });

        keyboardController.RegisterKeyCallback(GLFW_KEY_T, []()
                                               {
    threshold.store(glm::min(0.9f, threshold.load() + 0.05f));
    std::cout << "[Params] threshold: " << threshold.load() << std::endl; });
        keyboardController.RegisterKeyCallback(GLFW_KEY_G, []()
                                               {
    threshold.store(glm::max(0.1f, threshold.load() - 0.05f));
    std::cout << "[Params] threshold: " << threshold.load() << std::endl; });

        keyboardController.RegisterKeyCallback(GLFW_KEY_SPACE, []()
                                               { std::cout << "[Action] Space pressed - Jump!" << std::endl; }, true, 0.5f);

        keyboardController.RegisterKeyCallback(GLFW_KEY_R, []()
                                               { std::cout << "[Action] R pressed - Reload" << std::endl; });

        Renderer::Shader shader;
        shader.Load("assets/shader/basic.vert", "assets/shader/basic.frag");

        // 创建立方体
        std::vector<Renderer::Cube> cubes;
        const int cubeCount = 10;
        for (int i = 0; i < cubeCount; ++i)
        {
            Renderer::Cube cube;
            cube.Create();
            cube.SetPosition(glm::vec3(i * 2.0f - cubeCount, 0.0f, 0.0f));
            cube.SetColor(glm::vec3(0.8f + i * 0.02f, 0.6f + i * 0.04f, 0.4f + i * 0.06f));
            cubes.push_back(std::move(cube));
        }

        glEnable(GL_DEPTH_TEST);

        float lastTime = static_cast<float>(glfwGetTime());
        double fps_lastTime = glfwGetTime();
        int fps_frameCount = 0;

        // 初始参数输出
        std::cout << "\n========== Initial Shader Parameters ==========\n";
        std::cout << "grainSize:      " << grainSize.load() << "\n";
        std::cout << "grainIntensity: " << grainIntensity.load() << "\n";
        std::cout << "blurAmount:     " << blurAmount.load() << "\n";
        std::cout << "threshold:      " << threshold.load() << "\n";
        std::cout << "===============================================\n\n";
        std::cout << "[Controls]\n";
        std::cout << "U/J: grainSize      I/K: grainIntensity\n";
        std::cout << "O/L: blurAmount     T/G: threshold\n";
        std::cout << "ESC: Exit\n\n";

        while (!window.ShouldClose())
        {
            double fps_currentTime = glfwGetTime();
            fps_frameCount++;

            if (fps_currentTime - fps_lastTime >= 0.5)
            {
                double fps = fps_frameCount / (fps_currentTime - fps_lastTime);

                std::cout << "\r[FPS: " << std::fixed << std::setprecision(1) << fps
                          << "] [Size: " << std::setprecision(3) << grainSize.load()
                          << "] [Inten: " << std::setprecision(2) << grainIntensity.load()
                          << "] [Blur: " << blurAmount.load()
                          << "] [Thresh: " << threshold.load() << "]   ";
                std::cout.flush();

                fps_frameCount = 0;
                fps_lastTime = fps_currentTime;
            }

            float currentTime = static_cast<float>(glfwGetTime());
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            keyboardController.Update(deltaTime);

            float moveSpeed = cameraSpeed * deltaTime;
            Core::Vec3 moveDirection(0.0f);
            if (keyboardController.IsKeyPressed(GLFW_KEY_W))
                moveDirection += mouseController.GetCameraFront();
            if (keyboardController.IsKeyPressed(GLFW_KEY_S))
                moveDirection -= mouseController.GetCameraFront();
            if (keyboardController.IsKeyPressed(GLFW_KEY_A))
                moveDirection -= glm::normalize(glm::cross(mouseController.GetCameraFront(), Core::Vec3(0.0f, 1.0f, 0.0f)));
            if (keyboardController.IsKeyPressed(GLFW_KEY_D))
                moveDirection += glm::normalize(glm::cross(mouseController.GetCameraFront(), Core::Vec3(0.0f, 1.0f, 0.0f)));
            if (glm::length(moveDirection) > 0.0f)
            {
                moveDirection = glm::normalize(moveDirection);
                cameraPos += moveDirection * moveSpeed;
            }

            Core::Mat4 projection = glm::perspective(glm::radians(mouseController.GetFOV()),
                                                     800.0f / 600.0f, 0.1f, 100.0f);
            Core::Mat4 view = glm::lookAt(cameraPos,
                                          cameraPos + mouseController.GetCameraFront(),
                                          Core::Vec3(0.0f, 1.0f, 0.0f));

            // 渲染
            shader.Use();
            shader.SetMat4("projection", projection);
            shader.SetMat4("view", view);

            shader.SetFloat("grainSize", grainSize.load());
            shader.SetFloat("grainIntensity", grainIntensity.load());
            shader.SetFloat("blurAmount", blurAmount.load());
            shader.SetFloat("threshold", threshold.load());

            shader.SetVec3("lightPos", Core::Vec3(5.0f, 5.0f, 5.0f));
            shader.SetVec3("objectColor", Core::Vec3(1.0f, 1.0f, 1.0f));

            shader.SetFloat("pixelBlockSize", pixelBlockSize.load()); // ✅ 新增

            glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            for (const auto &cube : cubes)
            {
                shader.SetMat4("model", cube.GetModelMatrix());
                cube.Draw();
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