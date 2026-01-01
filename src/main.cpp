/**
 * ========================================
 * Disco 舞台演示 - Disco Stage Demo (Enhanced with Skybox)
 * ========================================
 *
 * 超级Disco舞台：水平平面 + 超大中心球体 + 48个彩色光源 + 天空盒环境光照
 *
 * 特性：
 * - 天空盒背景渲染
 * - 轻量级IBL环境光照（从天空盒采样）
 * - 水平平面舞台（50x50纯白色地板）
 * - 中央巨型Disco球：800个立方体外层（边长0.4，分布半径4.0）+ 核心球体（半径3.0）
 * - 8个彩色球形灯：每个100个立方体外层（边长0.2，分布半径1.0）+ 核心球体（半径1.0）
 * - 48个混乱旋转彩色点光源（三层布局，覆盖整个舞台）
 * - 中央聚光灯（窄光束向下照射地板）
 * - 动态光源旋转效果
 * - 实时光照计算
 *
 * 光源系统（超级Disco风格）：
 * - 48个点光源分三层圆形布局
 *   * 内圈（16个）：半径7-9米，高度3-4米，强度10x，范围13米
 *   * 中圈（16个）：半径12.5-15.5米，高度4.5-5.5米，强度12x，范围32米
 *   * 外圈（16个）：半径18-22米，高度6-7米，强度15x，范围50米
 * - 48种不同颜色：基础色、亮色变体、深色变体
 * - 角度错开布局：内圈0°，中圈11.25°，外圈22.5°
 * - 覆盖范围：22米半径（整个舞台）
 *
 * 中心球体（增强版）：
 * - 立方体数量：800个（原来500个）
 * - 立方体大小：0.4米（原来0.35米）
 * - 分布半径：4.0米（原来2.5米）
 * - 核心球体半径：3.0米（原来1.8米）
 * - 整体尺寸增加60%，视觉冲击力更强
 *
 * 技术特点：
 * - 斐波那契球面算法确保立方体均匀分布
 * - 统一立方体大小：中央0.4，周边0.2
 * - 核心球体半径略小于立方体层，形成内部核心
 * - 立方体附着在核心球体表面，形成镜面反射层
 * - 总计1600个立方体 + 9个核心球体
 * - 3个渲染器（平面、立方体、球体）
 * - 48个混乱旋转彩色点光源，营造超级Disco氛围
 * - 天空盒环境光照，物体暗部显示天空颜色
 *
 * 控制说明：
 * - WASD: 前后左右移动
 * - Q/E: 上下移动
 * - 鼠标: 旋转视角
 * - TAB: 切换鼠标捕获
 * - ESC: 退出
 * - SPACE: 暂停/恢复光源动画
 * - 1/2/3: 切换环境光模式（固定颜色/天空盒采样/半球光照）
 * - [/]: 调整环境光强度
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
#include "Renderer/Environment/Skybox.hpp"
#include "Renderer/Environment/AmbientLighting.hpp"

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
const char *WINDOW_TITLE = "Super Disco Stage + Skybox | Space:Pause 1/2/3:AmbMode [/]:Intensity";

// 性能统计
float fps = 0.0f;
int totalFrames = 0;

// 全局变量（用于键盘控制）
Renderer::AmbientLighting::Mode g_currentAmbientMode = Renderer::AmbientLighting::Mode::SKYBOX_SAMPLE;
float g_ambientIntensity = 0.3f;

// ========================================
// 场景生成函数
// ========================================

/**
 * 初始化多光源系统
 */
