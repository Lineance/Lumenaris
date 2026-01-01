#include "Core/Window.hpp"
#include "Core/Camera.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp"
#include "Core/Logger.hpp"
#include "Renderer/Core/RenderContext.hpp"  // ⭐ NEW - 多Context架构
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
#include <filesystem>

namespace fs = std::filesystem;

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
// 前置声明：DiscoStage 结构体
// ========================================
struct DiscoStage
{
    std::vector<std::unique_ptr<Renderer::InstancedRenderer>> renderers;
    std::vector<std::shared_ptr<Renderer::MeshBuffer>> meshBuffers;
    std::vector<std::shared_ptr<Renderer::InstanceData>> instanceDataList;
    std::shared_ptr<Renderer::InstanceData> bunnyInstances; // 传入的实例数据
    std::shared_ptr<Renderer::InstanceData> bunnyData;      // CreateForOBJ返回的实例数据（实际使用的）
    size_t bunnyRendererStart = 0;                          // bunny渲染器起始索引
    size_t bunnyRendererCount = 0;                          // bunny渲染器数量（材质数）
};

// ========================================
// 辅助函数：计算光源运动模式
// ========================================
glm::vec3 CalculateLightMotion(int index, float time, float baseRadius, float baseHeight)
{
    int motionPattern = index % 4;
    float angleOffset = static_cast<float>(index) * glm::two_pi<float>() / 48.0f;
    float speed = 0.5f + static_cast<float>(index % 5) * 0.3f;

    switch (motionPattern)
    {
    case 0: // 椭圆运动
    {
        float radiusX = baseRadius * 1.2f;
        float radiusZ = baseRadius * 0.8f;
        float height = baseHeight + std::sin(time * speed * 3.0f) * 0.8f;
        return glm::vec3(
            std::sin(time * speed + angleOffset) * radiusX,
            height,
            std::cos(time * speed + angleOffset) * radiusZ);
    }
    case 1: // 8字形运动（利萨如曲线）
    {
        float radius = baseRadius * 0.9f;
        return glm::vec3(
            std::sin(time * speed + angleOffset) * radius,
            baseHeight + std::sin(time * speed * 2.0f) * 0.6f,
            std::sin(time * speed * 2.0f + angleOffset) * radius * 0.7f);
    }
    case 2: // 螺旋进出运动
    {
        float radiusVariation = std::sin(time * speed * 0.5f) * (baseRadius * 0.25f);
        float currentRadius = baseRadius + radiusVariation;
        float height = baseHeight + std::cos(time * speed) * 1.0f;
        return glm::vec3(
            std::sin(time * speed * 1.5f + angleOffset) * currentRadius,
            height,
            std::cos(time * speed * 1.5f + angleOffset) * currentRadius);
    }
    default: // 随机抖动圆形运动
    {
        float radius = baseRadius * 1.1f;
        float jitterX = std::sin(time * speed * 7.0f + index) * 2.0f;
        float jitterZ = std::cos(time * speed * 5.0f + index) * 2.0f;
        float height = baseHeight + std::sin(time * speed * 4.0f) * 0.7f;
        return glm::vec3(
            std::sin(time * speed * 0.8f + angleOffset) * radius + jitterX,
            height,
            std::cos(time * speed * 0.8f + angleOffset) * radius + jitterZ);
    }
    }
}

