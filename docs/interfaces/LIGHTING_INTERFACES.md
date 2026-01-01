# Lighting 模块接口文档

## 📋 目录

- [Light 基类](#light-基类)
- [DirectionalLight 类](#directionallight-类)
- [PointLight 类](#pointlight-类)
- [SpotLight 类](#spotlight-类)
- [LightManager 类](#lightmanager-类)

---

## Light 基类

所有光源的抽象基类，定义通用属性。

```cpp
namespace Renderer::Lighting {

enum class LightType {
    DIRECTIONAL,  // 平行光
    POINT,        // 点光源
    SPOT          // 聚光灯
};

class Light {
public:
    Light(const glm::vec3& color = glm::vec3(1.0f),
          float intensity = 1.0f,
          float ambient = 0.1f,
          float diffuse = 0.8f,
          float specular = 0.5f);

    virtual ~Light() = default;

    // 通用属性
    const glm::vec3& GetColor() const;
    void SetColor(const glm::vec3& color);

    float GetIntensity() const;
    void SetIntensity(float intensity);

    bool IsEnabled() const;
    void SetEnabled(bool enabled);
    void Toggle();

    float GetAmbient() const;
    void SetAmbient(float ambient);

    float GetDiffuse() const;
    void SetDiffuse(float diffuse);

    float GetSpecular() const;
    void SetSpecular(float specular);

    // 虚函数
    virtual LightType GetType() const = 0;
    virtual void ApplyToShader(Shader& shader, int index = 0) const = 0;
    virtual std::string GetDescription() const = 0;
};

}
```

### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `GetColor()` | 无 | const glm::vec3& | 获取光源颜色 |
| `SetColor()` | color | void | 设置光源颜色（RGB，0-1） |
| `GetIntensity()` | 无 | float | 获取光源强度 |
| `SetIntensity()` | intensity | void | 设置光源强度（建议0.1-10.0） |
| `IsEnabled()` | 无 | bool | 检查光源是否启用 |
| `SetEnabled()` | enabled | void | 启用/禁用光源 |
| `Toggle()` | 无 | void | 切换光源开关状态 |
| `GetAmbient()` | 无 | float | 获取环境光分量（0-1） |
| `SetAmbient()` | ambient | void | 设置环境光分量 |
| `GetDiffuse()` | 无 | float | 获取漫反射分量（0-1） |
| `SetDiffuse()` | diffuse | void | 设置漫反射分量 |
| `GetSpecular()` | 无 | float | 获取镜面反射分量（0-1） |
| `SetSpecular()` | specular | void | 设置镜面反射分量 |

---

## DirectionalLight 类

平行光（方向光），模拟太阳光。

```cpp
namespace Renderer::Lighting {

class DirectionalLight : public Light {
public:
    DirectionalLight(
        const glm::vec3& direction = glm::vec3(-0.2f, -1.0f, -0.3f),
        const glm::vec3& color = glm::vec3(1.0f),
        float intensity = 1.0f,
        float ambient = 0.1f,
        float diffuse = 0.8f,
        float specular = 0.5f
    );

    LightType GetType() const override;
    void ApplyToShader(Shader& shader, int index = 0) const override;
    std::string GetDescription() const override;

    const glm::vec3& GetDirection() const;
    void SetDirection(const glm::vec3& direction);
};

}
```

### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `GetDirection()` | 无 | const glm::vec3& | 获取光线方向 |
| `SetDirection()` | direction | void | 设置光线方向（会被归一化） |

### 特性
- ✅ 无位置信息（所有光线平行）
- ✅ 无衰减
- ✅ 适用于全局照明（太阳、月亮）
- ✅ 性能开销小

### 使用示例

```cpp
// 创建太阳光
auto sun = std::make_shared<Renderer::Lighting::DirectionalLight>(
    glm::vec3(-0.3f, -1.0f, -0.2f),  // 从右上方照射
    glm::vec3(1.0f, 0.95f, 0.8f),    // 暖白色
    0.8f                              // 强度
);

sun->SetAmbient(0.15f);  // 增加环境光

// 添加到管理器
Renderer::Lighting::LightManager::GetInstance().AddDirectionalLight(sun);
```

---

## PointLight 类

点光源，从一点向所有方向发光。

```cpp
namespace Renderer::Lighting {

class PointLight : public Light {
public:
    struct Attenuation {
        float constant;
        float linear;
        float quadratic;

        // 静态工厂方法
        static Attenuation Range7();
        static Attenuation Range13();
        static Attenuation Range20();
        static Attenuation Range32();
        static Attenuation Range50();
        static Attenuation Range65();
        static Attenuation Range100();
    };

    PointLight(
        const glm::vec3& position = glm::vec3(0.0f),
        const glm::vec3& color = glm::vec3(1.0f),
        float intensity = 1.0f,
        float ambient = 0.1f,
        float diffuse = 0.8f,
        float specular = 0.5f,
        const Attenuation& attenuation = Attenuation::Range20()
    );

    LightType GetType() const override;
    void ApplyToShader(Shader& shader, int index = 0) const override;
    std::string GetDescription() const override;

    const glm::vec3& GetPosition() const;
    void SetPosition(const glm::vec3& position);

    const Attenuation& GetAttenuation() const;
    void SetAttenuation(const Attenuation& attenuation);

    float GetEffectiveRange() const;
};

}
```

### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `GetPosition()` | 无 | const glm::vec3& | 获取光源位置 |
| `SetPosition()` | position | void | 设置光源位置 |
| `GetAttenuation()` | 无 | const Attenuation& | 获取衰减参数 |
| `SetAttenuation()` | attenuation | void | 设置衰减参数 |
| `GetEffectiveRange()` | 无 | float | 获取有效照射距离（近似值） |

### 衰减参数说明

| 预设 | 常数项 | 线性项 | 二次项 | 有效距离 |
|------|--------|--------|--------|----------|
| `Range7()` | 1.0 | 0.7 | 1.8 | ~7米 |
| `Range13()` | 1.0 | 0.35 | 0.44 | ~13米 |
| `Range20()` | 1.0 | 0.22 | 0.20 | ~20米 |
| `Range32()` | 1.0 | 0.14 | 0.07 | ~32米 |
| `Range50()` | 1.0 | 0.09 | 0.032 | ~50米 |
| `Range65()` | 1.0 | 0.07 | 0.017 | ~65米 |
| `Range100()` | 1.0 | 0.045 | 0.0075 | ~100米 |

### 使用示例

```cpp
// 创建灯泡
auto bulb = std::make_shared<Renderer::Lighting::PointLight>(
    glm::vec3(0.0f, 3.0f, 0.0f),     // 位置
    glm::vec3(1.0f, 0.8f, 0.6f),      // 暖黄色
    2.0f,                              // 强度
    0.05f, 0.8f, 0.5f,                 // 光照分量
    Renderer::Lighting::PointLight::Attenuation::Range20()
);

// 动态移动
bulb->SetPosition(glm::vec3(
    std::sin(time) * 5.0f,
    3.0f,
    std::cos(time) * 5.0f
));

// 闪烁效果
float flicker = 0.8f + 0.2f * std::sin(time * 10.0f);
bulb->SetIntensity(flicker);
```

---

## SpotLight 类

聚光灯，从一点向特定方向锥形发光。

```cpp
namespace Renderer::Lighting {

class SpotLight : public PointLight {
public:
    SpotLight(
        const glm::vec3& position = glm::vec3(0.0f),
        const glm::vec3& direction = glm::vec3(0.0f, -1.0f, 0.0f),
        const glm::vec3& color = glm::vec3(1.0f),
        float intensity = 1.0f,
        float ambient = 0.1f,
        float diffuse = 0.8f,
        float specular = 0.5f,
        const PointLight::Attenuation& attenuation = PointLight::Attenuation::Range20(),
        float cutOff = glm::radians(12.5f),
        float outerCutOff = glm::radians(15.0f)
    );

    LightType GetType() const override;
    void ApplyToShader(Shader& shader, int index = 0) const override;
    std::string GetDescription() const override;

    const glm::vec3& GetDirection() const;
    void SetDirection(const glm::vec3& direction);

    float GetCutOff() const;
    void SetCutOff(float cutOff);

    float GetOuterCutOff() const;
    void SetOuterCutOff(float outerCutOff);

    // 度数版本（更直观）
    float GetCutOffDegrees() const;
    void SetCutOffDegrees(float degrees);
    float GetOuterCutOffDegrees() const;
    void SetOuterCutOffDegrees(float degrees);
};

}
```

### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `GetDirection()` | 无 | const glm::vec3& | 获取光照方向 |
| `SetDirection()` | direction | void | 设置光照方向 |
| `GetCutOff()` | 无 | float | 获取内锥角度（弧度） |
| `SetCutOff()` | cutOff | void | 设置内锥角度（弧度） |
| `GetCutOffDegrees()` | 无 | float | 获取内锥角度（度数） |
| `SetCutOffDegrees()` | degrees | void | 设置内锥角度（度数） |
| `GetOuterCutOffDegrees()` | 无 | float | 获取外锥角度（度数） |

### 特性
- ✅ 位置和方向
- ✅ 截止角度（内锥和外锥）
- ✅ 边缘柔化效果
- ✅ 随距离衰减
- ✅ 适用于手电筒、车灯、舞台灯

### 使用示例

```cpp
// 创建手电筒（跟随摄像机）
auto flashlight = std::make_shared<Renderer::Lighting::SpotLight>(
    camera.GetPosition(),
    camera.GetFront(),
    glm::vec3(1.0f, 1.0f, 1.0f),
    1.5f,
    0.0f, 0.8f, 1.0f,
    Renderer::Lighting::PointLight::Attenuation::Range50(),
    glm::radians(10.0f),   // 10度内锥
    glm::radians(12.5f)    // 12.5度外锥（边缘柔化）
);

// 每帧更新位置和方向
flashlight->SetPosition(camera.GetPosition());
flashlight->SetDirection(camera.GetFront());
```

---

## LightManager 类

光源管理器，单例模式，管理场景中的所有光源。

```cpp
namespace Renderer::Lighting {

class LightManager {
public:
    static const int MAX_DIRECTIONAL_LIGHTS = 4;
    static const int MAX_POINT_LIGHTS = 16;
    static const int MAX_SPOT_LIGHTS = 8;

    // 单例访问
    static LightManager& GetInstance();

    // 添加光源
    int AddDirectionalLight(const DirectionalLightPtr& light);
    int AddPointLight(const PointLightPtr& light);
    int AddSpotLight(const SpotLightPtr& light);

    // 移除光源
    bool RemoveDirectionalLight(int index);
    bool RemovePointLight(int index);
    bool RemoveSpotLight(int index);
    void ClearAll();

    // 获取光源
    DirectionalLightPtr GetDirectionalLight(int index);
    PointLightPtr GetPointLight(int index);
    SpotLightPtr GetSpotLight(int index);

    // 查询数量
    int GetDirectionalLightCount() const;
    int GetPointLightCount() const;
    int GetSpotLightCount() const;
    int GetTotalLightCount() const;

    // 应用到着色器
    void ApplyToShader(Shader& shader) const;

    // 调试信息
    std::string GetStatistics() const;
    void PrintAllLights() const;
};

}
```

### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `GetInstance()` | 无 | LightManager& | 获取单例实例 |
| `AddDirectionalLight()` | light | int | 添加平行光，返回索引 |
| `AddPointLight()` | light | int | 添加点光源，返回索引 |
| `AddSpotLight()` | light | int | 添加聚光灯，返回索引 |
| `RemoveDirectionalLight()` | index | bool | 移除平行光 |
| `RemovePointLight()` | index | bool | 移除点光源 |
| `RemoveSpotLight()` | index | bool | 移除聚光灯 |
| `ClearAll()` | 无 | void | 清空所有光源 |
| `GetDirectionalLight()` | index | DirectionalLightPtr | 获取平行光指针 |
| `GetPointLight()` | index | PointLightPtr | 获取点光源指针 |
| `GetSpotLight()` | index | SpotLightPtr | 获取聚光灯指针 |
| `GetDirectionalLightCount()` | 无 | int | 获取平行光数量 |
| `GetPointLightCount()` | 无 | int | 获取点光源数量 |
| `GetSpotLightCount()` | 无 | int | 获取聚光灯数量 |
| `GetTotalLightCount()` | 无 | int | 获取光源总数 |
| `ApplyToShader()` | shader | void | 将所有光源应用到着色器 |
| `GetStatistics()` | 无 | string | 获取统计信息 |
| `PrintAllLights()` | 无 | void | 打印所有光源信息 |

### 使用示例

```cpp
// 1. 获取管理器实例
auto& lightManager = Renderer::Lighting::LightManager::GetInstance();

// 2. 添加光源
auto sun = std::make_shared<Renderer::Lighting::DirectionalLight>(...);
int sunIndex = lightManager.AddDirectionalLight(sun);

// 3. 在渲染循环中应用
multiLightShader.Use();
lightManager.ApplyToShader(multiLightShader);

// 4. 动态更新光源
auto sunLight = lightManager.GetDirectionalLight(sunIndex);
sunLight->SetIntensity(0.5f);

// 5. 移除光源
lightManager.RemoveDirectionalLight(sunIndex);

// 6. 调试信息
lightManager.PrintAllLights();
```

### 光源数量限制

| 类型 | 最大数量 | 说明 |
|------|---------|------|
| 平行光 | 4 | 通常1个足够（太阳） |
| 点光源 | 16 | 建议8个以内 |
| 聚光灯 | 8 | 建议4个以内 |

---

## 完整使用流程

```cpp
// 1. 初始化光源
auto& lightManager = Renderer::Lighting::LightManager::GetInstance();

// 太阳光
auto sun = std::make_shared<Renderer::Lighting::DirectionalLight>(
    glm::vec3(-0.2f, -1.0f, -0.3f),
    glm::vec3(1.0f, 0.95f, 0.9f)
);
lightManager.AddDirectionalLight(sun);

// 灯泡
auto bulb = std::make_shared<Renderer::Lighting::PointLight>(
    glm::vec3(0.0f, 5.0f, 0.0f),
    glm::vec3(1.0f, 0.8f, 0.6f),
    2.0f,
    0.1f, 0.8f, 0.5f,
    Renderer::Lighting::PointLight::Attenuation::Range20()
);
lightManager.AddPointLight(bulb);

// 手电筒
auto flashlight = std::make_shared<Renderer::Lighting::SpotLight>(...);
int flashlightIndex = lightManager.AddSpotLight(flashlight);

// 2. 渲染循环
while (!glfwWindowShouldClose(window)) {
    // 更新手电筒位置
    auto fl = lightManager.GetSpotLight(flashlightIndex);
    fl->SetPosition(camera.GetPosition());
    fl->SetDirection(camera.GetFront());

    // 渲染
    multiLightShader.Use();
    lightManager.ApplyToShader(multiLightShader);
    multiLightShader.SetVec3("viewPos", camera.GetPosition());

    // 渲染物体...
    RenderObjects();
}
```

---

*多光源系统提供了灵活、高效的光照管理，支持复杂场景的光照需求。*
