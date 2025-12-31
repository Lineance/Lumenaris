/**
 * 实例化渲染测试程序
 *
 * 本程序演示如何使用 InstancedMesh 类进行高效的批量渲染
 *
 * 功能演示：
 * 1. 创建 10x10x2 = 200 个立方体的实例化渲染
 * 2. 每个立方体有独立的位置、旋转、缩放和颜色
 * 3. 单次绘制调用渲染所有实例，性能比逐个绘制提升 10-100 倍
 *
 * 使用方法：
 * - 编译后运行程序
 * - 使用 WASD 移动摄像机
 * - 使用 Q/E 上下移动
 * - 使用鼠标旋转视角
 * - 按 TAB 切换鼠标捕获
 * - 按 ESC 退出
 *
 * 性能对比：
 * - 传统方式：200次绘制调用（每个立方体一次）
 * - 实例化方式：1次绘制调用（所有立方体一次）
 * - CPU-GPU 通信大幅减少
 */

#include "Core/Window.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp"
#include "Core/Logger.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/InstancedMesh.hpp"
#include "Renderer/Cube.hpp"
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>

// 摄像机参数
glm::vec3 cameraPos = glm::vec3(0.0f, 5.0f, 15.0f);
float cameraSpeed = 5.0f;

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
        Core::Logger::GetInstance().Info("Starting Instanced Rendering Test");
        Core::Logger::GetInstance().Info("Window resolution: 1280x720");

        // 创建窗口
        Core::Window window(1280, 720, "Instanced Rendering Test");
        window.Init();

        // 初始化输入控制器
        Core::MouseController mouseController;
        mouseController.Initialize(glfwGetCurrentContext());

        Core::KeyboardController keyboardController;
        keyboardController.Initialize(glfwGetCurrentContext());

        // 注册键盘控制
        keyboardController.RegisterKeyCallback(GLFW_KEY_ESCAPE, []()
        {
            Core::Logger::GetInstance().Info("Exit requested");
            exit(0);
        });
        keyboardController.RegisterKeyCallback(GLFW_KEY_TAB, [&mouseController]()
        {
            mouseController.ToggleMouseCapture();
        });

        // 加载实例化着色器
        Renderer::Shader instancedShader;
        instancedShader.Load("assets/shader/instanced.vert", "assets/shader/instanced.frag");

        // 创建实例化网格
        Core::Logger::GetInstance().Info("Creating instanced cubes...");
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

        // 创建 10x10x2 的立方体阵列
        int gridSize = 10;
        float spacing = 1.5f;
        float startX = -((gridSize - 1) * spacing) / 2.0f;
        float startZ = -((gridSize - 1) * spacing) / 2.0f;

        for (int x = 0; x < gridSize; ++x)
        {
            for (int z = 0; z < gridSize; ++z)
            {
                for (int y = 0; y < 2; ++y)
                {
                    glm::vec3 position(
                        startX + x * spacing,
                        y * spacing,
                        startZ + z * spacing
                    );

                    // 每个立方体有轻微旋转
                    glm::vec3 rotation(
                        x * 5.0f,
                        y * 10.0f,
                        z * 5.0f
                    );

                    glm::vec3 scale(0.8f, 0.8f, 0.8f);

                    // 生成彩虹渐变颜色
                    float t = static_cast<float>(x) / (gridSize - 1);
                    float t2 = static_cast<float>(z) / (gridSize - 1);
                    glm::vec3 color(
                        0.5f + 0.5f * std::sin(t * 3.14159f),
                        0.5f + 0.5f * std::sin(t2 * 3.14159f),
                        0.5f + 0.5f * std::cos(t * 3.14159f)
                    );

                    instancedCubes.AddInstance(position, rotation, scale, color);
                }
            }
        }

        instancedCubes.Create();
        Core::Logger::GetInstance().Info("Created " + std::to_string(instancedCubes.GetInstanceCount()) + " instances");

        // 启用深度测试
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

        // 光照参数
        glm::vec3 lightPos = glm::vec3(5.0f, 10.0f, 5.0f);

        // 渲染循环
        float lastTime = static_cast<float>(glfwGetTime());
        double fps_lastTime = glfwGetTime();
        int fps_frameCount = 0;
        int totalFrameCount = 0;

        Core::Logger::GetInstance().Info("Starting render loop...");
        Core::Logger::GetInstance().Info("Controls: WASD=Move, Q/E=Up/Down, Mouse=Look, TAB=Toggle Capture, ESC=Exit");

        while (!window.ShouldClose())
        {
            // FPS 计算
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

            // 更新输入
            keyboardController.Update(deltaTime);

            // 摄像机移动
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
                                                    aspectRatio, 0.1f, 100.0f);
            glm::mat4 view = glm::lookAt(cameraPos,
                                         cameraPos + mouseController.GetCameraFront(),
                                         glm::vec3(0.0f, 1.0f, 0.0f));

            // 渲染
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            instancedShader.Use();
            instancedShader.SetMat4("projection", projection);
            instancedShader.SetMat4("view", view);
            instancedShader.SetVec3("lightPos", lightPos);
            instancedShader.SetVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
            instancedShader.SetVec3("viewPos", cameraPos);
            instancedShader.SetFloat("ambientStrength", 0.2f);
            instancedShader.SetFloat("specularStrength", 0.5f);
            instancedShader.SetFloat("shininess", 32.0f);
            instancedShader.SetBool("useInstanceColor", true);

            instancedCubes.Draw();

            window.SwapBuffers();
            window.PollEvents();
        }

        Core::Logger::GetInstance().Info("Render loop ended");
        Core::Logger::GetInstance().Info("Total frames rendered: " + std::to_string(totalFrameCount));
    }
    catch (const std::exception& e)
    {
        Core::Logger::GetInstance().Error("Fatal error: " + std::string(e.what()));
        Core::Logger::GetInstance().Shutdown();
        return -1;
    }

    Core::Logger::GetInstance().Shutdown();
    return 0;
}