// ========================================
// 辅助函数：更新Disco舞台动画
// ========================================
void UpdateDiscoStageAnimation(DiscoStage &stage, float time)
{
    const float goldenRatio = (1.0f + std::sqrt(5.0f)) / 2.0f;

    // 索引定义
    const size_t cubeDataIndex = 1;
    const size_t sphereDataIndex = 2;
    const size_t torusDataIndex = 3;
    const size_t platformDataIndex = 4;

    auto &cubeMatrices = stage.instanceDataList[cubeDataIndex]->GetModelMatrices();
    auto &sphereMatrices = stage.instanceDataList[sphereDataIndex]->GetModelMatrices();
    auto &torusMatrices = stage.instanceDataList[torusDataIndex]->GetModelMatrices();
    auto &platformMatrices = stage.instanceDataList[platformDataIndex]->GetModelMatrices();

    // 中央球体自转
    float centerRotX = std::sin(time * 0.3f) * 360.0f;
    float centerRotY = time * 20.0f;
    float centerRotZ = std::cos(time * 0.2f) * 360.0f;
    glm::mat4 centerModel = glm::mat4(1.0f);
    centerModel = glm::translate(centerModel, glm::vec3(0.0f, 8.0f, 0.0f));
    centerModel = glm::rotate(centerModel, glm::radians(centerRotX), glm::vec3(1.0f, 0.0f, 0.0f));
    centerModel = glm::rotate(centerModel, glm::radians(centerRotY), glm::vec3(0.0f, 1.0f, 0.0f));
    centerModel = glm::rotate(centerModel, glm::radians(centerRotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    centerModel = glm::scale(centerModel, glm::vec3(3.0f));
    sphereMatrices[0] = centerModel;

    // 中央disco球周围的800个立方体自转
    const int centerCubesCount = 800;
    const float discoBallRadius = 4.0f;
    const float centerCubeSize = 0.4f;

    for (int j = 0; j < centerCubesCount; ++j)
    {
        float theta = 2.0f * glm::pi<float>() * j / goldenRatio;
        float phi = std::acos(1.0f - 2.0f * (j + 0.5f) / centerCubesCount);
        glm::vec3 localOffset = glm::vec3(
            discoBallRadius * std::sin(phi) * std::cos(theta),
            discoBallRadius * std::sin(phi) * std::sin(theta),
            discoBallRadius * std::cos(phi));
        glm::vec3 cubePos = glm::vec3(0.0f, 8.0f, 0.0f) + localOffset;

        glm::mat4 cubeModel = glm::mat4(1.0f);
        cubeModel = glm::translate(cubeModel, cubePos);
        cubeModel = glm::rotate(cubeModel, glm::radians(centerRotX), glm::vec3(1.0f, 0.0f, 0.0f));
        cubeModel = glm::rotate(cubeModel, glm::radians(centerRotY), glm::vec3(0.0f, 1.0f, 0.0f));
        cubeModel = glm::rotate(cubeModel, glm::radians(centerRotZ), glm::vec3(0.0f, 0.0f, 1.0f));
        cubeModel = glm::scale(cubeModel, glm::vec3(centerCubeSize));
        cubeMatrices[j] = cubeModel;
    }

    // 8个彩色球体（自转 + 公转）
    float orbitRadius = 10.0f;
    float orbitSpeed = 0.5f;
    const int cubesPerLight = 100;

    for (int i = 0; i < 8; ++i)
    {
        float initialAngle = i * (360.0f / 8.0f);
        float currentOrbitAngle = glm::radians(initialAngle + time * orbitSpeed * 50.0f);
        float x = orbitRadius * std::cos(currentOrbitAngle);
        float z = orbitRadius * std::sin(currentOrbitAngle);
        glm::vec3 lightCenter(x, 5.0f, z);

        // 自转
        float selfRotSpeed = 0.5f + static_cast<float>(i) * 0.2f;
        float selfRotX = std::sin(time * selfRotSpeed + i) * 180.0f;
        float selfRotY = time * (50.0f + i * 15.0f);
        float selfRotZ = std::cos(time * selfRotSpeed * 0.7f + i * 2.0f) * 180.0f;

        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, lightCenter);
        lightModel = glm::rotate(lightModel, glm::radians(selfRotX), glm::vec3(1.0f, 0.0f, 0.0f));
        lightModel = glm::rotate(lightModel, glm::radians(selfRotY), glm::vec3(0.0f, 1.0f, 0.0f));
        lightModel = glm::rotate(lightModel, glm::radians(selfRotZ), glm::vec3(0.0f, 0.0f, 1.0f));
        lightModel = glm::scale(lightModel, glm::vec3(1.0f + (i % 3) * 0.2f));
        sphereMatrices[i + 1] = lightModel;

        // 更新每个彩色球体周围的100个立方体
        const float lightCubeSize = 0.2f;
        int cubeStartIndex = centerCubesCount + i * cubesPerLight;
        float lightRadius = 1.0f + (i % 3) * 0.2f;

        for (int j = 0; j < cubesPerLight; ++j)
        {
            float theta = 2.0f * glm::pi<float>() * j / goldenRatio;
            float phi = std::acos(1.0f - 2.0f * (j + 0.5f) / cubesPerLight);
            glm::vec3 localOffset = glm::vec3(
                lightRadius * std::sin(phi) * std::cos(theta),
                lightRadius * std::sin(phi) * std::sin(theta),
                lightRadius * std::cos(phi));
            glm::vec3 cubePos = lightCenter + localOffset;

            glm::mat4 cubeModel = glm::mat4(1.0f);
            cubeModel = glm::translate(cubeModel, cubePos);
            cubeModel = glm::rotate(cubeModel, glm::radians(selfRotX), glm::vec3(1.0f, 0.0f, 0.0f));
            cubeModel = glm::rotate(cubeModel, glm::radians(selfRotY), glm::vec3(0.0f, 1.0f, 0.0f));
            cubeModel = glm::rotate(cubeModel, glm::radians(selfRotZ), glm::vec3(0.0f, 0.0f, 1.0f));
            cubeModel = glm::scale(cubeModel, glm::vec3(lightCubeSize));
            cubeMatrices[cubeStartIndex + j] = cubeModel;
        }
    }

    // 更新圆环动画（5个装饰性圆环）
    const int numToruses = 5;
    for (int i = 0; i < numToruses; ++i)
    {
        float baseY = 8.0f + (i - 2) * 1.5f;
        float majorRadius = 5.0f + i * 0.8f;
        float minorRadius = 0.15f - i * 0.01f;
        float floatOffset = std::sin(time * 2.0f + i * 1.5f) * 0.5f;
        float y = baseY + floatOffset;
        float torusRot = time * (20.0f + i * 10.0f);
        float majorScale = majorRadius;
        float minorScale = minorRadius / 0.07f;

        glm::mat4 torusModel = glm::mat4(1.0f);
        torusModel = glm::translate(torusModel, glm::vec3(0.0f, y, 0.0f));
        torusModel = glm::rotate(torusModel, glm::radians(90.0f + torusRot), glm::vec3(0.0f, 1.0f, 0.0f));
        torusModel = glm::scale(torusModel, glm::vec3(majorScale, minorScale, majorScale));
        torusMatrices[i] = torusModel;
    }

    // 更新平台动画（16个环形平台 + 18个螺旋楼梯）
    const int numPlatforms = 16;
    const int numSteps = 18;

    // 更新16个环形平台
    for (int i = 0; i < numPlatforms; ++i)
    {
        float baseAngle = i * (360.0f / numPlatforms);
        float radius = 15.0f;
        float orbitSpeed = 0.3f + (i % 4) * 0.1f;
        float currentAngle = baseAngle + time * orbitSpeed * 10.0f;
        float x = radius * cosf(glm::radians(currentAngle));
        float z = radius * sinf(glm::radians(currentAngle));
        float floatY = 0.5f + std::sin(time * 1.5f + i * 0.5f) * 0.3f;

        glm::mat4 platformModel = glm::mat4(1.0f);
        platformModel = glm::translate(platformModel, glm::vec3(x, floatY, z));
        platformModel = glm::rotate(platformModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        platformModel = glm::rotate(platformModel, glm::radians(currentAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        platformModel = glm::scale(platformModel, glm::vec3(3.0f, 3.0f, 0.2f));
        platformMatrices[i] = platformModel;
    }

    // 更新18个螺旋楼梯台阶
    for (int i = 0; i < numSteps; ++i)
    {
        float baseAngle = i * (360.0f / numSteps) * 2.0f;
        float spiralRadius = 6.0f;
        float spiralHeight = 6.0f;
        float stepHeight = spiralHeight / numSteps;
        float x = spiralRadius * cosf(glm::radians(baseAngle));
        float z = spiralRadius * sinf(glm::radians(baseAngle));
        float y = 2.0f + i * stepHeight;
        float overallRotation = time * 15.0f;
        float rotatedX = x * std::cos(glm::radians(overallRotation)) - z * std::sin(glm::radians(overallRotation));
        float rotatedZ = x * std::sin(glm::radians(overallRotation)) + z * std::cos(glm::radians(overallRotation));

        glm::mat4 stepModel = glm::mat4(1.0f);
        stepModel = glm::translate(stepModel, glm::vec3(rotatedX, y, rotatedZ));
        stepModel = glm::rotate(stepModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        stepModel = glm::rotate(stepModel, glm::radians(baseAngle + overallRotation), glm::vec3(0.0f, 0.0f, 1.0f));
        stepModel = glm::scale(stepModel, glm::vec3(1.2f, 1.0f, 1.0f));
        platformMatrices[numPlatforms + i] = stepModel;
    }

    // ========================================
    // 更新斯坦福兔子动画（正常方向跳舞）
    // ========================================
    if (stage.bunnyData)
    {
        auto &bunnyMatrices = stage.bunnyData->GetModelMatrices();

        // 调试：每秒输出一次bunny的位置
        static int debugFrameCount = 0;
        static float lastDebugY = 0.0f;
        if (++debugFrameCount % 60 == 0)
        {
            glm::vec3 currentPos = glm::vec3(bunnyMatrices[0][3]);
            Core::Logger::GetInstance().Info("=== BUNNY ANIMATION DEBUG ===");
            Core::Logger::GetInstance().Info("Time: " + std::to_string(time));
            Core::Logger::GetInstance().Info("Bunny position: (" + std::to_string(currentPos.x) + ", " +
                                             std::to_string(currentPos.y) + ", " + std::to_string(currentPos.z) + ")");
            Core::Logger::GetInstance().Info("Y position changed: " + std::to_string(currentPos.y - lastDebugY));
            Core::Logger::GetInstance().Info("bunnyData pointer: " + std::to_string(reinterpret_cast<uintptr_t>(stage.bunnyData.get())));
            Core::Logger::GetInstance().Info("============================");
            lastDebugY = currentPos.y;
        }

        float moveRadius = 3.0f; // 移动范围
        float moveSpeed = 1.5f;  // 移动速度

        float bunnyX = std::sin(time * moveSpeed * 0.7f) * moveRadius * 0.6f +
                       std::sin(time * moveSpeed * 1.3f) * moveRadius * 0.3f;
        float bunnyZ = std::cos(time * moveSpeed * 0.9f) * moveRadius * 0.5f +
                       std::cos(time * moveSpeed * 1.1f) * moveRadius * 0.4f;
        float jumpHeight = std::abs(std::sin(time * moveSpeed * 2.0f)) * 1.0f; // 跳跃高度
        float bunnyY = 1.0f + jumpHeight;                                      // 基础高度1米
        float bunnyRotY = std::atan2(bunnyX, bunnyZ) * 180.0f / glm::pi<float>() + 180.0f; // 加上初始180度
        float breatheScale = 1.0f + std::sin(time * moveSpeed * 1.5f) * 0.1f; // 呼吸效果更明显

        glm::mat4 bunnyModel = glm::mat4(1.0f);
        bunnyModel = glm::translate(bunnyModel, glm::vec3(bunnyX, bunnyY, bunnyZ));
        bunnyModel = glm::rotate(bunnyModel, glm::radians(bunnyRotY), glm::vec3(0.0f, 1.0f, 0.0f));
        bunnyModel = glm::scale(bunnyModel, glm::vec3(2.0f * breatheScale)); // 使用2.0作为基准缩放
        bunnyMatrices[0] = bunnyModel;
    }
}

// ========================================
// 场景生成函数
// ========================================

/**
 * 初始化多光源系统
 */
void SetupLighting(
    Renderer::Core::RenderContext &renderContext,
    std::vector<Renderer::Lighting::PointLightPtr> &outRotatingLights,
    Renderer::Lighting::SpotLightPtr &outFlashlight,
    glm::vec3 &outCenterPosition)
{
    auto &lightManager = renderContext.GetLightManager();  // ⭐ 使用RenderContext

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
    // 立方体组合成的球形灯 + 新增几何体
    // ========================================
    auto cubeInstances = std::make_shared<Renderer::InstanceData>();
    auto sphereInstances = std::make_shared<Renderer::InstanceData>();
    auto torusInstances = std::make_shared<Renderer::InstanceData>();    // 新增：圆环实例
    auto platformInstances = std::make_shared<Renderer::InstanceData>(); // 新增：圆形平台实例

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
    // 新增：装饰性圆环（围绕中央Disco球）- 更细更圆
    // ========================================
    const int numToruses = 5; // 5个圆环
    for (int i = 0; i < numToruses; ++i)
    {
        float y = 8.0f + (i - 2) * 1.5f;       // 不同高度
        float majorRadius = 5.0f + i * 0.8f;   // 主半径（圆环半径）：5.0m -> 8.2m
        float minorRadius = 0.15f - i * 0.01f; // 管半径（圆环粗细）：0.15m -> 0.11m（更细）

        glm::vec3 torusColor(
            0.8f + 0.2f * std::sin(i * 1.5f),
            0.6f + 0.2f * std::cos(i * 1.5f),
            0.9f);

        // 基础Torus是 majorRadius=1.0, minorRadius=0.07
        // 我们需要缩放到实际尺寸
        // X和Z方向缩放：majorRadius / 1.0 = majorRadius
        // Y方向缩放：minorRadius / 0.07
        float majorScale = majorRadius;         // 主半径缩放
        float minorScale = minorRadius / 0.07f; // 管半径缩放（相对于基础mesh）

        torusInstances->Add(
            glm::vec3(0.0f, y, 0.0f),
            glm::vec3(90.0f, 0.0f, 0.0f),                  // 水平放置
            glm::vec3(majorScale, minorScale, majorScale), // X和Z相同确保正圆，Y单独缩放管粗细
            torusColor);
    }

    // ========================================
    // 新增：环形平台（围绕8个球形灯的外圈）
    // ========================================
    const int numPlatforms = 16; // 16个圆形平台
    for (int i = 0; i < numPlatforms; ++i)
    {
        float angle = i * (360.0f / numPlatforms);
        float radius = 15.0f; // 比球形灯更外圈
        float x = radius * cosf(glm::radians(angle));
        float z = radius * sinf(glm::radians(angle));

        glm::vec3 platformColor(
            0.3f + 0.1f * (i % 3),
            0.4f + 0.1f * ((i + 1) % 3),
            0.5f + 0.1f * ((i + 2) % 3));

        platformInstances->Add(
            glm::vec3(x, 0.5f, z),         // 放在地面高度
            glm::vec3(-90.0f, 0.0f, 0.0f), // 水平放置
            glm::vec3(3.0f, 3.0f, 0.2f),   // 半径3米，厚度0.2米
            platformColor);
    }

    // ========================================
    // 新增：螺旋楼梯（围绕中央Disco球）- 更多台阶、更厚、间隔更小
    // ========================================
    const int numSteps = 18; // 增加到18级台阶
    float spiralRadius = 6.0f;
    float spiralHeight = 6.0f;
    float stepWidth = 1.2f; // 台阶宽度
    float stepDepth = 1.0f; // 增加台阶深度到1.0m（更厚）
    float stepHeight = spiralHeight / numSteps;

    for (int i = 0; i < numSteps; ++i)
    {
        float angle = i * (360.0f / numSteps) * 2.0f; // 旋转720度（2圈）
        float x = spiralRadius * cosf(glm::radians(angle));
        float z = spiralRadius * sinf(glm::radians(angle));
        float y = 2.0f + i * stepHeight;

        glm::vec3 stepColor(
            0.7f,
            0.5f + 0.1f * (i % 2),
            0.9f - 0.1f * (i % 2));

        platformInstances->Add(
            glm::vec3(x, y, z),
            glm::vec3(-90.0f, 0.0f, angle), // 水平 + 旋转
            glm::vec3(stepWidth, stepDepth, 1.0f),
            stepColor);
    }

    // ========================================
    // 新增：斯坦福兔子在舞台上跳舞
    // ========================================
    // 注意：bunny将在最后添加到渲染列表

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

        // 关键验证：确保floorInstances和bunnyData不是同一个对象
        if (stage.bunnyData)
        {
            bool floorDifferentFromBunny = (floorInstances.get() != stage.bunnyData.get());
            Core::Logger::GetInstance().Info("Floor vs Bunny pointers: " + std::string(floorDifferentFromBunny ? "DIFFERENT (GOOD)" : "SAME (BAD!)"));
            if (!floorDifferentFromBunny)
            {
                Core::Logger::GetInstance().Error("CRITICAL: floorInstances and bunnyData are the SAME pointer!");
            }
        }

        Core::Logger::GetInstance().Info("Floor renderer index: " + std::to_string(stage.renderers.size() - 1));

        // 验证：输出floor的初始位置
        auto &floorMatrices = floorInstances->GetModelMatrices();
        glm::vec3 initialFloorPos = glm::vec3(floorMatrices[0][3]);
        Core::Logger::GetInstance().Info("Initial floor position: (" +
                                         std::to_string(initialFloorPos.x) + ", " +
                                         std::to_string(initialFloorPos.y) + ", " +
                                         std::to_string(initialFloorPos.z) + ")");
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
        Core::Logger::GetInstance().Info("Cube renderer index: " + std::to_string(stage.renderers.size() - 1));
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
        Core::Logger::GetInstance().Info("Sphere renderer index: " + std::to_string(stage.renderers.size() - 1));
    }
    catch (const std::exception &e)
    {
        Core::Logger::GetInstance().Error("Failed to create sphere renderer: " + std::string(e.what()));
    }

    // ========================================
    // 新增：圆环渲染器 - 使用高分段数和正确半径
    // ========================================
    Core::Logger::GetInstance().Info("Creating decorative torus renderer...");
    try
    {
        // 创建单位圆环作为基础，然后通过实例缩放调整大小
        // majorRadius=1.0, minorRadius=0.07 (相对比例，确保管足够粗)
        Renderer::MeshBuffer torusMesh = Renderer::MeshBufferFactory::CreateTorusBuffer(1.0f, 0.07f, 96, 64);
        auto torusMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(torusMesh));
        stage.meshBuffers.push_back(torusMeshPtr);

        auto torusRenderer = std::make_unique<Renderer::InstancedRenderer>();
        torusRenderer->SetMesh(torusMeshPtr);
        torusRenderer->SetInstances(torusInstances);
        torusRenderer->Initialize();
        stage.renderers.push_back(std::move(torusRenderer));
        stage.instanceDataList.push_back(torusInstances);
        Core::Logger::GetInstance().Info("Torus renderer index: " + std::to_string(stage.renderers.size() - 1));
    }
    catch (const std::exception &e)
    {
        Core::Logger::GetInstance().Error("Failed to create torus renderer: " + std::string(e.what()));
    }

    // ========================================
    // 新增：平台渲染器（环形平台 + 螺旋楼梯）
    // ========================================
    Core::Logger::GetInstance().Info("Creating platform renderer...");
    try
    {
        Renderer::MeshBuffer platformMesh = Renderer::MeshBufferFactory::CreatePlaneBuffer(1.0f, 1.0f, 1, 1);
        auto platformMeshPtr = std::make_shared<Renderer::MeshBuffer>(std::move(platformMesh));
        stage.meshBuffers.push_back(platformMeshPtr);

        auto platformRenderer = std::make_unique<Renderer::InstancedRenderer>();
        platformRenderer->SetMesh(platformMeshPtr);
        platformRenderer->SetInstances(platformInstances);
        platformRenderer->Initialize();
        stage.renderers.push_back(std::move(platformRenderer));
        stage.instanceDataList.push_back(platformInstances);
        Core::Logger::GetInstance().Info("Platform renderer index: " + std::to_string(stage.renderers.size() - 1));
    }
    catch (const std::exception &e)
    {
        Core::Logger::GetInstance().Error("Failed to create platform renderer: " + std::string(e.what()));
    }

    // ========================================
    // 新增：斯坦福兔子在舞台上跳舞（最后添加）
    // ========================================
    std::string bunnyPath = "assets/models/bunny.obj";
    std::shared_ptr<Renderer::InstanceData> bunnyInstances = nullptr;

    try
    {
        bunnyInstances = std::make_shared<Renderer::InstanceData>();

        // 添加1个兔子实例 - 放大尺寸以便观察
        bunnyInstances->Add(
            glm::vec3(0.0f, 1.0f, 0.0f), // 位置：舞台中心，地面以上1米
            glm::vec3(0.0f, 180.0f, 0.0f), // 旋转180度面向相机
            glm::vec3(2.0f),             // 缩放：2倍（显著增大）
            glm::vec3(1.0f, 0.0f, 0.0f)  // 红色
        );

        // 创建实例化渲染器（每个材质一个）
        auto [bunnyRenderers, bunnyMeshes, bunnyData] =
            Renderer::InstancedRenderer::CreateForOBJ(bunnyPath, bunnyInstances);

        // 记录bunny渲染器的索引范围
        stage.bunnyRendererStart = stage.renderers.size();
        stage.bunnyRendererCount = bunnyRenderers.size();

        Core::Logger::GetInstance().Info("=== BUNNY RENDERER DEBUG ===");
        Core::Logger::GetInstance().Info("Bunny has " + std::to_string(bunnyRenderers.size()) + " renderers (materials)");
        Core::Logger::GetInstance().Info("Bunny has " + std::to_string(bunnyMeshes.size()) + " mesh buffers");
        for (size_t i = 0; i < bunnyMeshes.size(); ++i)
        {
            Core::Logger::GetInstance().Info("  Mesh " + std::to_string(i) +
                                             " VAO: " + std::to_string(bunnyMeshes[i]->GetVAO()) +
                                             " Vertices: " + std::to_string(bunnyMeshes[i]->GetVertexCount()) +
                                             " Indices: " + std::to_string(bunnyMeshes[i]->GetIndexCount()) +
                                             " HasIndices: " + std::string(bunnyMeshes[i]->HasIndices() ? "YES" : "NO"));
        }
        Core::Logger::GetInstance().Info("================================");

        size_t bunnyRendererIndex = stage.renderers.size();
        for (auto &renderer : bunnyRenderers)
        {
            renderer.Initialize();
            stage.renderers.push_back(std::make_unique<Renderer::InstancedRenderer>(std::move(renderer)));
        }

        // 先添加到instanceDataList，然后再验证
        for (auto &mesh : bunnyMeshes)
        {
            stage.meshBuffers.push_back(mesh);
        }
        stage.instanceDataList.push_back(bunnyData);
        stage.bunnyData = bunnyData; // 保存bunnyData到DiscoStage

        // 现在可以安全地验证了
        Core::Logger::GetInstance().Info("=== CRITICAL VERIFICATION ===");
        Core::Logger::GetInstance().Info("Bunny renderer index: " + std::to_string(bunnyRendererIndex));
        Core::Logger::GetInstance().Info("Bunny renderer uses instanceData at: " +
                                         std::to_string(reinterpret_cast<uintptr_t>(bunnyInstances.get())));
        Core::Logger::GetInstance().Info("instanceDataList at: " +
                                         std::to_string(reinterpret_cast<uintptr_t>(stage.instanceDataList[stage.instanceDataList.size() - 1].get())));
        Core::Logger::GetInstance().Info("bunnyData at: " +
                                         std::to_string(reinterpret_cast<uintptr_t>(bunnyData.get())));
        Core::Logger::GetInstance().Info("All three should be the SAME!");
        Core::Logger::GetInstance().Info("================================");

        Core::Logger::GetInstance().Info("Stanford Bunny loaded successfully - " +
                                         std::to_string(bunnyRenderers.size()) + " renderers (materials)");
        Core::Logger::GetInstance().Info("Bunny renderer indices: [" +
                                         std::to_string(stage.bunnyRendererStart) + " to " +
                                         std::to_string(stage.bunnyRendererStart + stage.bunnyRendererCount - 1) + "]");

        // 验证指针地址
        Core::Logger::GetInstance().Info("=== POINTER VERIFICATION ===");
        Core::Logger::GetInstance().Info("bunnyInstances.get(): " + std::to_string(reinterpret_cast<uintptr_t>(bunnyInstances.get())));
        Core::Logger::GetInstance().Info("bunnyData.get(): " + std::to_string(reinterpret_cast<uintptr_t>(bunnyData.get())));
        Core::Logger::GetInstance().Info("stage.instanceDataList[last].get(): " + std::to_string(reinterpret_cast<uintptr_t>(stage.instanceDataList[stage.instanceDataList.size() - 1].get())));
        Core::Logger::GetInstance().Info("stage.bunnyData.get(): " + std::to_string(reinterpret_cast<uintptr_t>(stage.bunnyData.get())));
        Core::Logger::GetInstance().Info("Same pointer? " + std::string(bunnyData.get() == stage.instanceDataList[stage.instanceDataList.size() - 1].get() ? "YES" : "NO"));

        // 关键：检查bunnyInstances和bunnyData是否是同一个指针
        bool samePointer = (bunnyInstances.get() == bunnyData.get());
        Core::Logger::GetInstance().Info("bunnyInstances == bunnyData? " + std::string(samePointer ? "YES" : "NO"));

        if (!samePointer)
        {
            Core::Logger::GetInstance().Error("WARNING: bunnyInstances and bunnyData are DIFFERENT pointers!");
            Core::Logger::GetInstance().Error("This means the renderer uses bunnyInstances, but we're modifying bunnyData!");
        }

        Core::Logger::GetInstance().Info("================================");
    }
    catch (const std::exception &e)
    {
        Core::Logger::GetInstance().Error("Failed to load Stanford Bunny: " + std::string(e.what()));
        bunnyInstances = nullptr;
    }

    stage.bunnyInstances = bunnyInstances;

    // 打印渲染器索引分配
    Core::Logger::GetInstance().Info("========================================");
    Core::Logger::GetInstance().Info("Renderer Index Distribution:");
    Core::Logger::GetInstance().Info("========================================");

    size_t idx = 0;
    Core::Logger::GetInstance().Info("Floor renderer: " + std::to_string(idx++));
    Core::Logger::GetInstance().Info("Cube renderer: " + std::to_string(idx++));
    Core::Logger::GetInstance().Info("Sphere renderer: " + std::to_string(idx++));
    Core::Logger::GetInstance().Info("Torus renderer: " + std::to_string(idx++));
    Core::Logger::GetInstance().Info("Platform renderer: " + std::to_string(idx++));
    if (stage.bunnyRendererCount > 0)
    {
        Core::Logger::GetInstance().Info("Bunny renderers: [" + std::to_string(stage.bunnyRendererStart) +
                                         " to " + std::to_string(stage.bunnyRendererStart + stage.bunnyRendererCount - 1) + "] " +
                                         "(" + std::to_string(stage.bunnyRendererCount) + " materials)");
    }

    Core::Logger::GetInstance().Info("========================================");
    Core::Logger::GetInstance().Info("InstanceData List Index Distribution:");
    Core::Logger::GetInstance().Info("========================================");

    size_t dataIdx = 0;
    Core::Logger::GetInstance().Info("Floor instanceData: " + std::to_string(dataIdx++));
    Core::Logger::GetInstance().Info("Cube instanceData: " + std::to_string(dataIdx++));
    Core::Logger::GetInstance().Info("Sphere instanceData: " + std::to_string(dataIdx++));
    Core::Logger::GetInstance().Info("Torus instanceData: " + std::to_string(dataIdx++));
    Core::Logger::GetInstance().Info("Platform instanceData: " + std::to_string(dataIdx++));
    if (stage.bunnyData)
    {
        Core::Logger::GetInstance().Info("Bunny instanceData: " + std::to_string(dataIdx++));
    }

    Core::Logger::GetInstance().Info("========================================");

    Core::Logger::GetInstance().Info("Super Disco Stage created: " +
                                     std::to_string(stage.renderers.size()) + " renderer types - " +
                                     "1 floor, " +
                                     std::to_string(cubeInstances->GetCount()) + " cubes (800 center + 800 colored), " +
                                     std::to_string(sphereInstances->GetCount()) + " core spheres (1 giant center + 8 colored), " +
                                     std::to_string(torusInstances->GetCount()) + " decorative toruses, " +
                                     std::to_string(platformInstances->GetCount()) + " platforms (16 outer + 18 spiral), " +
                                     (bunnyInstances ? "1 dancing Stanford Bunny" : "0 bunny"));

    return stage;
}

// ========================================
// 主程序
// ========================================

int main()
{

    // Ensure logs directory exists
    namespace fs = std::filesystem;
    if (!fs::exists("logs"))
    {
        try
        {
            fs::create_directory("logs");
        }
        catch (const std::exception &e)
        {
            return 1;
        }
    }

    try
    {
        // ========================================
        // 初始化日志系统
        // ========================================
        Core::LogRotationConfig rotationConfig;
        rotationConfig.type = Core::RotationType::SIZE;
        rotationConfig.maxFileSize = 5 * 1024 * 1024; // 5MB
        rotationConfig.maxFiles = 3;

        std::string logPath = "logs/cool_cubes_demo.log";

        Core::Logger::GetInstance().Initialize(
            logPath,
            true,
            Core::LogLevel::INFO, // ✅ 改为INFO级别，确保能看到启动日志
            true,
            rotationConfig);

        Core::Logger::GetInstance().Info("========================================");
        Core::Logger::GetInstance().Info("Cool Cubes Demo - Starting...");
        Core::Logger::GetInstance().Info("========================================");
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
        // ⭐ 创建主场景RenderContext
        // ========================================
        Renderer::Core::RenderContext mainContext;

        // ========================================
        // 初始化多光源系统
        // ========================================
        std::vector<Renderer::Lighting::PointLightPtr> rotatingPointLights;
        Renderer::Lighting::SpotLightPtr flashlight;
        glm::vec3 centerPosition(0.0f, 0.0f, 0.0f);
        SetupLighting(mainContext, rotatingPointLights, flashlight, centerPosition);  // ⭐ 传递Context

        // ========================================
        // 创建Skybox系统
        // ========================================
        Core::Logger::GetInstance().Info("========================================");
        Core::Logger::GetInstance().Info("Setting up Skybox system...");
        Core::Logger::GetInstance().Info("========================================");

        Renderer::Skybox skybox;
        skybox.Initialize();
        skybox.LoadShaders("assets/shader/skybox.vert", "assets/shader/skybox.frag");

        auto coronaConfig = Renderer::SkyboxLoader::CreateCustomConfig(
            "assets/textures/skybox",
            {"corona_rt.png", "corona_lf.png", "corona_up.png", "corona_dn.png", "corona_bk.png", "corona_ft.png"},
            Renderer::CubemapConvention::BLENDER);

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

        // ========================================
        // 注册键盘回调
        // ========================================

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
                float time = static_cast<float>(glfwGetTime());

                // 更新48个旋转点光源（使用辅助函数简化）
                for (size_t i = 0; i < rotatingPointLights.size(); ++i)
                {
                    // 根据光源所在的层级调整运动范围
                    float baseRadius = (i < 16) ? 8.0f : (i < 32) ? 14.0f
                                                                  : 20.0f;
                    float baseHeight = (i < 16) ? 3.5f : (i < 32) ? 5.0f
                                                                  : 6.5f;

                    glm::vec3 offset = CalculateLightMotion(i, time, baseRadius, baseHeight);

                    // 偶数索引添加额外的随机扰动
                    if (i % 2 == 0)
                    {
                        offset.x += std::sin(time * 2.0f + i) * 1.5f;
                        offset.z += std::cos(time * 1.5f + i) * 1.5f;
                    }

                    rotatingPointLights[i]->SetPosition(centerPosition + offset);
                }

                // 更新Disco舞台几何体动画
                UpdateDiscoStageAnimation(discoStage, time);

                // 更新手电筒位置和方向
                if (flashlight)
                {
                    flashlight->SetPosition(camera.GetPosition());
                    flashlight->SetDirection(camera.GetFront());
                }
            }

            // 更新实例数据到GPU
            // 索引顺序：floor=0, cube=1, sphere=2, torus=3, platform=4, bunny=5+
            const size_t cubeIndex = 1;
            const size_t sphereIndex = 2;
            const size_t torusIndex = 3;
            const size_t platformIndex = 4;

            // 更新几何体渲染器
            discoStage.renderers[cubeIndex]->UpdateInstanceData();
            discoStage.renderers[sphereIndex]->UpdateInstanceData();
            discoStage.renderers[torusIndex]->UpdateInstanceData();
            discoStage.renderers[platformIndex]->UpdateInstanceData();

            // 更新bunny的所有材质渲染器
            static bool firstBunnyUpdate = true;
            if (firstBunnyUpdate)
            {
                Core::Logger::GetInstance().Info("=== BUNNY UPDATE DEBUG ===");
                Core::Logger::GetInstance().Info("bunnyRendererStart: " + std::to_string(discoStage.bunnyRendererStart));
                Core::Logger::GetInstance().Info("bunnyRendererCount: " + std::to_string(discoStage.bunnyRendererCount));
                Core::Logger::GetInstance().Info("renderers.size(): " + std::to_string(discoStage.renderers.size()));
                Core::Logger::GetInstance().Info("Will update renderers " +
                                                 std::to_string(discoStage.bunnyRendererStart) + " to " +
                                                 std::to_string(discoStage.bunnyRendererStart + discoStage.bunnyRendererCount - 1));
                Core::Logger::GetInstance().Info("========================");
                firstBunnyUpdate = false;
            }

            for (size_t i = discoStage.bunnyRendererStart;
                 i < discoStage.bunnyRendererStart + discoStage.bunnyRendererCount; ++i)
            {
                if (i < discoStage.renderers.size())
                {
                    // 获取renderer使用的instances指针
                    auto rendererInstances = discoStage.renderers[i]->GetInstances();

                    static int updateCount = 0;
                    if (++updateCount <= 5) // 只输出前5次
                    {
                        Core::Logger::GetInstance().Info("Renderer " + std::to_string(i) +
                                                         " UpdateInstanceData() - instances pointer: " +
                                                         std::to_string(reinterpret_cast<uintptr_t>(rendererInstances.get())));
                    }

                    discoStage.renderers[i]->UpdateInstanceData();
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

            // 应用环境光照（使用纹理单元10，避免与常用纹理冲突）
            ambientLighting.ApplyToShader(ambientShader);  // 默认 textureUnit = 10


            // ⭐ 应用所有直接光源（使用RenderContext）
            mainContext.GetLightManager().ApplyToShader(ambientShader);

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
            if (firstRender)
            {
                Core::Logger::GetInstance().Info("=== RENDERING ===");
                Core::Logger::GetInstance().Info("Total renderers: " + std::to_string(discoStage.renderers.size()));
                for (size_t i = 0; i < discoStage.renderers.size(); ++i)
                {
                    Core::Logger::GetInstance().Info("Renderer " + std::to_string(i) +
                                                     " VAO: " + std::to_string(discoStage.renderers[i]->GetMesh()->GetVAO()) +
                                                     " Instances: " + std::to_string(discoStage.renderers[i]->GetInstanceCount()));
                }
                Core::Logger::GetInstance().Info("================");
                firstRender = false;
            }

            for (size_t i = 0; i < discoStage.renderers.size(); ++i)
            {
                discoStage.renderers[i]->Render();
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
        // Try to log the error, but if logger is not initialized, use stderr
        try
        {
            Core::Logger::GetInstance().Error("Fatal error: " + std::string(e.what()));
            Core::Logger::GetInstance().Shutdown();
        }
        catch (...)
        {
        }
        return -1;
    }

    try
    {
        Core::Logger::GetInstance().Shutdown();
    }
    catch (...)
    {
        // Ignore shutdown errors
    }
    return 0;
}
