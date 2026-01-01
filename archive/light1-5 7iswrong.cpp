// ！！！注意里面的场景实现，这种切换才是正确的，之前的场景切换有误
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
const char* WINDOW_TITLE = "Multi-Light Demo - 1-7: Scenes | SPACE: Pause Animation";

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
 * 场景 6: 混合几何体对比 (Mixed Geometry Comparison)
 * 使用不同的几何体类型，展示它们在相同光照下的效果
 * 这个场景会创建多个独立的渲染器，每个渲染器使用不同的几何体
 */
struct MixedGeometryScene
{
    std::vector<std::unique_ptr<Renderer::InstancedRenderer>> renderers;
    std::vector<std::shared_ptr<Renderer::MeshBuffer>> meshBuffers;
    std::vector<std::shared_ptr<Renderer::InstanceData>> instanceDataList;
};

MixedGeometryScene CreateMixedGeometryScene()
{
    Core::Logger::GetInstance().Info("Creating Mixed Geometry Scene...");

    MixedGeometryScene scene;

    // ========================================
    // 1. 创建球体实例
    // ========================================
    auto sphereInstances = std::make_shared<Renderer::InstanceData>();
    for (int i = 0; i < 3; ++i)
    {
        glm::vec3 position(-6.0f + i * 6.0f, 1.0f, -8.0f);
        glm::vec3 rotation(0.0f, 0.0f, 0.0f);
        glm::vec3 scale(1.2f);
        glm::vec3 color(0.2f, 0.6f, 1.0f); // 蓝色
        sphereInstances->Add(position, rotation, scale, color);
    }

    // ========================================
    // 2. 创建圆柱体实例
    // ========================================
    auto cylinderInstances = std::make_shared<Renderer::InstanceData>();
    for (int i = 0; i < 3; ++i)
    {
        glm::vec3 position(-6.0f + i * 6.0f, 1.5f, 0.0f);
        glm::vec3 rotation(0.0f, 0.0f, 0.0f);
        glm::vec3 scale(1.0f);
        glm::vec3 color(0.2f, 1.0f, 0.4f); // 绿色
        cylinderInstances->Add(position, rotation, scale, color);
    }

    // ========================================
    // 3. 创建圆锥体实例
    // ========================================
    auto coneInstances = std::make_shared<Renderer::InstanceData>();
    for (int i = 0; i < 3; ++i)
    {
        glm::vec3 position(-6.0f + i * 6.0f, 1.5f, 8.0f);
        glm::vec3 rotation(0.0f, 0.0f, 0.0f);
        glm::vec3 scale(1.0f);
        glm::vec3 color(1.0f, 0.6f, 0.2f); // 橙色
        coneInstances->Add(position, rotation, scale, color);
    }

    // ========================================
    // 创建球体渲染器
    // ========================================
    Renderer::MeshBuffer sphereMesh = Renderer::MeshBufferFactory::CreateSphereBuffer(32, 32, 1.0f);
    auto sphereMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(sphereMesh));
    scene.meshBuffers.push_back(sphereMeshPtr);

    auto sphereRenderer = std::make_unique<Renderer::InstancedRenderer>();
    sphereRenderer->SetMesh(sphereMeshPtr);
    sphereRenderer->SetInstances(sphereInstances);
    sphereRenderer->Initialize();
    scene.renderers.push_back(std::move(sphereRenderer));
    scene.instanceDataList.push_back(sphereInstances);

    // ========================================
    // 创建圆柱体渲染器
    // ========================================
    Renderer::MeshBuffer cylinderMesh = Renderer::MeshBufferFactory::CreateCylinderBuffer(1.0f, 2.0f, 32);
    auto cylinderMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(cylinderMesh));
    scene.meshBuffers.push_back(cylinderMeshPtr);

    auto cylinderRenderer = std::make_unique<Renderer::InstancedRenderer>();
    cylinderRenderer->SetMesh(cylinderMeshPtr);
    cylinderRenderer->SetInstances(cylinderInstances);
    cylinderRenderer->Initialize();
    scene.renderers.push_back(std::move(cylinderRenderer));
    scene.instanceDataList.push_back(cylinderInstances);

    // ========================================
    // 创建圆锥体渲染器
    // ========================================
    Renderer::MeshBuffer coneMesh = Renderer::MeshBufferFactory::CreateConeBuffer(1.0f, 2.0f, 32);
    auto coneMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(coneMesh));
    scene.meshBuffers.push_back(coneMeshPtr);

    auto coneRenderer = std::make_unique<Renderer::InstancedRenderer>();
    coneRenderer->SetMesh(coneMeshPtr);
    coneRenderer->SetInstances(coneInstances);
    coneRenderer->Initialize();
    scene.renderers.push_back(std::move(coneRenderer));
    scene.instanceDataList.push_back(coneInstances);

    Core::Logger::GetInstance().Info("Mixed Geometry Scene created: " +
                                     std::to_string(scene.renderers.size()) + " renderer types, " +
                                     std::to_string(sphereInstances->GetCount() + cylinderInstances->GetCount() + coneInstances->GetCount()) + " total objects");

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
        // 创建场景
        // ========================================
        std::vector<std::shared_ptr<Renderer::InstanceData>> scenes;
        scenes.push_back(CreateMultiLightDemoPlane());     // 场景 0: 多光源演示平面
        scenes.push_back(CreateVerticalCubeWall());        // 场景 1: 垂直立方体墙
        scenes.push_back(CreateSphereOfCubes());           // 场景 2: 球形立方体阵列
        scenes.push_back(CreateCubeTunnel());              // 场景 3: 隧道
        scenes.push_back(CreateConcentricRings());         // 场景 4: 同心圆环
        scenes.push_back(CreateGeometryShowcase());        // 场景 5: 几何体展示场

        int currentScene = 0;

        // ========================================
        // 创建混合几何体场景（特殊场景）
        // ========================================
        MixedGeometryScene mixedGeometryScene = CreateMixedGeometryScene();

        // ========================================
        // 创建渲染器
        // ========================================
        Core::Logger::GetInstance().Info("Creating instanced renderers...");

        std::vector<std::unique_ptr<Renderer::InstancedRenderer>> renderers;
        std::vector<std::shared_ptr<Renderer::MeshBuffer>> meshBuffers;  // 保持mesh存活

        // 为每个场景创建独立的 MeshBuffer 和渲染器
        for (size_t i = 0; i < scenes.size(); ++i)
        {
            // 为每个场景创建独立的 MeshBuffer（避免共享VAO导致实例属性冲突）
            Renderer::MeshBuffer cubeMesh = Renderer::MeshBufferFactory::CreateCubeBuffer();
            auto cubeMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(cubeMesh));
            meshBuffers.push_back(cubeMeshPtr);

            auto renderer = std::make_unique<Renderer::InstancedRenderer>();
            renderer->SetMesh(cubeMeshPtr);
            renderer->SetInstances(scenes[i]);
            renderer->Initialize();
            renderers.push_back(std::move(renderer));

            Core::Logger::GetInstance().Info("Scene " + std::to_string(i + 1) + " created with " +
                                             std::to_string(scenes[i]->GetCount()) + " instances");
        }

        // 场景切换回调
        keyboardController.RegisterKeyCallback(GLFW_KEY_1, [&currentScene, &camera]()
        {
            currentScene = 0;
            camera.SetPosition(glm::vec3(0.0f, 12.0f, 25.0f));
            Core::Logger::GetInstance().Info("Switched to Scene 1: Multi-Light Demo Plane (30x30 plane)");
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_2, [&currentScene, &camera]()
        {
            currentScene = 1;
            camera.SetPosition(glm::vec3(0.0f, 10.0f, 20.0f));
            Core::Logger::GetInstance().Info("Switched to Scene 2: Vertical Cube Wall (20x15 wall)");
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_3, [&currentScene, &camera]()
        {
            currentScene = 2;
            camera.SetPosition(glm::vec3(0.0f, 12.0f, 25.0f));
            Core::Logger::GetInstance().Info("Switched to Scene 3: Sphere of Cubes (400 cubes)");
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_4, [&currentScene, &camera]()
        {
            currentScene = 3;
            camera.SetPosition(glm::vec3(0.0f, 5.0f, -5.0f));
            Core::Logger::GetInstance().Info("Switched to Scene 4: Cube Tunnel (12 segments)");
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_5, [&currentScene, &camera]()
        {
            currentScene = 4;
            camera.SetPosition(glm::vec3(0.0f, 20.0f, 0.0f));
            Core::Logger::GetInstance().Info("Switched to Scene 5: Concentric Rings (6 rings)");
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_6, [&currentScene, &camera]()
        {
            currentScene = 5;
            camera.SetPosition(glm::vec3(0.0f, 5.0f, 15.0f));
            Core::Logger::GetInstance().Info("Switched to Scene 6: Geometry Showcase (20 objects)");
        });

        keyboardController.RegisterKeyCallback(GLFW_KEY_7, [&currentScene, &camera]()
        {
            currentScene = 6;
            camera.SetPosition(glm::vec3(0.0f, 3.0f, 12.0f));
            Core::Logger::GetInstance().Info("Switched to Scene 7: Mixed Geometry (9 objects - Sphere, Cylinder, Cone)");
        });

        // 光源控制回调
        bool animateLights = true;

        keyboardController.RegisterKeyCallback(GLFW_KEY_SPACE, [&animateLights]()
        {
            animateLights = !animateLights;
            Core::Logger::GetInstance().Info("Light animation " + std::string(animateLights ? "resumed" : "paused"));
        });

        Core::Logger::GetInstance().Info("========================================");
        Core::Logger::GetInstance().Info("Multi-light demo scenes loaded successfully!");
        Core::Logger::GetInstance().Info("Total scenes: " + std::to_string(scenes.size() + 1)); // +1 for mixed geometry
        Core::Logger::GetInstance().Info("========================================");
        Core::Logger::GetInstance().Info("Controls:");
        Core::Logger::GetInstance().Info("  WASD - Move camera");
        Core::Logger::GetInstance().Info("  Q/E  - Move up/down");
        Core::Logger::GetInstance().Info("  Mouse - Look around");
        Core::Logger::GetInstance().Info("  TAB  - Toggle mouse capture");
        Core::Logger::GetInstance().Info("  1-7  - Switch scenes");
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
            // 渲染当前场景
            // ========================================

            // ✅ 添加场景渲染的调试信息
            static int lastScene = -1;
            if (lastScene != currentScene)
            {
                if (currentScene == 6) // 混合几何体场景
                {
                    Core::Logger::GetInstance().Info("Rendering scene 7 (Mixed Geometry) with " +
                                                     std::to_string(mixedGeometryScene.renderers.size()) + " renderer types");
                }
                else
                {
                    Core::Logger::GetInstance().Info("Rendering scene " + std::to_string(currentScene + 1) +
                                                     " with " + std::to_string(scenes[currentScene]->GetCount()) + " instances");
                }
                lastScene = currentScene;
            }

            // 场景6是特殊的混合几何体场景，需要渲染多个渲染器
            if (currentScene == 6)
            {
                for (const auto& renderer : mixedGeometryScene.renderers)
                {
                    renderer->Render();
                }
            }
            else
            {
                renderers[currentScene]->Render();
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
