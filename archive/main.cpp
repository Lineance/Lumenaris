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

// 窗口设置
const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const char* WINDOW_TITLE = "Sphere Demo - 9 Spheres | SPACE: Pause Animation";

// 性能统计
float fps = 0.0f;
int totalFrames = 0;

// ========================================
// 场景生成函数
// ========================================

/**
 * 初始化多光源系统
 */
void SetupLighting(
    std::vector<Renderer::Lighting::PointLightPtr>& outRotatingLights,
    Renderer::Lighting::SpotLightPtr& outFlashlight,
    glm::vec3& outCenterPosition)
{
    auto& lightManager = Renderer::Lighting::LightManager::GetInstance();

    Core::Logger::GetInstance().Info("========================================");
    Core::Logger::GetInstance().Info("Setting up multi-light system...");
    Core::Logger::GetInstance().Info("========================================");

    // 1. 太阳光（平行光）- 暖白色，从斜上方照射
    auto sun = std::make_shared<Renderer::Lighting::DirectionalLight>(
        glm::vec3(-0.3f, -1.0f, -0.2f),  // 方向：从右上方往下
        glm::vec3(1.0f, 0.95f, 0.8f),     // 暖白色
        0.2f,                             // ✅ 进一步降低强度
        0.1f, 0.3f, 0.2f                  // ✅ 降低环境光，让彩色光更突出
    );
    lightManager.AddDirectionalLight(sun);
    Core::Logger::GetInstance().Info("✓ Added sun (directional light)");

    // 2. 彩色点光源阵列（4个）- 平面演示专用配置
    glm::vec3 pointLightColors[] = {
        glm::vec3(1.0f, 0.0f, 0.0f),  // 🔴 纯红色
        glm::vec3(0.0f, 1.0f, 0.0f),  // 🟢 纯绿色
        glm::vec3(0.0f, 0.0f, 1.0f),  // 🔵 纯蓝色
        glm::vec3(1.0f, 1.0f, 0.0f)   // 🟡 纯黄色
    };

    float radius = 8.0f;  // ✅ 适中的旋转半径
    float height = 6.0f;  // ✅ 降低高度，更靠近平面
    outRotatingLights.clear();

    for (int i = 0; i < 4; ++i)
    {
        float angle = i * glm::two_pi<float>() / 4.0f;
        glm::vec3 pos(
            std::cos(angle) * radius,
            height,
            std::sin(angle) * radius
        );

        auto pointLight = std::make_shared<Renderer::Lighting::PointLight>(
            pos,
            pointLightColors[i],
            8.0f,                              // ✅ 非常强的强度，效果明显
            0.0f, 0.0f, 1.0f,                  // ✅ 无环境光，只有漫反射和镜面
            Renderer::Lighting::PointLight::Attenuation::Range20()  // ✅ 20米范围
        );
        lightManager.AddPointLight(pointLight);
        outRotatingLights.push_back(pointLight);
        Core::Logger::GetInstance().Info("✓ Added rotating point light " + std::to_string(i) +
                                         " at (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ", " + std::to_string(pos.z) + ")");
    }

    // 3. 手电筒（聚光灯）- 跟随相机
    outFlashlight = std::make_shared<Renderer::Lighting::SpotLight>(
        glm::vec3(0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        2.0f,                              // 手电筒强度
        0.0f, 0.9f, 1.0f,                  // 光照分量
        Renderer::Lighting::PointLight::Attenuation::Range50(),
        glm::radians(12.5f),                // 内锥角
        glm::radians(20.0f)                 // 外锥角
    );
    lightManager.AddSpotLight(outFlashlight);
    Core::Logger::GetInstance().Info("✓ Added flashlight (spot light)");

    // 中心点位置（用于旋转动画）
    outCenterPosition = glm::vec3(0.0f, height, 0.0f);

    Core::Logger::GetInstance().Info("========================================");
    Core::Logger::GetInstance().Info("Multi-light configuration:");
    Core::Logger::GetInstance().Info("  - Point light radius: " + std::to_string(radius) + "m");
    Core::Logger::GetInstance().Info("  - Point light height: " + std::to_string(height) + "m");
    Core::Logger::GetInstance().Info("  - Point light intensity: 8.0x");
    Core::Logger::GetInstance().Info("  - Pure RGB colors for maximum visibility");
    Core::Logger::GetInstance().Info("========================================");
    lightManager.PrintAllLights();
    Core::Logger::GetInstance().Info("========================================");
}

// ========================================
// 场景生成函数
// ========================================

/**
 * 场景 0: 多光源演示平面 (Multi-Light Demo Plane)
 * 30x30 白色立方体平面，展示4个旋转的彩色点光源
 */
std::shared_ptr<Renderer::InstanceData> CreateMultiLightDemoPlane()
{
    Core::Logger::GetInstance().Info("Creating Multi-Light Demo Plane...");

    auto instances = std::make_shared<Renderer::InstanceData>();

    // 创建一个30x30的平面网格，立方体紧密排列
    int gridSize = 30;
    float spacing = 1.2f;  // 立方体之间的间距
    float cubeSize = 1.0f;

    // 计算偏移量使网格居中
    float offset = (gridSize * spacing) / 2.0f;

    for (int x = 0; x < gridSize; ++x)
    {
        for (int z = 0; z < gridSize; ++z)
        {
            glm::vec3 position(
                x * spacing - offset,
                0.0f,  // 地面平面
                z * spacing - offset
            );

            glm::vec3 rotation(0.0f, 0.0f, 0.0f);
            glm::vec3 scale(cubeSize);

            // 使用白色，让彩色光源效果更明显
            glm::vec3 color(0.95f, 0.95f, 0.95f);

            instances->Add(position, rotation, scale, color);
        }
    }

    Core::Logger::GetInstance().Info("Multi-Light Demo Plane created: " +
                                     std::to_string(instances->GetCount()) + " cubes");
    return instances;
}

/**
 * 场景 1: 垂直立方体墙 (Vertical Cube Wall)
 * 创建一面垂直的立方体墙，用于展示光源在不同高度的效果
 */
std::shared_ptr<Renderer::InstanceData> CreateVerticalCubeWall()
{
    Core::Logger::GetInstance().Info("Creating Vertical Cube Wall...");

    auto instances = std::make_shared<Renderer::InstanceData>();

    // 创建一面 20宽 x 15高 的墙
    int width = 20;
    int height = 15;
    float spacing = 1.2f;
    float cubeSize = 1.0f;

    float offsetX = (width * spacing) / 2.0f;
    float offsetZ = (height * spacing) / 2.0f;

    for (int x = 0; x < width; ++x)
    {
        for (int y = 0; y < height; ++y)
        {
            glm::vec3 position(
                x * spacing - offsetX,
                y * spacing,  // 从地面向上延伸
                0.0f           // 墙在中心平面
            );

            glm::vec3 rotation(0.0f, 0.0f, 0.0f);
            glm::vec3 scale(cubeSize);

            // 使用不同颜色：底部蓝色，中间绿色，顶部红色
            float t = static_cast<float>(y) / static_cast<float>(height);
            glm::vec3 color(
                t,           // 红色从0到1
                0.5f,        // 绿色固定
                1.0f - t     // 蓝色从1到0
            );

            instances->Add(position, rotation, scale, color);
        }
    }

    Core::Logger::GetInstance().Info("Vertical Cube Wall created: " +
                                     std::to_string(instances->GetCount()) + " cubes");
    return instances;
}

/**
 * 场景 2: 球形立方体阵列 (Sphere of Cubes)
 * 创建球形分布的立方体，展示全方位光照效果
 */
std::shared_ptr<Renderer::InstanceData> CreateSphereOfCubes()
{
    Core::Logger::GetInstance().Info("Creating Sphere of Cubes...");

    auto instances = std::make_shared<Renderer::InstanceData>();

    // 斐波那契球面算法
    int numPoints = 400;
    float radius = 10.0f;
    float goldenRatio = (1.0f + std::sqrt(5.0f)) / 2.0f;

    for (int i = 0; i < numPoints; ++i)
    {
        float theta = 2.0f * glm::pi<float>() * i / goldenRatio;
        float phi = std::acos(1.0f - 2.0f * (i + 0.5f) / numPoints);

        glm::vec3 position(
            radius * std::sin(phi) * std::cos(theta),
            radius * std::cos(phi) + radius,  // 抬高，使球体在地面之上
            radius * std::sin(phi) * std::sin(theta)
        );

        glm::vec3 rotation(0.0f, 0.0f, 0.0f);
        glm::vec3 scale(0.8f);

        // 根据位置着色
        glm::vec3 color(
            (position.x + radius) / (2.0f * radius),
            (position.y) / (2.0f * radius),
            (position.z + radius) / (2.0f * radius)
        );

        instances->Add(position, rotation, scale, color);
    }

    Core::Logger::GetInstance().Info("Sphere of Cubes created: " +
                                     std::to_string(instances->GetCount()) + " cubes");
    return instances;
}

/**
 * 场景 3: 隧道 (Cube Tunnel)
 * 创建一个立方体隧道，展示光源在封闭空间内的效果
 */
std::shared_ptr<Renderer::InstanceData> CreateCubeTunnel()
{
    Core::Logger::GetInstance().Info("Creating Cube Tunnel...");

    auto instances = std::make_shared<Renderer::InstanceData>();

    int segments = 12;      // 隧道段数
    float segmentLength = 3.0f;
    float tunnelRadius = 5.0f;
    int cubesPerRing = 16;  // 每圈的立方体数

    for (int seg = 0; seg < segments; ++seg)
    {
        float z = seg * segmentLength;

        for (int i = 0; i < cubesPerRing; ++i)
        {
            float angle = (i / static_cast<float>(cubesPerRing)) * glm::two_pi<float>();

            // 创建圆形截面
            glm::vec3 position(
                std::cos(angle) * tunnelRadius,
                std::sin(angle) * tunnelRadius,
                z
            );

            glm::vec3 rotation(
                0.0f,
                -glm::degrees(angle),
                0.0f
            );
            glm::vec3 scale(1.0f, 1.0f, 0.5f);  // 扁平的立方体

            // 彩虹色渐变
            float t = static_cast<float>(seg) / static_cast<float>(segments);
            glm::vec3 color(
                std::sin(t * glm::two_pi<float>()) * 0.5f + 0.5f,
                std::sin(t * glm::two_pi<float>() + 2.0f) * 0.5f + 0.5f,
                std::sin(t * glm::two_pi<float>() + 4.0f) * 0.5f + 0.5f
            );

            instances->Add(position, rotation, scale, color);
        }
    }

    Core::Logger::GetInstance().Info("Cube Tunnel created: " +
                                     std::to_string(instances->GetCount()) + " cubes");
    return instances;
}

/**
 * 场景 4: 同心圆环 (Concentric Rings)
 * 创建多个同心圆环，展示距离对光照衰减的影响
 */
std::shared_ptr<Renderer::InstanceData> CreateConcentricRings()
{
    Core::Logger::GetInstance().Info("Creating Concentric Rings...");

    auto instances = std::make_shared<Renderer::InstanceData>();

    int numRings = 6;
    int cubesPerRing = 32;
    float startRadius = 3.0f;
    float ringSpacing = 2.5f;

    for (int ring = 0; ring < numRings; ++ring)
    {
        float radius = startRadius + ring * ringSpacing;

        for (int i = 0; i < cubesPerRing; ++i)
        {
            float angle = (i / static_cast<float>(cubesPerRing)) * glm::two_pi<float>();

            glm::vec3 position(
                std::cos(angle) * radius,
                0.0f,
                std::sin(angle) * radius
            );

            glm::vec3 rotation(0.0f, 0.0f, 0.0f);
            glm::vec3 scale(1.0f);

            // 白色，便于观察光照衰减
            glm::vec3 color(0.9f, 0.9f, 0.9f);

            instances->Add(position, rotation, scale, color);
        }
    }

    Core::Logger::GetInstance().Info("Concentric Rings created: " +
                                     std::to_string(instances->GetCount()) + " cubes");
    return instances;
}

/**
 * 场景 5: 几何体展示场 (Geometry Showcase)
 * 展示不同几何体的旋转阵列（使用立方体）
 * 注意：由于InstancedRenderer的限制，每个场景只能使用一种几何体
 * 真正的混合几何体场景需要多个渲染器
 */
std::shared_ptr<Renderer::InstanceData> CreateGeometryShowcase()
{
    Core::Logger::GetInstance().Info("Creating Geometry Showcase...");

    auto instances = std::make_shared<Renderer::InstanceData>();

    // 创建一个大的立方体环形阵列，类似几何体展示
    int numObjects = 20;
    float radius = 10.0f;

    for (int i = 0; i < numObjects; ++i)
    {
        float angle = (i / static_cast<float>(numObjects)) * glm::two_pi<float>();

        glm::vec3 position(
            std::cos(angle) * radius,
            1.0f,
            std::sin(angle) * radius
        );

        glm::vec3 rotation(0.0f, glm::degrees(angle), 0.0f);
        glm::vec3 scale(1.0f);

        // 彩虹色渐变
        glm::vec3 color(
            std::sin(angle) * 0.5f + 0.5f,
            std::sin(angle + 2.0f) * 0.5f + 0.5f,
            std::sin(angle + 4.0f) * 0.5f + 0.5f
        );

        instances->Add(position, rotation, scale, color);
    }

    Core::Logger::GetInstance().Info("Geometry Showcase created: " +
                                     std::to_string(instances->GetCount()) + " cubes");
    return instances;
}

/**
 * 场景: 混合几何体对比 (Mixed Geometry Comparison)
 * 展示球体、平面、圆环体
 */
struct MixedGeometryScene
{
    std::vector<std::unique_ptr<Renderer::InstancedRenderer>> renderers;
    std::vector<std::shared_ptr<Renderer::MeshBuffer>> meshBuffers;
    std::vector<std::shared_ptr<Renderer::InstanceData>> instanceDataList;
};

MixedGeometryScene CreateMixedGeometryScene()
{
    Core::Logger::GetInstance().Info("Creating Mixed Geometry Scene (Spheres, Planes, Tori)...");

    MixedGeometryScene scene;

    // ========================================
    // 创建球体实例（3行3列）
    // ========================================
    auto sphereInstances = std::make_shared<Renderer::InstanceData>();

    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            glm::vec3 position(-6.0f + col * 6.0f, 1.0f, -8.0f + row * 8.0f);
            glm::vec3 rotation(0.0f, 0.0f, 0.0f);
            glm::vec3 scale(1.2f);

            // 不同颜色
            glm::vec3 color;
            if (row == 0)
                color = glm::vec3(0.2f, 0.6f, 1.0f);  // 蓝色
            else if (row == 1)
                color = glm::vec3(0.2f, 1.0f, 0.4f);  // 绿色
            else
                color = glm::vec3(1.0f, 0.6f, 0.2f);  // 橙色

            sphereInstances->Add(position, rotation, scale, color);
        }
    }

    // ========================================
    // 创建平面实例（3个平面，沿X轴排列）
    // ========================================
    auto planeInstances = std::make_shared<Renderer::InstanceData>();

    // 平面1：左侧，红色
    planeInstances->Add(
        glm::vec3(-15.0f, 0.0f, 0.0f),
        glm::vec3(-90.0f, 0.0f, 0.0f),  // 旋转使其竖立
        glm::vec3(4.0f, 4.0f, 1.0f),
        glm::vec3(1.0f, 0.3f, 0.3f)
    );

    // 平面2：中间，黄色
    planeInstances->Add(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(-90.0f, 0.0f, 0.0f),
        glm::vec3(4.0f, 4.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 0.3f)
    );

    // 平面3：右侧，紫色
    planeInstances->Add(
        glm::vec3(15.0f, 0.0f, 0.0f),
        glm::vec3(-90.0f, 0.0f, 0.0f),
        glm::vec3(4.0f, 4.0f, 1.0f),
        glm::vec3(0.8f, 0.3f, 1.0f)
    );

    // ========================================
    // 创建圆环体实例（2行2列）
    // ========================================
    auto torusInstances = std::make_shared<Renderer::InstanceData>();

    for (int row = 0; row < 2; ++row)
    {
        for (int col = 0; col < 2; ++col)
        {
            glm::vec3 position(-4.5f + col * 9.0f, 2.5f, 12.0f + row * 6.0f);
            glm::vec3 rotation(90.0f, 0.0f, 0.0f);  // 平躺
            glm::vec3 scale(1.5f);

            // 青色和品红色交替
            glm::vec3 color = (row + col) % 2 == 0 ?
                glm::vec3(0.3f, 1.0f, 1.0f) :  // 青色
                glm::vec3(1.0f, 0.3f, 1.0f);  // 品红

            torusInstances->Add(position, rotation, scale, color);
        }
    }

    // ========================================
    // 创建球体渲染器
    // ========================================
    Core::Logger::GetInstance().Info("Creating sphere renderer...");

    try
    {
        Renderer::MeshBuffer sphereMesh = Renderer::MeshBufferFactory::CreateSphereBuffer(32, 32, 1.0f);
        auto sphereMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(sphereMesh));
        scene.meshBuffers.push_back(sphereMeshPtr);

        auto sphereRenderer = std::make_unique<Renderer::InstancedRenderer>();
        sphereRenderer->SetMesh(sphereMeshPtr);
        sphereRenderer->SetInstances(sphereInstances);
        sphereRenderer->Initialize();
        scene.renderers.push_back(std::move(sphereRenderer));
        scene.instanceDataList.push_back(sphereInstances);

        Core::Logger::GetInstance().Info("Sphere renderer created successfully");
    }
    catch (const std::exception& e)
    {
        Core::Logger::GetInstance().Error("Failed to create sphere renderer: " + std::string(e.what()));
    }

    // ========================================
    // 创建平面渲染器
    // ========================================
    Core::Logger::GetInstance().Info("Creating plane renderer...");

    try
    {
        Renderer::MeshBuffer planeMesh = Renderer::MeshBufferFactory::CreatePlaneBuffer(1.0f, 1.0f, 1, 1);
        auto planeMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(planeMesh));
        scene.meshBuffers.push_back(planeMeshPtr);

        auto planeRenderer = std::make_unique<Renderer::InstancedRenderer>();
        planeRenderer->SetMesh(planeMeshPtr);
        planeRenderer->SetInstances(planeInstances);
        planeRenderer->Initialize();
        scene.renderers.push_back(std::move(planeRenderer));
        scene.instanceDataList.push_back(planeInstances);

        Core::Logger::GetInstance().Info("Plane renderer created successfully");
    }
    catch (const std::exception& e)
    {
        Core::Logger::GetInstance().Error("Failed to create plane renderer: " + std::string(e.what()));
    }

    // ========================================
    // 创建圆环体渲染器
    // ========================================
    Core::Logger::GetInstance().Info("Creating torus renderer...");

    try
    {
        Renderer::MeshBuffer torusMesh = Renderer::MeshBufferFactory::CreateTorusBuffer(1.0f, 0.3f, 32, 24);
        auto torusMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(torusMesh));
        scene.meshBuffers.push_back(torusMeshPtr);

        auto torusRenderer = std::make_unique<Renderer::InstancedRenderer>();
        torusRenderer->SetMesh(torusMeshPtr);
        torusRenderer->SetInstances(torusInstances);
        torusRenderer->Initialize();
        scene.renderers.push_back(std::move(torusRenderer));
        scene.instanceDataList.push_back(torusInstances);

        Core::Logger::GetInstance().Info("Torus renderer created successfully");
    }
    catch (const std::exception& e)
    {
        Core::Logger::GetInstance().Error("Failed to create torus renderer: " + std::string(e.what()));
    }

    Core::Logger::GetInstance().Info("Mixed Geometry Scene created: " +
                                     std::to_string(scene.renderers.size()) + " renderer types, " +
                                     std::to_string(sphereInstances->GetCount()) + " spheres, " +
                                     std::to_string(planeInstances->GetCount()) + " planes, " +
                                     std::to_string(torusInstances->GetCount()) + " tori");

    return scene;
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
        Core::LogLevel::WARNING,  // ✅ 改为WARNING级别，减少INFO输出
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
        // 初始化输入控制器和摄像机
        // ========================================
        Core::Logger::GetInstance().Info("Initializing input controllers and camera...");

        // 创建摄像机
        Core::Camera camera(
            glm::vec3(0.0f, 12.0f, 25.0f),  // ✅ 适合观察平面演示的位置
            glm::vec3(0.0f, 1.0f, 0.0f),    // 世界上向量
            -90.0f,                          // 初始偏航角
            -30.0f                           // ✅ 向下看的俯仰角
        );

        Core::MouseController mouseController;
        mouseController.Initialize(glfwGetCurrentContext());
        mouseController.SetMouseCapture(true);

        // 设置鼠标移动回调来更新摄像机方向
        glfwSetCursorPosCallback(glfwGetCurrentContext(), [](GLFWwindow* window, double xpos, double ypos) {
            static bool firstMouse = true;
            static float lastX = WINDOW_WIDTH / 2.0f;
            static float lastY = WINDOW_HEIGHT / 2.0f;

            // 检查鼠标是否被捕获
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
            float yoffset = lastY - static_cast<float>(ypos); // 反转Y轴

            lastX = static_cast<float>(xpos);
            lastY = static_cast<float>(ypos);

            // 从窗口用户指针获取摄像机
            Core::Camera* cam = static_cast<Core::Camera*>(glfwGetWindowUserPointer(window));
            if (cam)
            {
                cam->ProcessMouseMovement(xoffset, yoffset);
            }
        });

        // 设置滚轮回调来调整FOV
        glfwSetScrollCallback(glfwGetCurrentContext(), [](GLFWwindow* window, double xoffset, double yoffset) {
            Core::Camera* cam = static_cast<Core::Camera*>(glfwGetWindowUserPointer(window));
            if (cam)
            {
                cam->ProcessMouseScroll(static_cast<float>(yoffset));
            }
        });

        // 设置窗口用户指针，使回调可以访问摄像机
        glfwSetWindowUserPointer(glfwGetCurrentContext(), &camera);

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
        // 初始化多光源系统
        // ========================================
        std::vector<Renderer::Lighting::PointLightPtr> rotatingPointLights;
        Renderer::Lighting::SpotLightPtr flashlight;
        glm::vec3 centerPosition(0.0f, 0.0f, 0.0f);
        SetupLighting(rotatingPointLights, flashlight, centerPosition);

        // ========================================
        // 加载着色器
        // ========================================
        Core::Logger::GetInstance().Info("Loading shaders...");
        Renderer::Shader multiLightShader;
        multiLightShader.Load("assets/shader/multi_light.vert", "assets/shader/multi_light.frag");

        // ========================================
        // 创建混合几何体场景
        // ========================================
        MixedGeometryScene mixedGeometryScene = CreateMixedGeometryScene();

        // 光源控制回调
        bool animateLights = true;

        keyboardController.RegisterKeyCallback(GLFW_KEY_SPACE, [&animateLights]()
        {
            animateLights = !animateLights;
            Core::Logger::GetInstance().Info("Light animation " + std::string(animateLights ? "resumed" : "paused"));
        });

        Core::Logger::GetInstance().Info("========================================");
        Core::Logger::GetInstance().Info("Mixed Geometry Scene loaded successfully!");
        Core::Logger::GetInstance().Info("Total renderers: " + std::to_string(mixedGeometryScene.renderers.size()));
        Core::Logger::GetInstance().Info("========================================");
        Core::Logger::GetInstance().Info("Controls:");
        Core::Logger::GetInstance().Info("  WASD - Move camera");
        Core::Logger::GetInstance().Info("  Q/E  - Move up/down");
        Core::Logger::GetInstance().Info("  Mouse - Look around");
        Core::Logger::GetInstance().Info("  TAB  - Toggle mouse capture");
        Core::Logger::GetInstance().Info("  SPACE - Pause/Resume light animation");
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
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);

        // ========================================
        // 渲染循环
        // ========================================
        Core::Logger::GetInstance().Info("Starting render loop...");

        double lastTime = glfwGetTime();
        double fpsLastTime = glfwGetTime();
        int fpsFrameCount = 0;
        int totalFrameCount = 0;
        float rotationAngle = 0.0f;
        bool animationPaused = false;

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
                    std::string logMessage = "Mixed Geometry | FPS: " +
                                             std::to_string(static_cast<int>(fps)) +
                                             " | Total Frames: " +
                                             std::to_string(totalFrameCount);
                    Core::Logger::GetInstance().Info(logMessage);
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
            // 更新光源
            // ========================================
            if (!animationPaused)
            {
                // 更新旋转的点光源位置
                float time = static_cast<float>(glfwGetTime());
                for (size_t i = 0; i < rotatingPointLights.size(); ++i)
                {
                    float angleOffset = static_cast<float>(i) * glm::two_pi<float>() / 4.0f;
                    float radius = 8.0f;  // ✅ 与SetupLighting中的半径一致
                    float speed = 1.0f;   // ✅ 适中的旋转速度

                    // 水平圆形旋转，高度固定
                    glm::vec3 offset(
                        std::sin(time * speed + angleOffset) * radius,
                        0.0f,  // ✅ 不再上下移动，保持在固定高度
                        std::cos(time * speed + angleOffset) * radius
                    );
                    rotatingPointLights[i]->SetPosition(centerPosition + offset);
                }

                // 更新手电筒位置和方向
                if (flashlight)
                {
                    flashlight->SetPosition(camera.GetPosition());
                    flashlight->SetDirection(camera.GetFront());
                }
            }

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
            // 渲染设置
            // ========================================
            float aspectRatio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
            glm::mat4 projection = camera.GetProjectionMatrix(aspectRatio, 0.1f, 300.0f);
            glm::mat4 view = camera.GetViewMatrix();

            // 设置日志上下文
            Core::LogContext renderContext;
            renderContext.renderPass = "MixedGeometry";
            renderContext.batchIndex = 0;
            renderContext.drawCallCount = 1;
            renderContext.currentShader = "MultiLightShader";
            Core::Logger::GetInstance().SetContext(renderContext);

            // 清空缓冲区
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // ========================================
            // 设置着色器参数
            // ========================================
            multiLightShader.Use();
            multiLightShader.SetMat4("projection", projection);
            multiLightShader.SetMat4("view", view);
            multiLightShader.SetVec3("viewPos", camera.GetPosition());
            multiLightShader.SetBool("useInstanceColor", true);
            multiLightShader.SetBool("useTexture", false);
            multiLightShader.SetFloat("shininess", 64.0f);
            multiLightShader.SetFloat("time", static_cast<float>(currentTime));

            // 应用所有光源
            Renderer::Lighting::LightManager::GetInstance().ApplyToShader(multiLightShader);

            // ========================================
            // 渲染混合几何体场景
            // ========================================

            static bool firstRender = true;
            if (firstRender)
            {
                Core::Logger::GetInstance().Info("Rendering Mixed Geometry Scene with " +
                                                 std::to_string(mixedGeometryScene.renderers.size()) + " renderers");
                firstRender = false;
            }

            // 渲染所有渲染器
            for (const auto& renderer : mixedGeometryScene.renderers)
            {
                renderer->Render();
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
