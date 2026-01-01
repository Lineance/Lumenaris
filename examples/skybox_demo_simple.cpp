/**
 * ========================================
 * Skybox Demo - Simple Version (No HDR)
 * ========================================
 *
 * 功能演示：
 * - 天空盒渲染（使用6个普通纹理文件）
 * - 与现有Phong光照系统兼容
 * - 反射效果（立方体反射天空盒）
 * - 背景旋转
 *
 * 纹理要求：
 * - 6个JPG/PNG纹理文件（推荐1024x1024或2048x2048）
 * - 命名：right.jpg, left.jpg, top.jpg, bottom.jpg, back.jpg, front.jpg
 *
 * 控制说明：
 * - WASD: 前后左右移动
 * - Q/E: 上下移动
 * - 鼠标: 旋转视角
 * - TAB: 切换鼠标捕获
 * - 1/2: 减少/增加背景旋转速度
 * - ESC: 退出
 *
 * ========================================
 */

#include "Core/Window.hpp"
#include "Core/Camera.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp"
#include "Core/Logger.hpp"
#include "Renderer/Environment/Skybox.hpp"
#include "Renderer/Lighting/LightManager.hpp"
#include "Renderer/Lighting/Light.hpp"
#include "Renderer/Resources/Shader.hpp"
#include "Renderer/Data/MeshBuffer.hpp"
#include "Renderer/Factory/MeshDataFactory.hpp"
#include "Renderer/Renderer/InstancedRenderer.hpp"
#include "Renderer/Data/InstanceData.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>

// 全局配置
const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const char* WINDOW_TITLE = "Skybox Demo (No HDR) - Press 1/2 to rotate";

/**
 * 创建反射立方体阵列
 */
std::shared_ptr<Renderer::InstanceData> CreateReflectiveCubes()
{
    Core::Logger::GetInstance().Info("Creating reflective cubes...");

    auto instances = std::make_shared<Renderer::InstanceData>();

    // 创建5x5的立方体阵列
    int gridSize = 5;
    float spacing = 3.0f;

    for (int x = 0; x < gridSize; ++x)
    {
        for (int z = 0; z < gridSize; ++z)
        {
            glm::vec3 position(
                (x - gridSize/2) * spacing,
                0.0f,
                (z - gridSize/2) * spacing
            );

            glm::vec3 rotation(0.0f, 0.0f, 0.0f);
            glm::vec3 scale(1.5f);

            // 不同颜色
            glm::vec3 color(
                (float)x / (gridSize - 1),
                0.5f,
                (float)z / (gridSize - 1)
            );

            instances->Add(position, rotation, scale, color);
        }
    }

    Core::Logger::GetInstance().Info("Reflective cubes created: " +
                                     std::to_string(instances->GetCount()) + " cubes");
    return instances;
}

/**
 * 创建地板
 */
std::shared_ptr<Renderer::InstanceData> CreateFloor()
{
    auto instances = std::make_shared<Renderer::InstanceData>();

    instances->Add(
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(-90.0f, 0.0f, 0.0f),
        glm::vec3(30.0f, 30.0f, 1.0f),
        glm::vec3(0.7f, 0.7f, 0.7f)
    );

    return instances;
}

/**
 * 设置光照
 */