void SetupLighting(
    std::vector<Renderer::Lighting::PointLightPtr> &outRotatingLights,
    Renderer::Lighting::SpotLightPtr &outFlashlight,
    glm::vec3 &outCenterPosition)
{
    auto &lightManager = Renderer::Lighting::LightManager::GetInstance();

    Core::Logger::GetInstance().Info("========================================");
    Core::Logger::GetInstance().Info("Setting up multi-light system...");
    Core::Logger::GetInstance().Info("========================================");

    // 1. 太阳光（平行光）- 弱化，仅提供基础照明
    auto sun = std::make_shared<Renderer::Lighting::DirectionalLight>(
        glm::vec3(0.0f, -1.0f, 0.0f), // 方向：从正上方往下
        glm::vec3(1.0f, 1.0f, 1.0f),  // 纯白色
        0.3f,                         // 大幅降低强度
        0.05f, 0.2f, 0.1f             // 非常低的环境光、漫反射和镜面
    );
    lightManager.AddDirectionalLight(sun);
    Core::Logger::GetInstance().Info("✓ Added weak sun (directional light) from above");

    // 2. 彩色点光源阵列（48个）- Disco舞台专用配置，布满整个舞台
    // 使用更多样化的颜色混合，分三层布局
    glm::vec3 pointLightColors[] = {
        // 第一层：基础颜色（16个）
        glm::vec3(1.0f, 0.0f, 0.0f), // 纯红
        glm::vec3(0.0f, 1.0f, 0.0f), // 纯绿
        glm::vec3(0.0f, 0.0f, 1.0f), // 纯蓝
        glm::vec3(1.0f, 1.0f, 0.0f), // 黄色
        glm::vec3(1.0f, 0.0f, 1.0f), // 洋红
        glm::vec3(0.0f, 1.0f, 1.0f), // 青色
        glm::vec3(1.0f, 0.5f, 0.0f), // 橙色
        glm::vec3(0.5f, 0.0f, 1.0f), // 紫色
        glm::vec3(1.0f, 0.0f, 0.5f), // 粉红
        glm::vec3(0.0f, 0.5f, 1.0f), // 天蓝
        glm::vec3(0.5f, 1.0f, 0.0f), // 酸橙
        glm::vec3(1.0f, 0.8f, 0.0f), // 金色
        glm::vec3(0.8f, 0.0f, 1.0f), // 薰衣草
        glm::vec3(0.0f, 1.0f, 0.5f), // 绿松石
        glm::vec3(1.0f, 0.5f, 0.5f), // 珊瑚色
        glm::vec3(0.5f, 1.0f, 0.8f), // 薄荷绿
        // 第二层：亮色变体（16个）
        glm::vec3(1.0f, 0.2f, 0.2f), // 亮红
        glm::vec3(0.2f, 1.0f, 0.2f), // 亮绿
        glm::vec3(0.2f, 0.2f, 1.0f), // 亮蓝
        glm::vec3(1.0f, 1.0f, 0.2f), // 亮黄
        glm::vec3(1.0f, 0.2f, 1.0f), // 亮洋红
        glm::vec3(0.2f, 1.0f, 1.0f), // 亮青
        glm::vec3(1.0f, 0.6f, 0.2f), // 亮橙
        glm::vec3(0.6f, 0.2f, 1.0f), // 亮紫
        glm::vec3(1.0f, 0.2f, 0.6f), // 亮粉红
        glm::vec3(0.2f, 0.6f, 1.0f), // 亮天蓝
        glm::vec3(0.6f, 1.0f, 0.2f), // 亮酸橙
        glm::vec3(1.0f, 0.9f, 0.2f), // 亮金色
        glm::vec3(0.9f, 0.2f, 1.0f), // 亮薰衣草
        glm::vec3(0.2f, 1.0f, 0.6f), // 亮绿松石
        glm::vec3(1.0f, 0.6f, 0.6f), // 亮珊瑚色
        glm::vec3(0.6f, 1.0f, 0.9f), // 亮薄荷绿
        // 第三层：深色变体（16个）
        glm::vec3(0.8f, 0.0f, 0.0f), // 深红
        glm::vec3(0.0f, 0.8f, 0.0f), // 深绿
        glm::vec3(0.0f, 0.0f, 0.8f), // 深蓝
        glm::vec3(0.8f, 0.8f, 0.0f), // 深黄
        glm::vec3(0.8f, 0.0f, 0.8f), // 深洋红
        glm::vec3(0.0f, 0.8f, 0.8f), // 深青
        glm::vec3(0.8f, 0.4f, 0.0f), // 深橙
        glm::vec3(0.4f, 0.0f, 0.8f), // 深紫
        glm::vec3(0.8f, 0.0f, 0.4f), // 深粉红
        glm::vec3(0.0f, 0.4f, 0.8f), // 深天蓝
        glm::vec3(0.4f, 0.8f, 0.0f), // 深酸橙
        glm::vec3(0.8f, 0.7f, 0.0f), // 深金色
        glm::vec3(0.7f, 0.0f, 0.8f), // 深薰衣草
        glm::vec3(0.0f, 0.8f, 0.4f), // 深绿松石
        glm::vec3(0.8f, 0.4f, 0.4f), // 深珊瑚色
        glm::vec3(0.4f, 0.8f, 0.7f)  // 深薄荷绿
    };

    outRotatingLights.clear();
    const int totalLights = 48; // 增加到48个光源

    // 第一层：内圈（16个）- 半径8米，高度3-5米
    for (int i = 0; i < 16; ++i)
    {
        float angle = i * glm::two_pi<float>() / 16.0f;
        float radius = 8.0f + (i % 3 - 1) * 1.0f; // 半径变化：7, 8, 9米
        float height = 3.0f + (i % 2) * 1.0f;     // 高度变化：3, 4米

        glm::vec3 pos(
            std::cos(angle) * radius,
            height,
            std::sin(angle) * radius);

        auto pointLight = std::make_shared<Renderer::Lighting::PointLight>(
            pos,
            pointLightColors[i],
            10.0f,                                                 // 增加强度
            0.0f, 0.0f, 1.0f,                                      // 无环境光，只有漫反射和镜面
            Renderer::Lighting::PointLight::Attenuation::Range13() // 13米范围
        );
        lightManager.AddPointLight(pointLight);
        outRotatingLights.push_back(pointLight);
        Core::Logger::GetInstance().Info("✓ Added inner circle point light " + std::to_string(i) +
                                         " at (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ", " + std::to_string(pos.z) + ")");
    }

    // 第二层：中圈（16个）- 半径14米，高度4-6米
    for (int i = 0; i < 16; ++i)
    {
        float angle = i * glm::two_pi<float>() / 16.0f + glm::radians(11.25f); // 错开角度
        float radius = 14.0f + (i % 3 - 1) * 1.5f;                             // 半径变化：12.5, 14, 15.5米
        float height = 4.5f + (i % 2) * 1.0f;                                  // 高度变化：4.5, 5.5米

        glm::vec3 pos(
            std::cos(angle) * radius,
            height,
            std::sin(angle) * radius);

        auto pointLight = std::make_shared<Renderer::Lighting::PointLight>(
            pos,
            pointLightColors[16 + i], // 使用第二层颜色
            12.0f,                    // 更强强度
            0.0f, 0.0f, 1.0f,
            Renderer::Lighting::PointLight::Attenuation::Range32() // 32米范围，覆盖更大区域
        );
        lightManager.AddPointLight(pointLight);
        outRotatingLights.push_back(pointLight);
        Core::Logger::GetInstance().Info("✓ Added middle circle point light " + std::to_string(16 + i) +
                                         " at (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ", " + std::to_string(pos.z) + ")");
    }

    // 第三层：外圈（16个）- 半径20米，高度5-7米
    for (int i = 0; i < 16; ++i)
    {
        float angle = i * glm::two_pi<float>() / 16.0f + glm::radians(22.5f); // 再次错开角度
        float radius = 20.0f + (i % 3 - 1) * 2.0f;                            // 半径变化：18, 20, 22米
        float height = 6.0f + (i % 2) * 1.0f;                                 // 高度变化：6, 7米

        glm::vec3 pos(
            std::cos(angle) * radius,
            height,
            std::sin(angle) * radius);

        auto pointLight = std::make_shared<Renderer::Lighting::PointLight>(
            pos,
            pointLightColors[32 + i], // 使用第三层颜色
            15.0f,                    // 最强强度
            0.0f, 0.0f, 1.0f,
            Renderer::Lighting::PointLight::Attenuation::Range50() // 50米范围，覆盖整个舞台
        );
        lightManager.AddPointLight(pointLight);
        outRotatingLights.push_back(pointLight);
        Core::Logger::GetInstance().Info("✓ Added outer circle point light " + std::to_string(32 + i) +
                                         " at (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ", " + std::to_string(pos.z) + ")");
    }

    // 3. 中央聚光灯 - 从Disco球位置向下照射到地板，缩小光圈
    auto centerSpotlight = std::make_shared<Renderer::Lighting::SpotLight>(
        glm::vec3(0.0f, 8.0f, 0.0f),                            // 位置：与中央Disco球一致
        glm::vec3(0.0f, -1.0f, 0.0f),                           // 方向：向下
        glm::vec3(1.0f, 1.0f, 1.0f),                            // 颜色：白色
        15.0f,                                                  // 强度
        0.0f, 0.0f, 1.0f,                                       // ambient, diffuse, specular
        Renderer::Lighting::PointLight::Attenuation::Range32(), // 32米范围
        glm::radians(15.0f),                                    // 内角：15度，缩小光圈
        glm::radians(25.0f)                                     // 外角：25度，更小的边缘
    );
    lightManager.AddSpotLight(centerSpotlight);
    Core::Logger::GetInstance().Info("✓ Added center spotlight (tight beam from disco ball to floor)");

    // 3. 手电筒（聚光灯）- 跟随相机
    outFlashlight = std::make_shared<Renderer::Lighting::SpotLight>(
        glm::vec3(0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        2.0f,             // 手电筒强度
        0.0f, 0.9f, 1.0f, // 光照分量
        Renderer::Lighting::PointLight::Attenuation::Range50(),
        glm::radians(12.5f), // 内锥角
        glm::radians(20.0f)  // 外锥角
    );
    lightManager.AddSpotLight(outFlashlight);
    Core::Logger::GetInstance().Info("✓ Added flashlight (spot light)");

    // 中心点位置（用于旋转动画）
    outCenterPosition = glm::vec3(0.0f, 4.5f, 0.0f); // 调整为三层平均高度

    Core::Logger::GetInstance().Info("========================================");
    Core::Logger::GetInstance().Info("Multi-light configuration (48 lights):");
    Core::Logger::GetInstance().Info("  - Inner circle: 16 lights @ 7-9m radius, 3-4m height, 10.0x intensity, 13m range");
    Core::Logger::GetInstance().Info("  - Middle circle: 16 lights @ 12.5-15.5m radius, 4.5-5.5m height, 12.0x intensity, 32m range");
    Core::Logger::GetInstance().Info("  - Outer circle: 16 lights @ 18-22m radius, 6-7m height, 15.0x intensity, 50m range");
    Core::Logger::GetInstance().Info("  - Total coverage: 22m radius (entire stage)");
    Core::Logger::GetInstance().Info("  - Color scheme: 48 unique colors (base/bright/dark variants)");
    Core::Logger::GetInstance().Info("  - Chaotic rotation: different speeds, directions, radii");
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
    float spacing = 1.2f; // 立方体之间的间距
    float cubeSize = 1.0f;

    // 计算偏移量使网格居中
    float offset = (gridSize * spacing) / 2.0f;

    for (int x = 0; x < gridSize; ++x)
    {
        for (int z = 0; z < gridSize; ++z)
        {
            glm::vec3 position(
                x * spacing - offset,
                0.0f, // 地面平面
                z * spacing - offset);

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
                y * spacing, // 从地面向上延伸
                0.0f         // 墙在中心平面
            );

            glm::vec3 rotation(0.0f, 0.0f, 0.0f);
            glm::vec3 scale(cubeSize);

            // 使用不同颜色：底部蓝色，中间绿色，顶部红色
            float t = static_cast<float>(y) / static_cast<float>(height);
            glm::vec3 color(
                t,       // 红色从0到1
                0.5f,    // 绿色固定
                1.0f - t // 蓝色从1到0
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
            radius * std::cos(phi) + radius, // 抬高，使球体在地面之上
            radius * std::sin(phi) * std::sin(theta));

        glm::vec3 rotation(0.0f, 0.0f, 0.0f);
        glm::vec3 scale(0.8f);

        // 根据位置着色
        glm::vec3 color(
            (position.x + radius) / (2.0f * radius),
            (position.y) / (2.0f * radius),
            (position.z + radius) / (2.0f * radius));

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

    int segments = 12; // 隧道段数
    float segmentLength = 3.0f;
    float tunnelRadius = 5.0f;
    int cubesPerRing = 16; // 每圈的立方体数

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
                z);

            glm::vec3 rotation(
                0.0f,
                -glm::degrees(angle),
                0.0f);
            glm::vec3 scale(1.0f, 1.0f, 0.5f); // 扁平的立方体

            // 彩虹色渐变
            float t = static_cast<float>(seg) / static_cast<float>(segments);
            glm::vec3 color(
                std::sin(t * glm::two_pi<float>()) * 0.5f + 0.5f,
                std::sin(t * glm::two_pi<float>() + 2.0f) * 0.5f + 0.5f,
                std::sin(t * glm::two_pi<float>() + 4.0f) * 0.5f + 0.5f);

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
                std::sin(angle) * radius);

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
            std::sin(angle) * radius);

        glm::vec3 rotation(0.0f, glm::degrees(angle), 0.0f);
        glm::vec3 scale(1.0f);

        // 彩虹色渐变
        glm::vec3 color(
            std::sin(angle) * 0.5f + 0.5f,
            std::sin(angle + 2.0f) * 0.5f + 0.5f,
            std::sin(angle + 4.0f) * 0.5f + 0.5f);

        instances->Add(position, rotation, scale, color);
    }

    Core::Logger::GetInstance().Info("Geometry Showcase created: " +
                                     std::to_string(instances->GetCount()) + " cubes");
    return instances;
}

