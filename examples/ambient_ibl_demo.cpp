/**
 * ========================================
 * Ambient IBL Demo - 轻量级环境光照演示
 * ========================================
 *
 * 功能演示：
 * - 天空盒渲染
 * - 轻量级IBL环境光照（从天空盒采样）
 * - 三种环境光模式切换：
 *   1. 固定颜色（传统Phong）
 *   2. 天空盒采样（Skybox Sample）
 *   3. 半球光照（Hemisphere）
 * - 与现有的多光源系统完全兼容
 *
 * 纹理要求：
 * - 6个JPG/PNG纹理文件（天空盒）
 *
 * 控制说明：
 * - WASD: 前后左右移动
 * - Q/E: 上下移动
 * - 鼠标: 旋转视角
 * - TAB: 切换鼠标捕获
 * - 1/2/3: 切换环境光模式
 * - [/]: 调整环境光强度
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
#include "Renderer/Environment/AmbientLighting.hpp"
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
const char* WINDOW_TITLE = "Ambient IBL Demo - Press 1/2/3 to switch mode";

// 全局变量
Renderer::AmbientLighting::Mode currentMode = Renderer::AmbientLighting::Mode::SKYBOX_SAMPLE;
float ambientIntensity = 0.4f;

/**
 * 创建测试场景
 */
std::shared_ptr<Renderer::InstanceData> CreateTestScene()
{
    Core::Logger::GetInstance().Info("Creating test scene...");

    auto instances = std::make_shared<Renderer::InstanceData>();

    // 创建球体阵列
    int gridSize = 4;
    float spacing = 4.0f;

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
                0.8f,
                (float)x / (gridSize - 1) * 0.5f + 0.5f,
                (float)z / (gridSize - 1) * 0.5f + 0.5f
            );

            instances->Add(position, rotation, scale, color);
        }
    }

    Core::Logger::GetInstance().Info("Test scene created: " +
                                     std::to_string(instances->GetCount()) + " spheres");
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
        glm::vec3(50.0f, 50.0f, 1.0f),
        glm::vec3(0.6f, 0.6f, 0.6f)
    );

    return instances;
}

/**
 * 设置光照
 */
