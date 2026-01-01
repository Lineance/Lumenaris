# 多光源系统使用指南

## 📋 概述

多光源系统提供了灵活的光照管理，支持三种类型的光源：
- **平行光 (DirectionalLight)**: 模拟太阳光，所有光线平行
- **点光源 (PointLight)**: 从一个点向所有方向发光，如灯泡
- **聚光灯 (SpotLight)**: 从一个点向特定方向锥形发光，如手电筒

## 🎯 核心特性

### 1. 光源类型

#### 平行光 (DirectionalLight)
```cpp
auto sun = std::make_shared<Renderer::Lighting::DirectionalLight>(
    glm::vec3(-0.2f, -1.0f, -0.3f),  // 方向
    glm::vec3(1.0f, 0.95f, 0.9f),    // 颜色（暖白色）
    1.0f,                             // 强度
    0.1f,                             // 环境光分量
    0.8f,                             // 漫反射分量
    0.5f                              // 镜面反射分量
);
```

#### 点光源 (PointLight)
```cpp
auto bulb = std::make_shared<Renderer::Lighting::PointLight>(
    glm::vec3(0.0f, 5.0f, 0.0f),     // 位置
    glm::vec3(1.0f, 0.8f, 0.6f),      // 颜色（暖黄色）
    1.0f,                              // 强度
    0.1f, 0.8f, 0.5f,                  // 光照分量
    Renderer::Lighting::PointLight::Attenuation::Range20()  // 衰减范围20米
);
```

#### 聚光灯 (SpotLight)
```cpp
auto flashlight = std::make_shared<Renderer::Lighting::SpotLight>(
    cameraPos,                        // 位置（跟随摄像机）
    cameraFront,                      // 方向（摄像机朝向）
    glm::vec3(1.0f, 1.0f, 1.0f),     // 颜色（白色）
    1.0f,                             // 强度
    0.1f, 0.8f, 0.5f,                 // 光照分量
    Renderer::Lighting::PointLight::Attenuation::Range50(),  // 50米范围
    glm::radians(12.5f),              // 内锥角度
    glm::radians(15.0f)               // 外锥角度（边缘柔化）
);
```

### 2. LightManager 使用

#### 添加光源
```cpp
auto &lightManager = Renderer::Lighting::LightManager::GetInstance();

// 添加平行光
int sunIndex = lightManager.AddDirectionalLight(sun);

// 添加多个点光源
int bulb1Index = lightManager.AddPointLight(bulb1);
int bulb2Index = lightManager.AddPointLight(bulb2);

// 添加聚光灯
int flashlightIndex = lightManager.AddSpotLight(flashlight);
```

#### 移除光源
```cpp
// 通过索引移除
lightManager.RemovePointLight(bulb1Index);

// 清空所有光源
lightManager.ClearAll();
```

#### 查询光源
```cpp
// 获取光源指针
auto light = lightManager.GetPointLight(bulb2Index);
if (light) {
    light->SetColor(glm::vec3(1.0f, 0.0f, 0.0f));  // 改为红色
    light->SetIntensity(2.0f);                       // 增加强度
}

// 获取光源数量
int numPointLights = lightManager.GetPointLightCount();
```

### 3. 在渲染循环中使用

#### 基本用法
```cpp
// 1. 设置着色器
multiLightShader.Use();

// 2. 设置材质属性
multiLightShader.SetVec3("objectColor", objectColor);
multiLightShader.SetFloat("shininess", 64.0f);
multiLightShader.SetBool("useInstanceColor", true);
multiLightShader.SetBool("useTexture", false);

// 3. 设置摄像机位置
multiLightShader.SetVec3("viewPos", camera.GetPosition());

// 4. 应用所有光源到着色器
lightManager.ApplyToShader(multiLightShader);

// 5. 设置变换矩阵
multiLightShader.SetMat4("projection", projection);
multiLightShader.SetMat4("view", view);

// 6. 渲染物体
renderer.Render();
```

#### 动态更新光源
```cpp
// 每帧更新聚光灯位置和方向（跟随摄像机）
auto flashlight = lightManager.GetSpotLight(flashlightIndex);
if (flashlight) {
    flashlight->SetPosition(camera.GetPosition());
    flashlight->SetDirection(camera.GetFront());
}

// 闪烁效果（随机改变强度）
static float time = 0.0f;
time += deltaTime;
float flicker = 0.8f + 0.2f * std::sin(time * 10.0f);
bulb->SetIntensity(flicker);

// 开关光源
if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
    bulb->Toggle();
}
```

## 🎨 完整示例

