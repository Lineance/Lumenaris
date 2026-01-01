/**
 * ========================================
 * 炫酷多方块渲染演示 - Cool Cubes Demo
 * ========================================
 *
 * 本演示展示现代 OpenGL 实例化渲染的强大性能
 *
 * 特性：
 * - 5000+ 动态旋转的立方体
 * - 多层建筑结构（螺旋塔、波浪地板、浮动岛屿）
 * - 实时光照和颜色渐变
 * - 流畅的摄像机控制
 * - FPS 性能监控
 *
 * 技术亮点：
 * - 使用新架构：MeshBuffer + InstanceData + InstancedRenderer
 * - 单次绘制调用渲染所有实例
 * - 移动语义优化，零不必要拷贝
 *
 * 控制说明：
 * - WASD: 前后左右移动
 * - Q/E: 上下移动
 * - 鼠标: 旋转视角
 * - TAB: 切换鼠标捕获
 * - ESC: 退出
 * - 1/2/3: 切换渲染场景
 *
 * ========================================
 */

#include "Core/Window.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp"
#include "Core/Logger.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/MeshBuffer.hpp"
#include "Renderer/MeshDataFactory.hpp"
#include "Renderer/InstancedRenderer.hpp"
#include "Renderer/InstanceData.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>

// ========================================
// 全局配置
// ========================================

// 窗口设置
const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const char* WINDOW_TITLE = "Cool Cubes Demo - Press 1/2/3 to Switch Scenes";

// 摄像机设置
glm::vec3 cameraPos = glm::vec3(0.0f, 15.0f, 40.0f);
float cameraSpeed = 15.0f;

// 光照设置
glm::vec3 lightPos = glm::vec3(20.0f, 30.0f, 20.0f);
glm::vec3 lightColor = glm::vec3(1.0f, 0.95f, 0.9f);

// 性能统计
float fps = 0.0f;
int totalFrames = 0;

// ========================================
// 场景生成函数
// ========================================

/**
 * 场景 1: 螺旋塔 (Spiral Tower)
 * 创建一个向上旋转的螺旋形立方体塔
 */
std::shared_ptr<Renderer::InstanceData> CreateSpiralTowerScene()
{
    Core::Logger::GetInstance().Info("Creating Scene 1: Spiral Tower...");

    auto instances = std::make_shared<Renderer::InstanceData>();

    int layers = 40;              // ✅ 增加层数
    int cubesPerLayer = 8;        // ✅ 减少每层立方体数
    float layerHeight = 1.0f;     // 层高
    float radius = 6.0f;          // ✅ 螺旋半径

    for (int layer = 0; layer < layers; ++layer)
    {
        float y = layer * layerHeight;
        float angleOffset = layer * 0.5f;  // ✅ 每层旋转角度

        for (int i = 0; i < cubesPerLayer; ++i)
        {
            float angle = angleOffset + (i / static_cast<float>(cubesPerLayer)) * glm::two_pi<float>();

            // 螺旋位置
            glm::vec3 position(
                std::cos(angle) * radius,
                y,
                std::sin(angle) * radius
            );

            // ✅ 旋转角度
            glm::vec3 rotation(
                layer * 3.0f,
                glm::degrees(angle),
                i * 5.0f
            );

            // 尺寸变化（底部大，顶部小）
            float scaleVar = 1.0f - (layer / static_cast<float>(layers)) * 0.5f;  // 1.0 -> 0.5
            glm::vec3 scale(scaleVar);

            // ✅ 火焰渐变颜色（从红到黄）
            float t = layer / static_cast<float>(layers);
            glm::vec3 color(
                1.0f,                          // R
                0.3f + 0.7f * t,              // G: 增加
                0.1f + 0.3f * (1.0f - t)      // B: 减少
            );

            instances->Add(position, rotation, scale, color);
        }
    }

    Core::Logger::GetInstance().Info("Spiral Tower created: " + std::to_string(instances->GetCount()) + " cubes");
    return instances;
}

/**
 * 场景 2: 波浪地板 (Wave Floor)
 * 创建一个动态波浪状的地板
 */
