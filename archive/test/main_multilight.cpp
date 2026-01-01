/**
 * ========================================
 * 多光源系统演示 - Multi-Light Demo
 * ========================================
 *
 * 展示完整的多光源系统和光源可视化
 *
 * 特性：
 * - 多种光源类型（平行光、点光源、聚光灯）
 * - 光源位置可视化（发光立方体标识）
 * - 彩色点光源阵列
 * - 动态光源旋转
 * - 实时光照计算
 *
 * 控制说明：
 * - WASD: 前后左右移动
 * - Q/E: 上下移动
 * - 鼠标: 旋转视角
 * - TAB: 切换鼠标捕获
 * - ESC: 退出
 * - 1/2/3: 切换渲染场景
 * - L: 切换光源显示/隐藏
 * - SPACE: 暂停/恢复光源动画
 *
 * ========================================
 */

#include "Core/Window.hpp"
#include "Core/Camera.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp"
#include "Core/Logger.hpp"
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
#include <random>

// ========================================
// 全局配置
// ========================================

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const char* WINDOW_TITLE = "Multi-Light Demo - WASD: Move | Mouse: Look | 1/2/3: Scenes | L: Toggle Lights";

// 性能统计
float fps = 0.0f;
int totalFrames = 0;

// ========================================
// 场景生成函数
// ========================================

/**
 * 场景 1: 螺旋塔 (Spiral Tower)
 */
std::shared_ptr<Renderer::InstanceData> CreateSpiralTowerScene()
{
    Core::Logger::GetInstance().Info("Creating Scene 1: Spiral Tower...");

    auto instances = std::make_shared<Renderer::InstanceData>();

    int layers = 40;
    int cubesPerLayer = 8;
    float layerHeight = 1.0f;
    float radius = 6.0f;

    for (int layer = 0; layer < layers; ++layer)
    {
        float y = layer * layerHeight;
        float angleOffset = layer * 0.5f;

        for (int i = 0; i < cubesPerLayer; ++i)
        {
            float angle = angleOffset + (i / static_cast<float>(cubesPerLayer)) * glm::two_pi<float>();

            glm::vec3 position(
                std::cos(angle) * radius,
                y,
                std::sin(angle) * radius
            );

            glm::vec3 rotation(
                layer * 3.0f,
                glm::degrees(angle),
                i * 5.0f
            );

            float scaleVar = 1.0f - (layer / static_cast<float>(layers)) * 0.5f;
            glm::vec3 scale(scaleVar);

            // 橙色到黄色渐变
            float t = layer / static_cast<float>(layers);
            glm::vec3 color(
                1.0f,
                0.3f + 0.7f * t,
                0.1f + 0.3f * (1.0f - t)
            );

            instances->Add(position, rotation, scale, color);
        }
    }

    Core::Logger::GetInstance().Info("Spiral Tower created: " + std::to_string(instances->GetCount()) + " cubes");
    return instances;
}

/**
 * 场景 2: 波浪地板 (Wave Floor)
 */
std::shared_ptr<Renderer::InstanceData> CreateWaveFloorScene()
{
    Core::Logger::GetInstance().Info("Creating Scene 2: Wave Floor...");

    auto instances = std::make_shared<Renderer::InstanceData>();

    int gridSize = 25;
    float spacing = 1.8f;

    float startX = -((gridSize - 1) * spacing) / 2.0f;
    float startZ = -((gridSize - 1) * spacing) / 2.0f;

    for (int x = 0; x < gridSize; ++x)
    {
        for (int z = 0; z < gridSize; ++z)
        {
            glm::vec3 position(
                startX + x * spacing,
                0.0f,
                startZ + z * spacing
            );

            float dist1 = std::sqrt(static_cast<float>((x - gridSize/2) * (x - gridSize/2) +
                                                     (z - gridSize/2) * (z - gridSize/2)));
            float dist2 = std::sqrt(static_cast<float>(x * x + z * z));

            position.y = std::sin(dist1 * 0.4f) * 2.0f + std::cos(dist2 * 0.3f) * 1.5f;

            glm::vec3 rotation(0.0f, 0.0f, 0.0f);
            glm::vec3 scale(1.0f, 0.3f, 1.0f);

            // 海洋蓝色渐变
            float t = dist1 / (gridSize * 0.6f);
            glm::vec3 color(
                0.0f,
                0.3f + 0.4f * t,
                0.6f + 0.4f * (1.0f - t)
            );

            instances->Add(position, rotation, scale, color);
        }
    }

    Core::Logger::GetInstance().Info("Wave Floor created: " + std::to_string(instances->GetCount()) + " cubes");
    return instances;
}