/**
 * 场景: Disco 舞台 (Disco Stage)
 * 使用所有几何体创建炫酷的Disco效果
 */
struct DiscoStage
{
    std::vector<std::unique_ptr<Renderer::InstancedRenderer>> renderers;
    std::vector<std::shared_ptr<Renderer::MeshBuffer>> meshBuffers;
    std::vector<std::shared_ptr<Renderer::InstanceData>> instanceDataList;
};

DiscoStage CreateDiscoStage()
{
    Core::Logger::GetInstance().Info("Creating Disco Stage...");

    DiscoStage stage;

    // ========================================
    // 舞台地板 - 使用平面
    // ========================================
    auto floorInstances = std::make_shared<Renderer::InstanceData>();

    // 中央舞池 - 纯白色地板（绕X轴旋转-90度使其水平）
    floorInstances->Add(
        glm::vec3(0.0f, -0.01f, 0.0f),
        glm::vec3(-90.0f, 0.0f, 0.0f), // 绕X轴旋转-90度，使平面水平
        glm::vec3(50.0f, 50.0f, 1.0f), // 原始平面在X-Y平面，旋转后X和Y变成地面的长宽
        glm::vec3(1.0f, 1.0f, 1.0f)    // 纯白色
    );

    // ========================================
    // 立方体组合成的球形灯
    // ========================================
    auto cubeInstances = std::make_shared<Renderer::InstanceData>();
    auto sphereInstances = std::make_shared<Renderer::InstanceData>();

    // 中央Disco球 - 使用大量统一立方体密集拟合
    glm::vec3 centerPos(0.0f, 8.0f, 0.0f);
    float discoBallRadius = 4.0f; // 增大到4.0米（原来2.5米）

    // 使用斐波那契球面分布算法均匀分布立方体
    const int numCubes = 800; // 增加到800个立方体（原来500个）
    const float goldenRatio = (1.0f + std::sqrt(5.0f)) / 2.0f;
    const float uniformCubeSize = 0.4f; // 增大立方体大小到0.4米（原来0.35米）

    for (int i = 0; i < numCubes; ++i)
    {
        // 斐波那契球面算法
        float theta = 2.0f * glm::pi<float>() * i / goldenRatio;
        float phi = std::acos(1.0f - 2.0f * (i + 0.5f) / numCubes);

        // 转换为笛卡尔坐标
        float x = discoBallRadius * std::sin(phi) * std::cos(theta);
        float y = discoBallRadius * std::sin(phi) * std::sin(theta);
        float z = discoBallRadius * std::cos(phi);

        glm::vec3 offset(x, y, z);

        // 根据位置设置颜色变化
        float colorVariation = 0.7f + 0.3f * std::sin(theta * 3.0f);
        glm::vec3 cubeColor(colorVariation, colorVariation, colorVariation + 0.1f);

        cubeInstances->Add(centerPos + offset, glm::vec3(0.0f), glm::vec3(uniformCubeSize), cubeColor);
    }

    // 中央核心球体 - 增大，作为内部核心
    sphereInstances->Add(
        glm::vec3(0.0f, 8.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f), // 旋转在渲染循环中应用
        glm::vec3(3.0f),             // 核心球体半径3.0米（原来1.8米），与立方体层更协调
        glm::vec3(1.0f, 1.0f, 0.95f) // 亮银白色
    );

    // 创建一圈立方体组合成的球形灯（8个位置，更加密集）
    for (int i = 0; i < 8; ++i)
    {
        float angle = i * (360.0f / 8.0f);
        float radius = 10.0f;
        float x = radius * cosf(glm::radians(angle));
        float z = radius * sinf(glm::radians(angle));

        glm::vec3 color;
        if (i % 4 == 0)
            color = glm::vec3(1.0f, 0.1f, 0.1f); // 红
        else if (i % 4 == 1)
            color = glm::vec3(0.1f, 1.0f, 0.1f); // 绿
        else if (i % 4 == 2)
            color = glm::vec3(0.1f, 0.1f, 1.0f); // 蓝
        else
            color = glm::vec3(1.0f, 1.0f, 0.1f); // 黄

        glm::vec3 lightCenter(x, 5.0f, z);

        // 每个球形灯使用100个统一立方体密集拟合
        const int cubesPerLight = 100;
        float lightRadius = 1.0f + (i % 3) * 0.2f; // 不同大小：1.0, 1.2, 1.4米
        const float lightCubeSize = 0.2f;          // 立方体边长

        for (int j = 0; j < cubesPerLight; ++j)
        {
            float theta = 2.0f * glm::pi<float>() * j / goldenRatio;
            float phi = std::acos(1.0f - 2.0f * (j + 0.5f) / cubesPerLight);

            float lx = lightRadius * std::sin(phi) * std::cos(theta);
            float ly = lightRadius * std::sin(phi) * std::sin(theta);
            float lz = lightRadius * std::cos(phi);

            glm::vec3 localOffset(lx, ly, lz);

            cubeInstances->Add(lightCenter + localOffset, glm::vec3(0.0f), glm::vec3(lightCubeSize), color);
        }

        // 每个球形灯的核心球体 - 与立方体层半径一致，但不同球体大小不同
        sphereInstances->Add(
            lightCenter,
            glm::vec3(0.0f, 0.0f, 0.0f), // 旋转在渲染循环中应用
            glm::vec3(lightRadius),      // 核心球体半径等于立方体层半径
            color * 1.2f                 // 核心球体颜色更亮
        );
    }

    // ========================================
    // 创建渲染器
    // ========================================

    // 地板渲染器
    Core::Logger::GetInstance().Info("Creating floor renderer...");
    try
    {
        Renderer::MeshBuffer floorMesh = Renderer::MeshBufferFactory::CreatePlaneBuffer(1.0f, 1.0f, 1, 1);
        auto floorMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(floorMesh));
        stage.meshBuffers.push_back(floorMeshPtr);

        auto floorRenderer = std::make_unique<Renderer::InstancedRenderer>();
        floorRenderer->SetMesh(floorMeshPtr);
        floorRenderer->SetInstances(floorInstances);
        floorRenderer->Initialize();
        stage.renderers.push_back(std::move(floorRenderer));
        stage.instanceDataList.push_back(floorInstances);
    }
    catch (const std::exception &e)
    {
        Core::Logger::GetInstance().Error("Failed to create floor renderer: " + std::string(e.what()));
    }

    // 立方体球形灯渲染器
    Core::Logger::GetInstance().Info("Creating cube-based sphere lights renderer...");
    try
    {
        Renderer::MeshBuffer cubeMesh = Renderer::MeshBufferFactory::CreateCubeBuffer();
        auto cubeMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(cubeMesh));
        stage.meshBuffers.push_back(cubeMeshPtr);

        auto cubeRenderer = std::make_unique<Renderer::InstancedRenderer>();
        cubeRenderer->SetMesh(cubeMeshPtr);
        cubeRenderer->SetInstances(cubeInstances);
        cubeRenderer->Initialize();
        stage.renderers.push_back(std::move(cubeRenderer));
        stage.instanceDataList.push_back(cubeInstances);
    }
    catch (const std::exception &e)
    {
        Core::Logger::GetInstance().Error("Failed to create cube renderer: " + std::string(e.what()));
    }

    // 中央核心球体渲染器
    Core::Logger::GetInstance().Info("Creating center core sphere renderer...");
    try
    {
        Renderer::MeshBuffer sphereMesh = Renderer::MeshBufferFactory::CreateSphereBuffer(32, 32, 1.0f);
        auto sphereMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(sphereMesh));
        stage.meshBuffers.push_back(sphereMeshPtr);

        auto sphereRenderer = std::make_unique<Renderer::InstancedRenderer>();
        sphereRenderer->SetMesh(sphereMeshPtr);
        sphereRenderer->SetInstances(sphereInstances);
        sphereRenderer->Initialize();
        stage.renderers.push_back(std::move(sphereRenderer));
        stage.instanceDataList.push_back(sphereInstances);
    }
    catch (const std::exception &e)
    {
        Core::Logger::GetInstance().Error("Failed to create sphere renderer: " + std::string(e.what()));
    }

    Core::Logger::GetInstance().Info("Super Disco Stage created: " +
                                     std::to_string(stage.renderers.size()) + " renderer types - " +
                                     "1 floor, " +
                                     std::to_string(cubeInstances->GetCount()) + " cubes (800 center + 800 colored), " +
                                     std::to_string(sphereInstances->GetCount()) + " core spheres (1 giant center + 8 colored)");

    return stage;
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
        Core::LogLevel::WARNING, // ✅ 改为WARNING级别，减少INFO输出
        true,
        rotationConfig);

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
            glm::vec3(0.0f, 12.0f, 25.0f), // ✅ 适合观察平面演示的位置
            glm::vec3(0.0f, 1.0f, 0.0f),   // 世界上向量
            -90.0f,                        // 初始偏航角
            -30.0f                         // ✅ 向下看的俯仰角
        );

        Core::MouseController mouseController;
        mouseController.Initialize(glfwGetCurrentContext());
        mouseController.SetMouseCapture(true);

        // 设置鼠标移动回调来更新摄像机方向
        glfwSetCursorPosCallback(glfwGetCurrentContext(), [](GLFWwindow *window, double xpos, double ypos)
                                 {
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
            } });

        // 设置滚轮回调来调整FOV
        glfwSetScrollCallback(glfwGetCurrentContext(), [](GLFWwindow *window, double xoffset, double yoffset)
                              {
            Core::Camera* cam = static_cast<Core::Camera*>(glfwGetWindowUserPointer(window));
            if (cam)
            {
                cam->ProcessMouseScroll(static_cast<float>(yoffset));
            } });

        // 设置窗口用户指针，使回调可以访问摄像机
        glfwSetWindowUserPointer(glfwGetCurrentContext(), &camera);

        Core::KeyboardController keyboardController;
        keyboardController.Initialize(glfwGetCurrentContext());

        // 键盘回调
        keyboardController.RegisterKeyCallback(GLFW_KEY_ESCAPE, []()
                                               {
            Core::Logger::GetInstance().Info("Exit requested");
            exit(0); });

        keyboardController.RegisterKeyCallback(GLFW_KEY_TAB, [&mouseController]()
                                               { mouseController.ToggleMouseCapture(); });

        // ========================================
        // 初始化多光源系统
        // ========================================
        std::vector<Renderer::Lighting::PointLightPtr> rotatingPointLights;
        Renderer::Lighting::SpotLightPtr flashlight;
        glm::vec3 centerPosition(0.0f, 0.0f, 0.0f);
        SetupLighting(rotatingPointLights, flashlight, centerPosition);

        // ========================================
        // 创建Skybox系统
        // ========================================
        Core::Logger::GetInstance().Info("========================================");
        Core::Logger::GetInstance().Info("Setting up Skybox system...");
        Core::Logger::GetInstance().Info("========================================");

        Renderer::Skybox skybox;
        skybox.Initialize();
        skybox.LoadShaders("assets/shader/skybox.vert", "assets/shader/skybox.frag");

        // ========================================
        // 加载天空盒纹理 - 使用SkyboxLoader
        // ========================================
        //
        // SkyboxLoader支持多种加载方式，适用于不同的天空盒资源格式：
        //
        // 方式1：CreateFromPattern - 基于约定和命名模式
        //   适用于：corona_rt.png, skybox_right.jpg, px.hdr 等
        //   使用预设的命名方案（OPENGL, MAYA, DIRECTX等）
        //
        // 方式2：CreateCustomScheme - 完全自定义面名称
        //   适用于：任意命名的6个面，完全可配置的后缀格式
        //   例如：FaceNamingScheme("rt", "lf", "up", "dn", "bk", "ft")
        //
        // 方式3：CreateCustomConfig - 完全自定义文件名
        //   适用于：任意命名的6个完整文件名
        //
        // 支持的约定（Convention）：
        //   - OPENGL: right, left, top, bottom, back, front
        //   - MAYA/Corona: rt, lf, up, dn, bk, ft (front/back与OpenGL相反)
        //   - DIRECTX: left, right, top, bottom, front, back
        //   - BLENDER: 与OpenGL类似
        //
        // 预设命名方案：
        //   - GetOpenGLScheme(): "right", "left", "top", "bottom", "back", "front"
        //   - GetMayaScheme(): "rt", "lf", "up", "dn", "bk", "ft"
        //   - GetDirectXScheme(): "left", "right", "top", "bottom", "front", "back"
        //   - GetHDRLabScheme(): "px", "nx", "py", "ny", "pz", "nz"
        //
        // ========================================

        // 加载Corona天空盒（使用完整自定义文件名）
        auto coronaConfig = Renderer::SkyboxLoader::CreateCustomConfig(
            "assets/textures/skybox",
            {"corona_rt.png", "corona_lf.png", "corona_up.png", "corona_dn.png", "corona_bk.png", "corona_ft.png"},
            Renderer::CubemapConvention::OPENGL);

        // 其他加载方式示例（注释掉，供参考）：
        //
        // 方式1：标准命名格式 (right.png, left.png, ...)
        // auto standardConfig = SkyboxLoader::CreateFromPattern(
        //     "assets/textures/skybox",
        //     "{face}",  // 或 "skybox_{face}"
        //     Renderer::CubemapConvention::OPENGL,
        //     ".png"
        // );
        //
        // 方式2：完全自定义面名称后缀
        // Renderer::FaceNamingScheme customSuffixes(
        //     "rt",   // right
        //     "lf",   // left
        //     "up",   // top
        //     "dn",   // bottom
        //     "bk",   // back
        //     "ft"    // front
        // );
        // auto customSchemeConfig = SkyboxLoader::CreateFromCustomScheme(
        //     "assets/textures/skybox",
        //     "corona_{face}",
        //     customSuffixes,
        //     Renderer::CubemapConvention::MAYA,
        //     ".png"
        // );
        //
        // 方式3：使用预设命名方案（如HDR Lab格式）
        // auto hdrConfig = SkyboxLoader::CreateFromCustomScheme(
        //     "assets/textures/skybox",
        //     "{face}",
        //     SkyboxLoader::GetHDRLabScheme(),  // "px", "nx", "py", "ny", "pz", "nz"
        //     Renderer::CubemapConvention::OPENGL,
        //     ".hdr"
        // );
        //
        // 方式4：完全自定义文件名
        // auto fullyCustomConfig = SkyboxLoader::CreateCustomConfig(
        //     "assets/textures/skybox",
        //     {"my_rt.jpg", "my_lf.jpg", "my_up.jpg", "my_dn.jpg", "my_bk.jpg", "my_ft.jpg"},
        //     Renderer::CubemapConvention::MAYA
        // );

        bool skyboxLoaded = skybox.LoadFromConfig(coronaConfig);

        if (skyboxLoaded)
        {
            Core::Logger::GetInstance().Info("✓ Skybox loaded successfully!");
            Core::Logger::GetInstance().Info("  Source: Corona skybox");
            Core::Logger::GetInstance().Info("  Convention: Maya (auto-converted to OpenGL)");
        }
        else
        {
            Core::Logger::GetInstance().Warning("✗ Skybox loading failed, continuing without skybox");
            Core::Logger::GetInstance().Info("  Tip: Check that corona_*.png files exist in assets/textures/skybox/");
        }

        // ========================================
        // 创建环境光照系统
        // ========================================
        Core::Logger::GetInstance().Info("Setting up ambient lighting system...");

        Renderer::AmbientLighting ambientLighting;
        ambientLighting.Initialize();

        if (skyboxLoaded)
        {
            ambientLighting.LoadFromSkybox(skybox.GetTextureID(), g_ambientIntensity);
            ambientLighting.SetMode(g_currentAmbientMode);
            Core::Logger::GetInstance().Info("✓ Ambient lighting loaded from skybox");
            Core::Logger::GetInstance().Info("  - Mode: SKYBOX_SAMPLE (default)");
            Core::Logger::GetInstance().Info("  - Intensity: " + std::to_string(g_ambientIntensity));
        }
        else
        {
            ambientLighting.SetMode(Renderer::AmbientLighting::Mode::HEMISPHERE);
            ambientLighting.SetHemisphereColors(
                glm::vec3(0.5f, 0.7f, 1.0f), // 蓝天
                glm::vec3(0.1f, 0.1f, 0.1f)  // 深灰地面
            );
            Core::Logger::GetInstance().Info("✓ Ambient lighting set to hemisphere mode (no skybox)");
        }

        // ========================================
        // 加载着色器
        // ========================================
        Core::Logger::GetInstance().Info("Loading shaders...");

        // 使用环境光照着色器
        Renderer::Shader ambientShader;
        ambientShader.Load("assets/shader/ambient_ibl.vert", "assets/shader/ambient_ibl.frag");
        Core::Logger::GetInstance().Info("Using ambient_ibl shader with skybox sampling");

        // ========================================
        // 创建Disco舞台
        // ========================================
        DiscoStage discoStage = CreateDiscoStage();

        // 初始化所有实例数据到GPU
        Core::Logger::GetInstance().Info("Uploading instance data to GPU...");
        for (size_t i = 0; i < discoStage.renderers.size(); ++i)
        {
            discoStage.renderers[i]->UpdateInstanceData();
            Core::Logger::GetInstance().Info("  Updated renderer " + std::to_string(i) + " with " +
                                             std::to_string(discoStage.renderers[i]->GetInstanceCount()) + " instances");
        }

        // 光源控制回调
        bool animateLights = true;

        keyboardController.RegisterKeyCallback(GLFW_KEY_SPACE, [&animateLights]()
                                               {
            animateLights = !animateLights;
            Core::Logger::GetInstance().Info("Light animation " + std::string(animateLights ? "resumed" : "paused")); });

        // 环境光模式切换
        keyboardController.RegisterKeyCallback(GLFW_KEY_1, [&ambientLighting]()
                                               {
            ambientLighting.SetMode(Renderer::AmbientLighting::Mode::SOLID_COLOR);
            g_currentAmbientMode = Renderer::AmbientLighting::Mode::SOLID_COLOR;
            Core::Logger::GetInstance().Info("Ambient mode: SOLID_COLOR (Traditional Phong)"); });

        keyboardController.RegisterKeyCallback(GLFW_KEY_2, [&ambientLighting]()
                                               {
            ambientLighting.SetMode(Renderer::AmbientLighting::Mode::SKYBOX_SAMPLE);
            g_currentAmbientMode = Renderer::AmbientLighting::Mode::SKYBOX_SAMPLE;
            Core::Logger::GetInstance().Info("Ambient mode: SKYBOX_SAMPLE (IBL from skybox)"); });

        keyboardController.RegisterKeyCallback(GLFW_KEY_3, [&ambientLighting]()
                                               {
            ambientLighting.SetMode(Renderer::AmbientLighting::Mode::HEMISPHERE);
            g_currentAmbientMode = Renderer::AmbientLighting::Mode::HEMISPHERE;
            Core::Logger::GetInstance().Info("Ambient mode: HEMISPHERE (Gradient sky to ground)"); });

        // 环境光强度调整
        keyboardController.RegisterKeyCallback(GLFW_KEY_RIGHT_BRACKET, [&]()
                                               {
            g_ambientIntensity = std::min(1.0f, g_ambientIntensity + 0.05f);
            ambientLighting.SetIntensity(g_ambientIntensity);
            Core::Logger::GetInstance().Info("Ambient intensity: " + std::to_string(g_ambientIntensity)); });

        keyboardController.RegisterKeyCallback(GLFW_KEY_LEFT_BRACKET, [&]()
                                               {
            g_ambientIntensity = std::max(0.0f, g_ambientIntensity - 0.05f);
            ambientLighting.SetIntensity(g_ambientIntensity);
            Core::Logger::GetInstance().Info("Ambient intensity: " + std::to_string(g_ambientIntensity)); });

        Core::Logger::GetInstance().Info("========================================");
        Core::Logger::GetInstance().Info("Disco Stage + Skybox loaded successfully!");
        Core::Logger::GetInstance().Info("Total renderers: " + std::to_string(discoStage.renderers.size()));
        Core::Logger::GetInstance().Info("Skybox: " + std::string(skyboxLoaded ? "Enabled" : "Disabled"));
        Core::Logger::GetInstance().Info("========================================");
        Core::Logger::GetInstance().Info("Controls:");
        Core::Logger::GetInstance().Info("  WASD   - Move camera");
        Core::Logger::GetInstance().Info("  Q/E    - Move up/down");
        Core::Logger::GetInstance().Info("  Mouse  - Look around");
        Core::Logger::GetInstance().Info("  TAB    - Toggle mouse capture");
        Core::Logger::GetInstance().Info("  SPACE  - Pause/Resume light animation");
        Core::Logger::GetInstance().Info("  1/2/3  - Switch ambient mode (Color/Skybox/Hemisphere)");
        Core::Logger::GetInstance().Info("  [ / ]  - Decrease/Increase ambient intensity");
        Core::Logger::GetInstance().Info("  ESC    - Exit");
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
                if (++logCounter >= 2) // 每1秒输出一次
                {
                    std::string logMessage = "Disco Stage | FPS: " +
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
                // 更新旋转的点光源位置 - 48个光源三层布局
                float time = static_cast<float>(glfwGetTime());
                for (size_t i = 0; i < rotatingPointLights.size(); ++i)
                {
                    // 每个光源有独特的运动参数
                    float angleOffset = static_cast<float>(i) * glm::two_pi<float>() / 48.0f; // 更新为48个光源
                    float speed = 0.5f + static_cast<float>(i % 5) * 0.3f;

                    // 根据光源所在的层级调整运动范围
                    float baseRadius, baseHeight;
                    if (i < 16)
                    {
                        // 内圈（0-15）
                        baseRadius = 8.0f;
                        baseHeight = 3.5f;
                    }
                    else if (i < 32)
                    {
                        // 中圈（16-31）
                        baseRadius = 14.0f;
                        baseHeight = 5.0f;
                    }
                    else
                    {
                        // 外圈（32-47）
                        baseRadius = 20.0f;
                        baseHeight = 6.5f;
                    }

                    // 不同的运动模式
                    int motionPattern = i % 4;
                    glm::vec3 offset(0.0f);

                    if (motionPattern == 0)
                    {
                        // 模式0：椭圆运动（水平拉伸）
                        float radiusX = baseRadius * 1.2f;
                        float radiusZ = baseRadius * 0.8f;
                        float height = baseHeight + std::sin(time * speed * 3.0f) * 0.8f;
                        offset = glm::vec3(
                            std::sin(time * speed + angleOffset) * radiusX,
                            height,
                            std::cos(time * speed + angleOffset) * radiusZ);
                    }
                    else if (motionPattern == 1)
                    {
                        // 模式1：8字形运动（利萨如曲线）
                        float radius = baseRadius * 0.9f;
                        offset = glm::vec3(
                            std::sin(time * speed + angleOffset) * radius,
                            baseHeight + std::sin(time * speed * 2.0f) * 0.6f,
                            std::sin(time * speed * 2.0f + angleOffset) * radius * 0.7f);
                    }
                    else if (motionPattern == 2)
                    {
                        // 模式2：螺旋进出运动
                        float radiusVariation = std::sin(time * speed * 0.5f) * (baseRadius * 0.25f);
                        float currentRadius = baseRadius + radiusVariation;
                        float height = baseHeight + std::cos(time * speed) * 1.0f;
                        offset = glm::vec3(
                            std::sin(time * speed * 1.5f + angleOffset) * currentRadius,
                            height,
                            std::cos(time * speed * 1.5f + angleOffset) * currentRadius);
                    }
                    else
                    {
                        // 模式3：随机抖动圆形运动
                        float radius = baseRadius * 1.1f;
                        float jitterX = std::sin(time * speed * 7.0f + i) * 2.0f;
                        float jitterZ = std::cos(time * speed * 5.0f + i) * 2.0f;
                        float height = baseHeight + std::sin(time * speed * 4.0f) * 0.7f;
                        offset = glm::vec3(
                            std::sin(time * speed * 0.8f + angleOffset) * radius + jitterX,
                            height,
                            std::cos(time * speed * 0.8f + angleOffset) * radius + jitterZ);
                    }

                    // 偶数索引添加额外的随机扰动
                    if (i % 2 == 0)
                    {
                        offset.x += std::sin(time * 2.0f + i) * 1.5f;
                        offset.z += std::cos(time * 1.5f + i) * 1.5f;
                    }

                    rotatingPointLights[i]->SetPosition(centerPosition + offset);
                }

                // 更新球体和立方体旋转（自转 + 公转）
                auto &sphereMatrices = discoStage.instanceDataList[2]->GetModelMatrices();
                auto &cubeMatrices = discoStage.instanceDataList[1]->GetModelMatrices();

                const float goldenRatio = (1.0f + std::sqrt(5.0f)) / 2.0f;

                // 中央球体自转（索引0）- 慢速三轴自转
                float centerRotX = std::sin(time * 0.3f) * 360.0f;
                float centerRotY = time * 20.0f;
                float centerRotZ = std::cos(time * 0.2f) * 360.0f;
                glm::mat4 centerModel = glm::mat4(1.0f);
                centerModel = glm::translate(centerModel, glm::vec3(0.0f, 8.0f, 0.0f));
                centerModel = glm::rotate(centerModel, glm::radians(centerRotX), glm::vec3(1.0f, 0.0f, 0.0f));
                centerModel = glm::rotate(centerModel, glm::radians(centerRotY), glm::vec3(0.0f, 1.0f, 0.0f));
                centerModel = glm::rotate(centerModel, glm::radians(centerRotZ), glm::vec3(0.0f, 0.0f, 1.0f));
                centerModel = glm::scale(centerModel, glm::vec3(3.0f)); // 更新为核心球体半径3.0米
                sphereMatrices[0] = centerModel;

                // 中央disco球周围的800个立方体自转（索引0-799）
                const int centerCubesCount = 800;   // 更新为800个立方体
                const float discoBallRadius = 4.0f; // 更新为4.0米半径
                const float centerCubeSize = 0.4f;  // 更新为0.4米立方体

                // 更新中心800个立方体的位置和旋转
                for (int j = 0; j < centerCubesCount; ++j)
                {
                    // 使用Fibonacci球算法重新计算每个立方体的相对位置
                    float theta = 2.0f * glm::pi<float>() * j / goldenRatio;
                    float phi = std::acos(1.0f - 2.0f * (j + 0.5f) / centerCubesCount);

                    float lx = discoBallRadius * std::sin(phi) * std::cos(theta);
                    float ly = discoBallRadius * std::sin(phi) * std::sin(theta);
                    float lz = discoBallRadius * std::cos(phi);

                    glm::vec3 localOffset(lx, ly, lz);
                    glm::vec3 cubePos = glm::vec3(0.0f, 8.0f, 0.0f) + localOffset;

                    // 应用与中央球体相同的自转
                    glm::mat4 cubeModel = glm::mat4(1.0f);
                    cubeModel = glm::translate(cubeModel, cubePos);
                    cubeModel = glm::rotate(cubeModel, glm::radians(centerRotX), glm::vec3(1.0f, 0.0f, 0.0f));
                    cubeModel = glm::rotate(cubeModel, glm::radians(centerRotY), glm::vec3(0.0f, 1.0f, 0.0f));
                    cubeModel = glm::rotate(cubeModel, glm::radians(centerRotZ), glm::vec3(0.0f, 0.0f, 1.0f));
                    cubeModel = glm::scale(cubeModel, glm::vec3(centerCubeSize));

                    cubeMatrices[j] = cubeModel;
                }

                // 8个彩色球体（索引1-8）- 自转 + 围绕中心公转
                float orbitRadius = 10.0f; // 公转半径
                float orbitSpeed = 0.5f;   // 公转速度
                const int cubesPerLight = 100;

                for (int i = 0; i < 8; ++i)
                {
                    // 初始角度位置
                    float initialAngle = i * (360.0f / 8.0f);

                    // 公转：围绕中心点(0, 8, 0)旋转
                    float currentOrbitAngle = glm::radians(initialAngle + time * orbitSpeed * 50.0f);
                    float x = orbitRadius * std::cos(currentOrbitAngle);
                    float z = orbitRadius * std::sin(currentOrbitAngle);

                    // 球体中心位置（包含公转）
                    glm::vec3 lightCenter(x, 5.0f, z);

                    // 自转：每个球体独特的自转速度
                    float selfRotSpeed = 0.5f + static_cast<float>(i) * 0.2f;
                    float selfRotX = std::sin(time * selfRotSpeed + i) * 180.0f;
                    float selfRotY = time * (50.0f + i * 15.0f); // 快速自转
                    float selfRotZ = std::cos(time * selfRotSpeed * 0.7f + i * 2.0f) * 180.0f;

                    // 构建模型矩阵：先平移到公转位置，再应用自转
                    glm::mat4 lightModel = glm::mat4(1.0f);
                    lightModel = glm::translate(lightModel, lightCenter);
                    lightModel = glm::rotate(lightModel, glm::radians(selfRotX), glm::vec3(1.0f, 0.0f, 0.0f));
                    lightModel = glm::rotate(lightModel, glm::radians(selfRotY), glm::vec3(0.0f, 1.0f, 0.0f));
                    lightModel = glm::rotate(lightModel, glm::radians(selfRotZ), glm::vec3(0.0f, 0.0f, 1.0f));

                    // 不同大小的球体
                    float lightRadius = 1.0f + (i % 3) * 0.2f;
                    lightModel = glm::scale(lightModel, glm::vec3(lightRadius));

                    sphereMatrices[i + 1] = lightModel; // 索引1-8对应8个彩色球体

                    // 更新每个彩色球体周围的100个立方体（索引800-1599）
                    const float lightCubeSize = 0.2f;
                    int cubeStartIndex = centerCubesCount + i * cubesPerLight; // 800 + i * 100

                    // 更新第i个彩色球体的100个立方体
                    for (int j = 0; j < cubesPerLight; ++j)
                    {
                        // 使用Fibonacci球算法计算立方体的相对位置
                        float theta = 2.0f * glm::pi<float>() * j / goldenRatio;
                        float phi = std::acos(1.0f - 2.0f * (j + 0.5f) / cubesPerLight);

                        float lx = lightRadius * std::sin(phi) * std::cos(theta);
                        float ly = lightRadius * std::sin(phi) * std::sin(theta);
                        float lz = lightRadius * std::cos(phi);

                        glm::vec3 localOffset(lx, ly, lz);
                        glm::vec3 cubePos = lightCenter + localOffset;

                        // 应用与球体相同的自转
                        glm::mat4 cubeModel = glm::mat4(1.0f);
                        cubeModel = glm::translate(cubeModel, cubePos);
                        cubeModel = glm::rotate(cubeModel, glm::radians(selfRotX), glm::vec3(1.0f, 0.0f, 0.0f));
                        cubeModel = glm::rotate(cubeModel, glm::radians(selfRotY), glm::vec3(0.0f, 1.0f, 0.0f));
                        cubeModel = glm::rotate(cubeModel, glm::radians(selfRotZ), glm::vec3(0.0f, 0.0f, 1.0f));
                        cubeModel = glm::scale(cubeModel, glm::vec3(lightCubeSize));

                        cubeMatrices[cubeStartIndex + j] = cubeModel;
                    }
                }

                // 更新手电筒位置和方向
                if (flashlight)
                {
                    flashlight->SetPosition(camera.GetPosition());
                    flashlight->SetDirection(camera.GetFront());
                }
            }

            // 更新实例数据到GPU
            discoStage.renderers[1]->UpdateInstanceData(); // 更新立方体
            discoStage.renderers[2]->UpdateInstanceData(); // 更新球体

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
            renderContext.renderPass = "DiscoStage";
            renderContext.batchIndex = 0;
            renderContext.drawCallCount = 1;
            renderContext.currentShader = "AmbientShader";
            Core::Logger::GetInstance().SetContext(renderContext);

            // 清空缓冲区
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // ========================================
            // 渲染天空盒（先渲染，作为背景）
            // ========================================
            if (skyboxLoaded)
            {
                skybox.Render(projection, view);
            }

            // ========================================
            // 设置着色器参数（使用环境光照）
            // ========================================
            ambientShader.Use();
            ambientShader.SetMat4("projection", projection);
            ambientShader.SetMat4("view", view);
            ambientShader.SetVec3("viewPos", camera.GetPosition());
            ambientShader.SetBool("useInstanceColor", true);
            ambientShader.SetBool("useTexture", false);
            ambientShader.SetFloat("shininess", 64.0f);
            ambientShader.SetFloat("time", static_cast<float>(currentTime));

            // 应用环境光照
            ambientLighting.ApplyToShader(ambientShader);

            // 应用所有直接光源
            Renderer::Lighting::LightManager::GetInstance().ApplyToShader(ambientShader);

            // ========================================
            // 渲染Disco舞台
            // ========================================

            static bool firstRender = true;
            if (firstRender)
            {
                Core::Logger::GetInstance().Info("Rendering Disco Stage with " +
                                                 std::to_string(discoStage.renderers.size()) + " renderers");
                firstRender = false;
            }

            // 渲染所有渲染器
            for (const auto &renderer : discoStage.renderers)
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
    catch (const std::exception &e)
    {
        Core::Logger::GetInstance().Error("Fatal error: " + std::string(e.what()));
        Core::Logger::GetInstance().Shutdown();
        return -1;
    }

    Core::Logger::GetInstance().Shutdown();
    return 0;
}