std::shared_ptr<Renderer::InstanceData> CreateWaveFloorScene()
{
    Core::Logger::GetInstance().Info("Creating Scene 2: Wave Floor...");

    auto instances = std::make_shared<Renderer::InstanceData>();

    int gridSize = 25;           // ✅ 网格大小
    float spacing = 1.8f;        // 间距

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

            // ✅ 更复杂的波浪（两个波浪叠加）
            float dist1 = std::sqrt(static_cast<float>((x - gridSize/2) * (x - gridSize/2) +
                                                     (z - gridSize/2) * (z - gridSize/2)));
            float dist2 = std::sqrt(static_cast<float>(x * x + z * z));

            position.y = std::sin(dist1 * 0.4f) * 2.0f + std::cos(dist2 * 0.3f) * 1.5f;

            // ✅ 简化旋转
            glm::vec3 rotation(0.0f, 0.0f, 0.0f);

            // ✅ 地面平坦的立方体（压扁）
            glm::vec3 scale(1.0f, 0.3f, 1.0f);

            // ✅ 海洋蓝色渐变
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
 * 创建多个浮动的小岛，每个小岛由多个立方体组成
 */
std::shared_ptr<Renderer::InstanceData> CreateFloatingIslandsScene()
{
    Core::Logger::GetInstance().Info("Creating Scene 3: Floating Islands...");

    auto instances = std::make_shared<Renderer::InstanceData>();

    std::random_device rd;
    std::mt19937 gen(42);  // ✅ 固定种子，确保每次运行相同
    std::uniform_real_distribution<float> posDist(-25.0f, 25.0f);
    std::uniform_real_distribution<float> heightDist(8.0f, 20.0f);
    std::uniform_real_distribution<float> sizeDist(0.4f, 1.2f);
    std::uniform_real_distribution<float> colorDist(0.5f, 1.0f);

    int islandCount = 12;       // ✅ 岛屿数量
    int cubesPerIsland = 100;   // ✅ 每个岛屿的立方体数

    for (int island = 0; island < islandCount; ++island)
    {
        // 岛屿中心位置
        glm::vec3 islandCenter(
            posDist(gen),
            heightDist(gen),
            posDist(gen)
        );

        float islandSize = sizeDist(gen) * 2.5f;

        // 每个岛屿的立方体（球形分布）
        for (int i = 0; i < cubesPerIsland; ++i)
        {
            // 球形分布
            float theta = std::uniform_real_distribution<float>(0, glm::two_pi<float>())(gen);
            float phi = std::uniform_real_distribution<float>(0, glm::pi<float>())(gen);
            float r = std::uniform_real_distribution<float>(0, islandSize)(gen);

            glm::vec3 position(
                islandCenter.x + r * std::sin(phi) * std::cos(theta),
                islandCenter.y + r * std::cos(phi) * 0.6f,  // 压扁的球体
                islandCenter.z + r * std::sin(phi) * std::sin(theta)
            );

            // ✅ 随机旋转
            glm::vec3 rotation(
                std::uniform_real_distribution<float>(0, 360)(gen),
                std::uniform_real_distribution<float>(0, 360)(gen),
                std::uniform_real_distribution<float>(0, 360)(gen)
            );

            float cubeSize = std::uniform_real_distribution<float>(0.4f, 0.9f)(gen);
            glm::vec3 scale(cubeSize);

            // ✅ 每个岛屿独特的颜色（紫色、粉色、橙色系）
            float hueShift = island / static_cast<float>(islandCount);
            glm::vec3 color(
                0.8f + 0.2f * std::sin(hueShift * glm::two_pi<float>()),         // R: 高
                0.3f + 0.3f * std::sin(hueShift * glm::two_pi<float>() + 2.0f),  // G: 中
                0.5f + 0.4f * std::sin(hueShift * glm::two_pi<float>() + 4.0f)   // B: 中高
            );

            instances->Add(position, rotation, scale, color);
        }
    }

    Core::Logger::GetInstance().Info("Floating Islands created: " + std::to_string(instances->GetCount()) + " cubes");
    return instances;
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
    rotationConfig.maxFileSize = 5 * 1024 * 1024; // 5MB
    rotationConfig.maxFiles = 3;

    Core::Logger::GetInstance().Initialize(
        "logs/cool_cubes_demo.log",
        true,
        Core::LogLevel::INFO,
        true,
        rotationConfig
    );

    Core::Logger::GetInstance().Info("========================================");
    Core::Logger::GetInstance().Info("Cool Cubes Demo - Starting...");
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
        // 初始化输入控制器
        // ========================================
        Core::Logger::GetInstance().Info("Initializing input controllers...");

        Core::MouseController mouseController;
        mouseController.Initialize(glfwGetCurrentContext());
        mouseController.SetMouseCapture(true);

        Core::KeyboardController keyboardController;
        keyboardController.Initialize(glfwGetCurrentContext());

        // 键盘回调
        keyboardController.RegisterKeyCallback(GLFW_KEY_ESCAPE, []()
        {
            Core::Logger::GetInstance().Info("Exit requested");
            exit(0);
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_TAB, [&mouseController]()
        {
            mouseController.ToggleMouseCapture();
        });

        // ========================================
        // 加载着色器
        // ========================================
        Core::Logger::GetInstance().Info("Loading instanced shader...");
        Renderer::Shader instancedShader;
        instancedShader.Load("assets/shader/instanced.vert", "assets/shader/instanced.frag");

        // ========================================
        // 创建网格（立方体）
        // ========================================
        Core::Logger::GetInstance().Info("Creating cube mesh buffer...");
        Renderer::MeshBuffer cubeMesh = Renderer::MeshBufferFactory::CreateCubeBuffer();

        // ========================================
        // 创建场景
        // ========================================
        std::vector<std::shared_ptr<Renderer::InstanceData>> scenes;
        scenes.push_back(CreateSpiralTowerScene());       // 场景 1
        scenes.push_back(CreateWaveFloorScene());         // 场景 2
        scenes.push_back(CreateFloatingIslandsScene());   // 场景 3

        int currentScene = 0;

        // ========================================
        // 创建渲染器
        // ========================================
        Core::Logger::GetInstance().Info("Creating instanced renderer...");

        // 输出立方体网格信息用于调试
        Core::Logger::GetInstance().Info("Cube mesh info:");
        Core::Logger::GetInstance().Info("  VAO: " + std::to_string(cubeMesh.GetVAO()));
        Core::Logger::GetInstance().Info("  Vertex Count: " + std::to_string(cubeMesh.GetVertexCount()));
        Core::Logger::GetInstance().Info("  Index Count: " + std::to_string(cubeMesh.GetIndexCount()));
        Core::Logger::GetInstance().Info("  Has Indices: " + std::string(cubeMesh.HasIndices() ? "Yes" : "No"));

        auto cubeMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(cubeMesh));

        std::vector<std::unique_ptr<Renderer::InstancedRenderer>> renderers;
        for (const auto& scene : scenes)
        {
            auto renderer = std::make_unique<Renderer::InstancedRenderer>();
            renderer->SetMesh(cubeMeshPtr);
            renderer->SetInstances(scene);
            renderer->Initialize();
            renderers.push_back(std::move(renderer));
        }

        // 场景切换回调（使用指针避免捕获全局变量）
        keyboardController.RegisterKeyCallback(GLFW_KEY_1, [&currentScene]()
        {
            currentScene = 0;
            cameraPos = glm::vec3(0.0f, 25.0f, 30.0f);  // ✅ 调整：更近的视角观察塔
            Core::Logger::GetInstance().Info("Switched to Scene 1: Spiral Tower");
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_2, [&currentScene]()
        {
            currentScene = 1;
            cameraPos = glm::vec3(0.0f, 15.0f, 50.0f);  // ✅ 调整：俯视波浪
            Core::Logger::GetInstance().Info("Switched to Scene 2: Wave Floor");
            Core::Logger::GetInstance().Info("Camera position: " +
                                            std::to_string(cameraPos.x) + ", " +
                                            std::to_string(cameraPos.y) + ", " +
                                            std::to_string(cameraPos.z));
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_3, [&currentScene]()
        {
            currentScene = 2;
            cameraPos = glm::vec3(0.0f, 20.0f, 55.0f);  // ✅ 调整：观察群岛
            Core::Logger::GetInstance().Info("Switched to Scene 3: Floating Islands");
        });

        Core::Logger::GetInstance().Info("========================================");
        Core::Logger::GetInstance().Info("All scenes loaded successfully!");
        Core::Logger::GetInstance().Info("Total scenes: " + std::to_string(scenes.size()));
        Core::Logger::GetInstance().Info("========================================");
        Core::Logger::GetInstance().Info("Controls:");
        Core::Logger::GetInstance().Info("  WASD - Move camera");
        Core::Logger::GetInstance().Info("  Q/E  - Move up/down");
        Core::Logger::GetInstance().Info("  Mouse - Look around");
        Core::Logger::GetInstance().Info("  TAB  - Toggle mouse capture");
        Core::Logger::GetInstance().Info("  1/2/3 - Switch scenes");
        Core::Logger::GetInstance().Info("  ESC  - Exit");
        Core::Logger::GetInstance().Info("========================================");

        // ========================================
        // OpenGL 设置
        // ========================================
        Core::Logger::GetInstance().Info("Configuring OpenGL...");
        glEnable(GL_DEPTH_TEST);

        // 面剔除设置（临时禁用以确保所有面都可见）
        // 如果某些面不可见，可能是顶点缠绕顺序问题
        // glEnable(GL_CULL_FACE);
        // glCullFace(GL_BACK);
        // glFrontFace(GL_CCW);

        // 深色背景
        glClearColor(0.05f, 0.08f, 0.12f, 1.0f);

        // ========================================
        // 渲染循环
        // ========================================
        Core::Logger::GetInstance().Info("Starting render loop...");

        double lastTime = glfwGetTime();
        double fpsLastTime = glfwGetTime();
        int fpsFrameCount = 0;
        int totalFrameCount = 0;
        float rotationAngle = 0.0f;

        while (!window.ShouldClose())
        {
            // FPS 计算
            double currentTime = glfwGetTime();
            fpsFrameCount++;
            totalFrameCount++;

            if (currentTime - fpsLastTime >= 0.5)
            {
                fps = fpsFrameCount / (currentTime - fpsLastTime);
                Core::Logger::GetInstance().SetFPS(static_cast<int>(fps));

                // 每秒输出一次统计
                static int logCounter = 0;
                if (++logCounter >= 2)  // 每1秒输出一次
                {
                    Core::Logger::GetInstance().Info(
                        "Scene " + std::to_string(currentScene + 1) + " | " +
                        "FPS: " + std::to_string(static_cast<int>(fps)) + " | " +
                        "Instances: " + std::to_string(scenes[currentScene]->GetCount()) + " | " +
                        "Total Frames: " + std::to_string(totalFrameCount)
                    );
                    logCounter = 0;
                }

                fpsFrameCount = 0;
                fpsLastTime = currentTime;
            }

            float deltaTime = static_cast<float>(currentTime - lastTime);
            lastTime = currentTime;

            // 更新动画
            rotationAngle += deltaTime * 10.0f;

            // ========================================
            // 输入处理
            // ========================================
            keyboardController.Update(deltaTime);

            float moveSpeed = cameraSpeed * deltaTime;
            glm::vec3 moveDirection(0.0f);

            // 摄像机移动
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

            // ========================================
            // 渲染设置
            // ========================================
            float aspectRatio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
            glm::mat4 projection = glm::perspective(glm::radians(mouseController.GetFOV()), aspectRatio, 0.1f, 300.0f);
            glm::mat4 view = glm::lookAt(cameraPos, cameraPos + mouseController.GetCameraFront(), glm::vec3(0.0f, 1.0f, 0.0f));

            // 设置日志上下文
            Core::LogContext renderContext;
            renderContext.renderPass = "CoolCubesDemo";
            renderContext.batchIndex = currentScene;
            renderContext.drawCallCount = 1;
            renderContext.currentShader = "InstancedShader";
            Core::Logger::GetInstance().SetContext(renderContext);

            // 清空缓冲区
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // ========================================
            // 设置着色器参数
            // ========================================
            instancedShader.Use();
            instancedShader.SetMat4("projection", projection);
            instancedShader.SetMat4("view", view);
            instancedShader.SetVec3("lightPos", lightPos);
            instancedShader.SetVec3("lightColor", lightColor);
            instancedShader.SetVec3("viewPos", cameraPos);
            instancedShader.SetFloat("ambientStrength", 0.5f);  // 增强环境光（原来 0.3f）
            instancedShader.SetFloat("specularStrength", 0.6f);
            instancedShader.SetFloat("shininess", 64.0f);
            instancedShader.SetBool("useInstanceColor", true);
            instancedShader.SetBool("useTexture", false);
            instancedShader.SetFloat("time", static_cast<float>(currentTime));  // 传递时间给着色器

            // ========================================
            // 渲染当前场景
            // ========================================

            // ✅ 添加场景渲染的调试信息
            static int lastScene = -1;
            if (lastScene != currentScene)
            {
                Core::Logger::GetInstance().Info("Rendering scene " + std::to_string(currentScene + 1) +
                                                 " with " + std::to_string(scenes[currentScene]->GetCount()) + " instances");
                lastScene = currentScene;
            }

            renderers[currentScene]->Render();

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