/**
 * 场景 3: 浮动群岛 (Floating Islands)
 */
std::shared_ptr<Renderer::InstanceData> CreateFloatingIslandsScene()
{
    Core::Logger::GetInstance().Info("Creating Scene 3: Floating Islands...");

    auto instances = std::make_shared<Renderer::InstanceData>();

    std::random_device rd;
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> posDist(-25.0f, 25.0f);
    std::uniform_real_distribution<float> heightDist(8.0f, 20.0f);
    std::uniform_real_distribution<float> sizeDist(0.4f, 1.2f);
    std::uniform_real_distribution<float> colorDist(0.5f, 1.0f);

    int islandCount = 12;
    int cubesPerIsland = 100;

    for (int island = 0; island < islandCount; ++island)
    {
        glm::vec3 islandCenter(
            posDist(gen),
            heightDist(gen),
            posDist(gen)
        );

        float islandSize = sizeDist(gen) * 2.5f;

        for (int i = 0; i < cubesPerIsland; ++i)
        {
            float theta = std::uniform_real_distribution<float>(0, glm::two_pi<float>())(gen);
            float phi = std::uniform_real_distribution<float>(0, glm::pi<float>())(gen);
            float r = std::uniform_real_distribution<float>(0, islandSize)(gen);

            glm::vec3 position(
                islandCenter.x + r * std::sin(phi) * std::cos(theta),
                islandCenter.y + r * std::cos(phi) * 0.6f,
                islandCenter.z + r * std::sin(phi) * std::sin(theta)
            );

            glm::vec3 rotation(
                std::uniform_real_distribution<float>(0, 360)(gen),
                std::uniform_real_distribution<float>(0, 360)(gen),
                std::uniform_real_distribution<float>(0, 360)(gen)
            );

            float cubeSize = std::uniform_real_distribution<float>(0.4f, 0.9f)(gen);
            glm::vec3 scale(cubeSize);

            float hueShift = island / static_cast<float>(islandCount);
            glm::vec3 color(
                0.8f + 0.2f * std::sin(hueShift * glm::two_pi<float>()),
                0.3f + 0.3f * std::sin(hueShift * glm::two_pi<float>() + 2.0f),
                0.5f + 0.4f * std::sin(hueShift * glm::two_pi<float>() + 4.0f)
            );

            instances->Add(position, rotation, scale, color);
        }
    }

    Core::Logger::GetInstance().Info("Floating Islands created: " + std::to_string(instances->GetCount()) + " cubes");
    return instances;
}

// ========================================
// 光源初始化函数
// ========================================