void SetupLighting()
{
    auto& lightManager = Renderer::Lighting::LightManager::GetInstance();

    // 太阳光（平行光）
    auto sun = std::make_shared<Renderer::Lighting::DirectionalLight>(
        glm::vec3(-0.2f, -1.0f, -0.3f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        1.0f,
        0.2f, 0.8f, 0.5f
    );
    lightManager.AddDirectionalLight(sun);

    Core::Logger::GetInstance().Info("Lighting setup complete");
}

int main()
{
    // ========================================
    // 初始化日志系统
    // ========================================
    Core::LogRotationConfig rotationConfig;
    rotationConfig.type = Core::RotationType::SIZE;
    rotationConfig.maxFileSize = 5 * 1024 * 1024;
    rotationConfig.maxFiles = 3;

    Core::Logger::GetInstance().Initialize(
        "logs/skybox_demo.log",
        true,
        Core::LogLevel::INFO,
        true,
        rotationConfig
    );

    Core::Logger::GetInstance().Info("========================================");
    Core::Logger::GetInstance().Info("Skybox Demo - Starting...");
    Core::Logger::GetInstance().Info("========================================");

    try
    {
        // ========================================
        // 创建窗口
        // ========================================
        Core::Window window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
        window.Init();

        // ========================================
        // 初始化输入控制器和摄像机
        // ========================================
        Core::Camera camera(
            glm::vec3(0.0f, 5.0f, 15.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            -90.0f,
            -20.0f
        );

        Core::MouseController mouseController;
        mouseController.Initialize(glfwGetCurrentContext());
        mouseController.SetMouseCapture(true);

        glfwSetCursorPosCallback(glfwGetCurrentContext(), [](GLFWwindow* window, double xpos, double ypos) {
            static bool firstMouse = true;
            static float lastX = WINDOW_WIDTH / 2.0f;
            static float lastY = WINDOW_HEIGHT / 2.0f;

            int mouseCaptured = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
            if (!mouseCaptured)
                return;

            if (firstMouse)
            {
                lastX = static_cast<float>(xpos);
                lastY = static_cast<float>(ypos);
                firstMouse = false;
            }

            float xoffset = static_cast<float>(xpos) - lastX;
            float yoffset = lastY - static_cast<float>(ypos);

            lastX = static_cast<float>(xpos);
            lastY = static_cast<float>(ypos);

            Core::Camera* cam = static_cast<Core::Camera*>(glfwGetWindowUserPointer(window));
            if (cam)
            {
                cam->ProcessMouseMovement(xoffset, yoffset);
            }
        });

        glfwSetScrollCallback(glfwGetCurrentContext(), [](GLFWwindow* window, double xoffset, double yoffset) {
            Core::Camera* cam = static_cast<Core::Camera*>(glfwGetWindowUserPointer(window));
            if (cam)
            {
                cam->ProcessMouseScroll(static_cast<float>(yoffset));
            }
        });

        glfwSetWindowUserPointer(glfwGetCurrentContext(), &camera);

        Core::KeyboardController keyboardController;
        keyboardController.Initialize(glfwGetCurrentContext());

        keyboardController.RegisterKeyCallback(GLFW_KEY_ESCAPE, []()
        {
            exit(0);
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_TAB, [&mouseController]()
        {
            mouseController.ToggleMouseCapture();
        });

        // ========================================
        // 设置光照
        // ========================================
        SetupLighting();

        // ========================================
        // 创建Skybox
        // ========================================
        Core::Logger::GetInstance().Info("Creating Skybox...");
        Renderer::Skybox skybox;
        skybox.Initialize();
        skybox.LoadShaders("assets/shader/skybox.vert", "assets/shader/skybox.frag");

        // 尝试加载天空盒纹理
        bool skyboxLoaded = skybox.Load(
            "assets/textures/skybox/right.jpg",
            "assets/textures/skybox/left.jpg",
            "assets/textures/skybox/top.jpg",
            "assets/textures/skybox/bottom.jpg",
            "assets/textures/skybox/back.jpg",
            "assets/textures/skybox/front.jpg"
        );

        if (!skyboxLoaded)
        {
            Core::Logger::GetInstance().Warning("Failed to load skybox textures");
            Core::Logger::GetInstance().Info("Continuing without skybox...");
        }
        else
        {
            Core::Logger::GetInstance().Info("Skybox loaded successfully!");
        }

        // ========================================
        // 加载着色器
        // ========================================
        Core::Logger::GetInstance().Info("Loading shaders...");
        Renderer::Shader phongShader;
        phongShader.Load("assets/shader/multi_light.vert", "assets/shader/multi_light.frag");

        // ========================================
        // 创建场景几何体
        // ========================================
        auto cubeInstances = CreateReflectiveCubes();
        auto floorInstances = CreateFloor();

        Renderer::MeshBuffer cubeMesh = Renderer::MeshBufferFactory::CreateCubeBuffer();
        auto cubeMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(cubeMesh));

        Renderer::MeshBuffer floorMesh = Renderer::MeshBufferFactory::CreatePlaneBuffer(1.0f, 1.0f, 1, 1);
        auto floorMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(floorMesh));

        auto cubeRenderer = std::make_unique<Renderer::InstancedRenderer>();
        cubeRenderer->SetMesh(cubeMeshPtr);
        cubeRenderer->SetInstances(cubeInstances);
        cubeRenderer->Initialize();

        auto floorRenderer = std::make_unique<Renderer::InstancedRenderer>();
        floorRenderer->SetMesh(floorMeshPtr);
        floorRenderer->SetInstances(floorInstances);
        floorRenderer->Initialize();

        // 背景旋转控制
        float backgroundRotationSpeed = 0.0f;

        keyboardController.RegisterKeyCallback(GLFW_KEY_1, [&backgroundRotationSpeed]()
        {
            backgroundRotationSpeed -= 5.0f;
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_2, [&backgroundRotationSpeed]()
        {
            backgroundRotationSpeed += 5.0f;
        });

        // ========================================
        // OpenGL 设置
        // ========================================
        glEnable(GL_DEPTH_TEST);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        Core::Logger::GetInstance().Info("========================================");
        Core::Logger::GetInstance().Info("Skybox Demo loaded successfully!");
        Core::Logger::GetInstance().Info("Controls:");
        Core::Logger::GetInstance().Info("  WASD - Move camera");
        Core::Logger::GetInstance().Info("  Q/E  - Move up/down");
        Core::Logger::GetInstance().Info("  Mouse - Look around");
        Core::Logger::GetInstance().Info("  TAB  - Toggle mouse capture");
        Core::Logger::GetInstance().Info("  1/2  - Decrease/Increase background rotation");
        Core::Logger::GetInstance().Info("  ESC  - Exit");
        Core::Logger::GetInstance().Info("========================================");

        // ========================================
        // 渲染循环
        // ========================================
        double lastTime = glfwGetTime();
        double fpsLastTime = glfwGetTime();
        int fpsFrameCount = 0;
        int totalFrameCount = 0;
        float backgroundRotation = 0.0f;

        while (!window.ShouldClose())
        {
            // FPS 计算
            double currentTime = glfwGetTime();
            fpsFrameCount++;
            totalFrameCount++;

            if (currentTime - fpsLastTime >= 0.5)
            {
                float fps = fpsFrameCount / (currentTime - fpsLastTime);
                Core::Logger::GetInstance().SetFPS(static_cast<int>(fps));

                static int logCounter = 0;
                if (++logCounter >= 2)
                {
                    std::string logMessage = "Skybox Demo | FPS: " +
                                             std::to_string(static_cast<int>(fps)) +
                                             " | Rotation: " +
                                             std::to_string(backgroundRotation);
                    Core::Logger::GetInstance().Info(logMessage);
                    logCounter = 0;
                }

                fpsFrameCount = 0;
                fpsLastTime = currentTime;
            }

            float deltaTime = static_cast<float>(currentTime - lastTime);
            lastTime = currentTime;

            // 更新背景旋转
            backgroundRotation += backgroundRotationSpeed * deltaTime;

            // ========================================
            // 输入处理
            // ========================================
            keyboardController.Update(deltaTime);

            if (keyboardController.IsKeyPressed(GLFW_KEY_W))
                camera.ProcessKeyboard(Core::Camera::MovementDirection::FORWARD, deltaTime);
            if (keyboardController.IsKeyPressed(GLFW_KEY_S))
                camera.ProcessKeyboard(Core::Camera::MovementDirection::BACKWARD, deltaTime);
            if (keyboardController.IsKeyPressed(GLFW_KEY_A))
                camera.ProcessKeyboard(Core::Camera::MovementDirection::LEFT, deltaTime);
            if (keyboardController.IsKeyPressed(GLFW_KEY_D))
                camera.ProcessKeyboard(Core::Camera::MovementDirection::RIGHT, deltaTime);
            if (keyboardController.IsKeyPressed(GLFW_KEY_Q))
                camera.ProcessKeyboard(Core::Camera::MovementDirection::DOWN, deltaTime);
            if (keyboardController.IsKeyPressed(GLFW_KEY_E))
                camera.ProcessKeyboard(Core::Camera::MovementDirection::UP, deltaTime);

            // ========================================
            // 渲染设置
            // ========================================
            float aspectRatio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
            glm::mat4 projection = camera.GetProjectionMatrix(aspectRatio, 0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();

            // 清空缓冲区
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // ========================================
            // 渲染天空盒（如果已加载）
            // ========================================
            if (skyboxLoaded)
            {
                skybox.SetRotation(backgroundRotation);
                skybox.Render(projection, view);
            }

            // ========================================
            // 渲染场景物体
            // ========================================
            phongShader.Use();
            phongShader.SetMat4("projection", projection);
            phongShader.SetMat4("view", view);
            phongShader.SetVec3("viewPos", camera.GetPosition());
            phongShader.SetBool("useInstanceColor", true);
            phongShader.SetBool("useTexture", false);
            phongShader.SetFloat("shininess", 64.0f);

            // 应用光照
            Renderer::Lighting::LightManager::GetInstance().ApplyToShader(phongShader);

            // 渲染立方体
            cubeRenderer->Render();

            // 渲染地板
            floorRenderer->Render();

            // ========================================
            // 交换缓冲区和事件处理
            // ========================================
            window.SwapBuffers();
            window.PollEvents();
        }

        Core::Logger::GetInstance().Info("========================================");
        Core::Logger::GetInstance().Info("Render loop ended");
        Core::Logger::GetInstance().Info("Total frames rendered: " + std::to_string(totalFrameCount));
        Core::Logger::GetInstance().Info("========================================");
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