```cpp
#include "Renderer/Lighting/LightManager.hpp"
#include "Renderer/Lighting/Light.hpp"
#include "Renderer/Resources/Shader.hpp"
#include "Core/Camera.hpp"

// 初始化光源系统
void SetupLighting()
{
    auto &lightManager = Renderer::Lighting::LightManager::GetInstance();

    // 1. 添加太阳光（平行光）
    auto sun = std::make_shared<Renderer::Lighting::DirectionalLight>(
        glm::vec3(-0.3f, -1.0f, -0.2f),
        glm::vec3(1.0f, 0.95f, 0.8f),
        0.5f,  // 较弱的环境光
        0.6f,  // 中等漫反射
        0.3f   // 较弱镜面反射
    );
    lightManager.AddDirectionalLight(sun);

    // 2. 添加多个彩色点光源（创建灯光阵列）
    glm::vec3 colors[] = {
        glm::vec3(1.0f, 0.0f, 0.0f),  // 红
        glm::vec3(0.0f, 1.0f, 0.0f),  // 绿
        glm::vec3(0.0f, 0.0f, 1.0f),  // 蓝
        glm::vec3(1.0f, 1.0f, 0.0f)   // 黄
    };

    for (int i = 0; i < 4; ++i)
    {
        float angle = i * glm::two_pi<float>() / 4.0f;
        glm::vec3 pos(std::cos(angle) * 10.0f, 5.0f, std::sin(angle) * 10.0f);

        auto light = std::make_shared<Renderer::Lighting::PointLight>(
            pos,
            colors[i],
            2.0f,  // 较高强度
            0.05f, 0.8f, 0.5f,
            Renderer::Lighting::PointLight::Attenuation::Range32()
        );
        lightManager.AddPointLight(light);
    }

    // 3. 添加手电筒（聚光灯）
    auto flashlight = std::make_shared<Renderer::Lighting::SpotLight>(
        glm::vec3(0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        1.5f,
        0.0f, 0.8f, 1.0f,
        Renderer::Lighting::PointLight::Attenuation::Range50(),
        glm::radians(10.0f),
        glm::radians(12.5f)
    );
    lightManager.AddSpotLight(flashlight);

    // 打印光源信息
    lightManager.PrintAllLights();
}

// 渲染循环
void RenderLoop(Camera &camera, Shader &multiLightShader)
{
    auto &lightManager = Renderer::Lighting::LightManager::GetInstance();

    // 更新手电筒位置和方向
    auto flashlight = lightManager.GetSpotLight(0);
    if (flashlight) {
        flashlight->SetPosition(camera.GetPosition());
        flashlight->SetDirection(camera.GetFront());
    }

    // 应用光源
    lightManager.ApplyToShader(multiLightShader);

    // 设置其他uniform
    multiLightShader.SetVec3("viewPos", camera.GetPosition());
    multiLightShader.SetBool("useInstanceColor", true);

    // 渲染场景...
}
```

## 📊 性能优化建议

### 1. 光源数量限制
```cpp
// 着色器中定义的最大数量
#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 8

// 查询当前数量
int count = lightManager.GetPointLightCount();
```

### 2. 光源剔除
```cpp
// 禁用距离过远的光源
for (int i = 0; i < lightManager.GetPointLightCount(); ++i) {
    auto light = lightManager.GetPointLight(i);
    float distance = glm::length(light->GetPosition() - cameraPos);
    if (distance > 50.0f) {
        light->SetEnabled(false);  // 禁用
    }
}
```

### 3. 使用预设的衰减参数
```cpp
// 根据需要的距离选择合适的衰减
PointLight::Attenuation::Range7()    // 7米  - 小房间
PointLight::Attenuation::Range13()   // 13米 - 中等房间
PointLight::Attenuation::Range20()   // 20米 - 大房间
PointLight::Attenuation::Range32()   // 32米 - 大厅
PointLight::Attenuation::Range50()   // 50米 - 室外
PointLight::Attenuation::Range100()  // 100米 - 大场景
```

## 🛠️ 调试工具

```cpp
// 获取统计信息
std::string stats = lightManager.GetStatistics();
Core::Logger::GetInstance().Info(stats);

// 打印所有光源信息
lightManager.PrintAllLights();

// 获取光源描述
auto light = lightManager.GetPointLight(0);
std::string desc = light->GetDescription();
Core::Logger::GetInstance().Info(desc);
```

## 📐 技术细节

### Phong 光照模型
系统使用改进的 Phong 光照模型：
- **环境光 (Ambient)**: 基础照明，无方向性
- **漫反射 (Diffuse)**: 基于光线与法线的夹角
- **镜面反射 (Specular)**: 基于视线与反射方向的夹角

### 衰减公式
点光源和聚光灯使用三次衰减公式：
```
attenuation = 1.0 / (constant + linear * d + quadratic * d²)
```

### Gamma 校正
着色器自动应用 Gamma 校正：
```glsl
result = pow(result, vec3(1.0 / 2.2));
```

## 🎓 最佳实践

1. **光源数量控制**: 尽量保持点光源数量在8个以内
2. **使用衰减参数**: 选择合适的衰减范围，避免过度计算
3. **动态光源开关**: 根据场景需要动态启用/禁用光源
4. **颜色温度**: 使用合理的颜色温度值（暖色2700K，冷色6500K）
5. **强度平衡**: 调整环境光、漫反射、镜面反射的平衡

## 📚 相关文件

- 头文件: `include/Renderer/Lighting/Light.hpp`
- 实现: `src/Renderer/Lighting/Light.cpp`
- 管理器: `include/Renderer/Lighting/LightManager.hpp`
- 着色器: `assets/shader/multi_light.vert` / `multi_light.frag`