void SetupLighting()
{
    auto& lightManager = Renderer::Lighting::LightManager::GetInstance();

    Core::Logger::GetInstance().Info("========================================");
    Core::Logger::GetInstance().Info("Setting up multi-light system...");
    Core::Logger::GetInstance().Info("========================================");

    // 1. 添加太阳光（平行光）- 暖白色
    auto sun = std::make_shared<Renderer::Lighting::DirectionalLight>(
        glm::vec3(-0.3f, -1.0f, -0.2f),     // 方向：从右上方照射
        glm::vec3(1.0f, 0.95f, 0.8f),        // 暖白色
        0.4f,                                // 强度适中
        0.15f, 0.6f, 0.3f                   // 较弱的环境光，中等漫反射，弱镜面反射
    );
    lightManager.AddDirectionalLight(sun);
    Core::Logger::GetInstance().Info("✓ Added sun (directional light)");

    // 2. 添加彩色点光源阵列（4个光源围绕场景旋转）
    glm::vec3 pointLightColors[] = {
        glm::vec3(1.0f, 0.2f, 0.2f),  // 红色
        glm::vec3(0.2f, 1.0f, 0.2f),  // 绿色
        glm::vec3(0.2f, 0.4f, 1.0f),  // 蓝色
        glm::vec3(1.0f, 0.9f, 0.2f)   // 黄色
    };

    float radius = 15.0f;
    for (int i = 0; i < 4; ++i)
    {
        float angle = i * glm::two_pi<float>() / 4.0f;
        glm::vec3 pos(
            std::cos(angle) * radius,
            10.0f,
            std::sin(angle) * radius
        );

        auto pointLight = std::make_shared<Renderer::Lighting::PointLight>(
            pos,
            pointLightColors[i],
            3.0f,  // 高强度，使彩色效果更明显
            0.05f, 0.8f, 0.5f,
            Renderer::Lighting::PointLight::Attenuation::Range32()
        );
        lightManager.AddPointLight(pointLight);
        Core::Logger::GetInstance().Info("✓ Added point light " + std::to_string(i) + " (color: " +
                                         std::to_string(pointLightColors[i].r) + ", " +
                                         std::to_string(pointLightColors[i].g) + ", " +
                                         std::to_string(pointLightColors[i].b) + ")");
    }

    // 3. 添加聚光灯（手电筒）- 将在渲染循环中跟随摄像机
    auto flashlight = std::make_shared<Renderer::Lighting::SpotLight>(
        glm::vec3(0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        2.0f,
        0.0f, 0.9f, 1.0f,
        Renderer::Lighting::PointLight::Attenuation::Range50(),
        glm::radians(12.5f),
        glm::radians(17.5f)
    );
    lightManager.AddSpotLight(flashlight);
    Core::Logger::GetInstance().Info("✓ Added flashlight (spot light)");

    Core::Logger::GetInstance().Info("========================================");
    lightManager.PrintAllLights();
    Core::Logger::GetInstance().Info("========================================");
}

/**
 * 创建光源标识（小立方体）
 */
std::vector<std::unique_ptr<Renderer::InstancedRenderer>> CreateLightVisualizers(
    const std::shared_ptr<Renderer::MeshBuffer>& cubeMesh,
    Renderer::Shader& visualizerShader)
{
    Core::Logger::GetInstance().Info("Creating light visualizers...");

    std::vector<std::unique_ptr<Renderer::InstancedRenderer>> visualizers;
    auto& lightManager = Renderer::Lighting::LightManager::GetInstance();

    // 为每个点光源创建一个发光立方体
    for (int i = 0; i < lightManager.GetPointLightCount(); ++i)
    {
        auto light = lightManager.GetPointLight(i);
        if (!light) continue;

        auto instances = std::make_shared<Renderer::InstanceData>();

        // 添加一个小立方体在光源位置
        instances->Add(
            light->GetPosition(),
            glm::vec3(0.0f),
            glm::vec3(0.3f),  // 小尺寸
            light->GetColor() * 2.0f  // 使用光源颜色，更亮
        );

        auto renderer = std::make_unique<Renderer::InstancedRenderer>();
        renderer->SetMesh(cubeMesh);
        renderer->SetInstances(instances);
        renderer->Initialize();

        visualizers.push_back(std::move(renderer));
    }

    Core::Logger::GetInstance().Info("Created " + std::to_string(visualizers.size()) + " light visualizers");
    return visualizers;
}

/**
 * 更新光源可视化标识的位置和颜色
 */
void UpdateLightVisualizers(
    std::vector<std::unique_ptr<Renderer::InstancedRenderer>>& visualizers)
{
    auto& lightManager = Renderer::Lighting::LightManager::GetInstance();

    for (int i = 0; i < lightManager.GetPointLightCount() && i < visualizers.size(); ++i)
    {
        auto light = lightManager.GetPointLight(i);
        if (!light) continue;

        // 更新实例数据
        auto instances = std::make_shared<Renderer::InstanceData>();
        instances->Add(
            light->GetPosition(),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.3f + 0.1f * std::sin(glfwGetTime() * 3.0f + i)),  // 脉冲效果
            light->GetColor() * 2.0f  // 使用光源颜色
        );

        visualizers[i]->SetInstances(instances);
        visualizers[i]->Initialize();
    }
}