void SetupLighting()
{
    auto& lightManager = Renderer::Lighting::LightManager::GetInstance();

    // 太阳光（平行光）- 降低强度，让环境光更明显
    auto sun = std::make_shared<Renderer::Lighting::DirectionalLight>(
        glm::vec3(-0.2f, -1.0f, -0.3f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        1.0f,
        0.05f, 0.3f, 0.2f  // 降低直接光强度
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
        "logs/ambient_ibl_demo.log",
        true,
        Core::LogLevel::INFO,
        true,
        rotationConfig
    );

    Core::Logger::GetInstance().Info("========================================");
    Core::Logger::GetInstance().Info("Ambient IBL Demo - Starting...");
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
            glm::vec3(0.0f, 5.0f, 12.0f),
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
        // 创建环境光照系统
        // ========================================
        Core::Logger::GetInstance().Info("Creating ambient lighting system...");
        Renderer::AmbientLighting ambientLighting;
        ambientLighting.Initialize();

        if (skyboxLoaded)
        {
            ambientLighting.LoadFromSkybox(skybox, 0.4f);
        }

        // 设置半球光照颜色（备用）
        ambientLighting.SetHemisphereColors(
            glm::vec3(0.5f, 0.7f, 1.0f),  // 蓝天
            glm::vec3(0.1f, 0.1f, 0.1f)   // 深灰地面
        );

        // ========================================
        // 加载着色器
        // ========================================
        Core::Logger::GetInstance().Info("Loading shaders...");
        Renderer::Shader ambientShader;
        ambientShader.Load("assets/shader/ambient_ibl.vert", "assets/shader/ambient_ibl.frag");

        // ========================================
        // 创建场景几何体
        // ========================================
        auto sphereInstances = CreateTestScene();
        auto floorInstances = CreateFloor();

        Renderer::MeshBuffer sphereMesh = Renderer::MeshBufferFactory::CreateSphereBuffer(32, 32, 1.0f);
        auto sphereMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(sphereMesh));

        Renderer::MeshBuffer floorMesh = Renderer::MeshBufferFactory::CreatePlaneBuffer(1.0f, 1.0f, 1, 1);
        auto floorMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(floorMesh));

        auto sphereRenderer = std::make_unique<Renderer::InstancedRenderer>();
        sphereRenderer->SetMesh(sphereMeshPtr);
        sphereRenderer->SetInstances(sphereInstances);
        sphereRenderer->Initialize();

        auto floorRenderer = std::make_unique<Renderer::InstancedRenderer>();
        floorRenderer->SetMesh(floorMeshPtr);
        floorRenderer->SetInstances(floorInstances);
        floorRenderer->Initialize();

        // ========================================
        // 键盘控制
        // ========================================
        keyboardController.RegisterKeyCallback(GLFW_KEY_1, [&ambientLighting]()
        {
            ambientLighting.SetMode(Renderer::AmbientLighting::Mode::SOLID_COLOR);
            currentMode = Renderer::AmbientLighting::Mode::SOLID_COLOR;
            Core::Logger::GetInstance().Info("Ambient mode: SOLID_COLOR (Traditional Phong)");
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_2, [&ambientLighting]()
        {
            ambientLighting.SetMode(Renderer::AmbientLighting::Mode::SKYBOX_SAMPLE);
            currentMode = Renderer::AmbientLighting::Mode::SKYBOX_SAMPLE;
            Core::Logger::GetInstance().Info("Ambient mode: SKYBOX_SAMPLE (IBL from skybox)");
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_3, [&ambientLighting]()
        {
            ambientLighting.SetMode(Renderer::AmbientLighting::Mode::HEMISPHERE);
            currentMode = Renderer::AmbientLighting::Mode::HEMISPHERE;
            Core::Logger::GetInstance().Info("Ambient mode: HEMISPHERE (Gradient sky to ground)");
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_RIGHT_BRACKET, [&]()
        {
            ambientIntensity = std::min(1.0f, ambientIntensity + 0.05f);
            ambientLighting.SetIntensity(ambientIntensity);
            Core::Logger::GetInstance().Info("Ambient intensity: " + std::to_string(ambientIntensity));
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_LEFT_BRACKET, [&]()
        {
            ambientIntensity = std::max(0.0f, ambientIntensity - 0.05f);
            ambientLighting.SetIntensity(ambientIntensity);
            Core::Logger::GetInstance().Info("Ambient intensity: " + std::to_string(ambientIntensity));
        });

        // ========================================
        // OpenGL 设置
        // ========================================
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        Core::Logger::GetInstance().Info("========================================");
        Core::Logger::GetInstance().Info("Ambient IBL Demo loaded successfully!");
        Core::Logger::GetInstance().Info("Controls:");
        Core::Logger::GetInstance().Info("  WASD - Move camera");
        Core::Logger::GetInstance().Info("  Q/E  - Move up/down");
        Core::Logger::GetInstance().Info("  Mouse - Look around");
        Core::Logger::GetInstance().Info("  TAB  - Toggle mouse capture");
        Core::Logger::GetInstance().Info("  1    - Solid color ambient");
        Core::Logger::GetInstance().Info("  2    - Skybox sample ambient (IBL)");
        Core::Logger::GetInstance().Info("  3    - Hemisphere ambient");
        Core::Logger::GetInstance().Info("  [ / ] - Decrease/Increase ambient intensity");
        Core::Logger::GetInstance().Info("  ESC  - Exit");
        Core::Logger::GetInstance().Info("========================================");

        // ========================================
        // 渲染循环
        // ========================================
        double lastTime = glfwGetTime();
        double fpsLastTime = glfwGetTime();
        int fpsFrameCount = 0;
        int totalFrameCount = 0;

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
                    std::string modeStr;
                    switch (currentMode)
                    {
                        case Renderer::AmbientLighting::Mode::SOLID_COLOR:
                            modeStr = "Solid Color";
                            break;
                        case Renderer::AmbientLighting::Mode::SKYBOX_SAMPLE:
                            modeStr = "Skybox IBL";
                            break;
                        case Renderer::AmbientLighting::Mode::HEMISPHERE:
                            modeStr = "Hemisphere";
                            break;
                    }

                    std::string logMessage = "Ambient IBL | FPS: " +
                                             std::to_string(static_cast<int>(fps)) +
                                             " | Mode: " + modeStr +
                                             " | Intensity: " + std::to_string(ambientIntensity);
                    Core::Logger::GetInstance().Info(logMessage);
                    logCounter = 0;
                }

                fpsFrameCount = 0;
                fpsLastTime = currentTime;
            }

            float deltaTime = static_cast<float>(currentTime - lastTime);
            lastTime = currentTime;

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
            // 渲染天空盒
            // ========================================
            if (skyboxLoaded)
            {
                skybox.Render(projection, view);
            }

            // ========================================
            // 渲染场景物体（使用环境光照）
            // ========================================
            ambientShader.Use();
            ambientShader.SetMat4("projection", projection);
            ambientShader.SetMat4("view", view);
            ambientShader.SetVec3("viewPos", camera.GetPosition());
            ambientShader.SetBool("useInstanceColor", true);
            ambientShader.SetBool("useTexture", false);
            ambientShader.SetFloat("shininess", 64.0f);

            // 应用环境光照
            ambientLighting.ApplyToShader(ambientShader);

            // 应用直接光源
            Renderer::Lighting::LightManager::GetInstance().ApplyToShader(ambientShader);

            // 渲染球体
            sphereRenderer->Render();

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