// ========================================
// 主程序
// ========================================

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
        "logs/multi_light_demo.log",
        true,
        Core::LogLevel::INFO,
        true,
        rotationConfig
    );

    Core::Logger::GetInstance().Info("========================================");
    Core::Logger::GetInstance().Info("Multi-Light Demo - Starting...");
    Core::Logger::GetInstance().Info("========================================");

    try
    {
        // ========================================
        // 创建窗口
        // ========================================
        Core::Logger::GetInstance().Info("Creating window...");
        Core::Window window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
        window.Init();

        // ========================================
        // 初始化摄像机和输入控制器
        // ========================================
        Core::Logger::GetInstance().Info("Initializing camera and input controllers...");

        Core::Camera camera(
            glm::vec3(0.0f, 20.0f, 45.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            -90.0f,
            -20.0f
        );

        Core::MouseController mouseController;
        mouseController.Initialize(glfwGetCurrentContext());
        mouseController.SetMouseCapture(true);

        // 设置鼠标回调
        glfwSetCursorPosCallback(glfwGetCurrentContext(), [](GLFWwindow* window, double xpos, double ypos) {
            static bool firstMouse = true;
            static float lastX = WINDOW_WIDTH / 2.0f;
            static float lastY = WINDOW_HEIGHT / 2.0f;

            int mouseCaptured = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
            if (!mouseCaptured) return;

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

        // ========================================
        // 初始化多光源系统
        // ========================================
        SetupLighting();

        // ========================================
        // 加载着色器
        // ========================================
        Core::Logger::GetInstance().Info("Loading shaders...");

        Renderer::Shader multiLightShader;
        multiLightShader.Load("assets/shader/multi_light.vert", "assets/shader/multi_light.frag");

        Renderer::Shader lightVisualizerShader;
        lightVisualizerShader.Load("assets/shader/basic.vert", "assets/shader/basic.frag");

        // ========================================
        // 创建立方体网格
        // ========================================
        Core::Logger::GetInstance().Info("Creating cube mesh buffer...");
        Renderer::MeshBuffer cubeMesh = Renderer::MeshBufferFactory::CreateCubeBuffer();
        auto cubeMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(cubeMesh));

        // ========================================
        // 创建场景
        // ========================================
        std::vector<std::shared_ptr<Renderer::InstanceData>> scenes;
        scenes.push_back(CreateSpiralTowerScene());
        scenes.push_back(CreateWaveFloorScene());
        scenes.push_back(CreateFloatingIslandsScene());

        int currentScene = 0;

        // ========================================
        // 创建场景渲染器
        // ========================================
        Core::Logger::GetInstance().Info("Creating scene renderers...");

        std::vector<std::unique_ptr<Renderer::InstancedRenderer>> renderers;
        for (const auto& scene : scenes)
        {
            auto renderer = std::make_unique<Renderer::InstancedRenderer>();
            renderer->SetMesh(cubeMeshPtr);
            renderer->SetInstances(scene);
            renderer->Initialize();
            renderers.push_back(std::move(renderer));
        }

        // ========================================
        // 创建光源可视化标识
        // ========================================
        bool showLights = true;
        bool animateLights = true;

        auto lightVisualizers = CreateLightVisualizers(cubeMeshPtr, lightVisualizerShader);

        // ========================================
        // 键盘回调
        // ========================================
        keyboardController.RegisterKeyCallback(GLFW_KEY_ESCAPE, []()
        {
            Core::Logger::GetInstance().Info("Exit requested");
            exit(0);
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_TAB, [&mouseController]()
        {
            mouseController.ToggleMouseCapture();
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_1, [&currentScene, &camera]()
        {
            currentScene = 0;
            camera.SetPosition(glm::vec3(0.0f, 25.0f, 30.0f));
            Core::Logger::GetInstance().Info("Switched to Scene 1: Spiral Tower");
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_2, [&currentScene, &camera]()
        {
            currentScene = 1;
            camera.SetPosition(glm::vec3(0.0f, 15.0f, 50.0f));
            Core::Logger::GetInstance().Info("Switched to Scene 2: Wave Floor");
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_3, [&currentScene, &camera]()
        {
            currentScene = 2;
            camera.SetPosition(glm::vec3(0.0f, 20.0f, 55.0f));
            Core::Logger::GetInstance().Info("Switched to Scene 3: Floating Islands");
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_L, [&showLights]()
        {
            showLights = !showLights;
            Core::Logger::GetInstance().Info("Light visualizers " + std::string(showLights ? "enabled" : "disabled"));
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_SPACE, [&animateLights]()
        {
            animateLights = !animateLights;
            Core::Logger::GetInstance().Info("Light animation " + std::string(animateLights ? "resumed" : "paused"));
        });

        // ========================================
        // OpenGL 设置
        // ========================================
        Core::Logger::GetInstance().Info("Configuring OpenGL...");
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);  // 深蓝色背景

        // ========================================
        // 渲染循环
        // ========================================
        Core::Logger::GetInstance().Info("Starting render loop...");

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
                fps = fpsFrameCount / (currentTime - fpsLastTime);

                static int logCounter = 0;
                if (++logCounter >= 2)
                {
                    Core::Logger::GetInstance().Info(
                        "Scene " + std::to_string(currentScene + 1) + " | " +
                        "FPS: " + std::to_string(static_cast<int>(fps)) + " | " +
                        "Instances: " + std::to_string(scenes[currentScene]->GetCount()) + " | " +
                        "Lights: " + std::to_string(Renderer::Lighting::LightManager::GetInstance().GetTotalLightCount())
                    );
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

            // 摄像机移动
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
            // 更新光源动画
            // ========================================
            if (animateLights)
            {
                auto& lightManager = Renderer::Lighting::LightManager::GetInstance();

                // 旋转点光源
                float rotationSpeed = 0.5f;
                float radius = 15.0f;
                for (int i = 0; i < lightManager.GetPointLightCount(); ++i)
                {
                    auto light = lightManager.GetPointLight(i);
                    if (!light) continue;

                    float angle = i * glm::two_pi<float>() / 4.0f + currentTime * rotationSpeed;
                    glm::vec3 newPos(
                        std::cos(angle) * radius,
                        10.0f + std::sin(currentTime * 2.0f + i) * 3.0f,  // 上下浮动
                        std::sin(angle) * radius
                    );
                    light->SetPosition(newPos);
                }

                // 更新手电筒位置和方向
                auto flashlight = lightManager.GetSpotLight(0);
                if (flashlight)
                {
                    flashlight->SetPosition(camera.GetPosition());
                    flashlight->SetDirection(camera.GetFront());
                }

                // 更新光源可视化标识
                if (showLights)
                {
                    UpdateLightVisualizers(lightVisualizers);
                }
            }

            // ========================================
            // 渲染设置
            // ========================================
            float aspectRatio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
            glm::mat4 projection = camera.GetProjectionMatrix(aspectRatio, 0.1f, 300.0f);
            glm::mat4 view = camera.GetViewMatrix();

            // 清空缓冲区
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // ========================================
            // 渲染场景（使用多光源着色器）
            // ========================================
            multiLightShader.Use();

            // 设置摄像机和材质
            multiLightShader.SetMat4("projection", projection);
            multiLightShader.SetMat4("view", view);
            multiLightShader.SetVec3("viewPos", camera.GetPosition());
            multiLightShader.SetFloat("shininess", 64.0f);
            multiLightShader.SetBool("useInstanceColor", true);
            multiLightShader.SetBool("useTexture", false);

            // 应用所有光源
            Renderer::Lighting::LightManager::GetInstance().ApplyToShader(multiLightShader);

            // 渲染场景
            renderers[currentScene]->Render();

            // ========================================
            // 渲染光源可视化标识
            // ========================================
            if (showLights)
            {
                lightVisualizerShader.Use();
                lightVisualizerShader.SetMat4("projection", projection);
                lightVisualizerShader.SetMat4("view", view);

                // 为每个光源标识设置自发光颜色
                auto& lightManager = Renderer::Lighting::LightManager::GetInstance();
                for (int i = 0; i < lightVisualizers.size(); ++i)
                {
                    auto light = lightManager.GetPointLight(i);
                    if (!light) continue;

                    lightVisualizerShader.SetVec3("lightColor", light->GetColor() * 1.5f);
                    lightVisualizerShader.SetVec3("objectColor", light->GetColor() * 2.0f);
                    lightVisualizerShader.SetFloat("ambientStrength", 1.0f);
                    lightVisualizerShader.SetBool("useTexture", false);

                    lightVisualizers[i]->Render();
                }
            }

            // ========================================
            // 交换缓冲区和事件处理
            // ========================================
            window.SwapBuffers();
            window.PollEvents();
        }

        // ========================================
        // 清理和退出
        // ========================================
        Core::Logger::GetInstance().Info("========================================");
        Core::Logger::GetInstance().Info("Render loop ended");
        Core::Logger::GetInstance().Info("Total frames rendered: " + std::to_string(totalFrameCount));
        Core::Logger::GetInstance().Info("Average FPS: " + std::to_string(fps));
        Core::Logger::GetInstance().Info("Shutting down gracefully...");
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
