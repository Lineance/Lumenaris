# OpenGL学习项目接口文档

## 📋 目录

- [Core 模块接口](#core-模块接口)
  - [Window 类](#window-类)
  - [Camera 类](#camera-类)
  - [MouseController 类](#mousecontroller-类)
  - [KeyboardController 类](#keyboardcontroller-类)
  - [Logger 类](#logger-类)
- [Lighting 模块接口](#lighting-模块接口)
  - [LightHandle 类](#lighthandle-类)
  - [Light 类](#light-类)
  - [DirectionalLight 类](#directionallight-类)
  - [PointLight 类](#pointlight-类)
  - [SpotLight 类](#spotlight-类)
  - [LightManager 类](#lightmanager-类)
- [Renderer 模块接口](#renderer-模块接口)
  - [IMesh 抽象接口](#imesh-抽象接口)
  - [IRenderer 抽象接口](#irenderer-抽象接口)
  - [MeshFactory 工厂类](#meshfactory-工厂类)
  - [Shader 类](#shader-类)
  - [Texture 类](#texture-类)
  - [Cube 类](#cube-类)
  - [Sphere 类](#sphere-类)
  - [OBJModel 类](#objmodel-类)
  - [InstanceData 类](#instancedata-类)
  - [MeshData 类](#meshdata-类)
  - [MeshBuffer 类](#meshbuffer-类)
  - [InstancedRenderer 类](#instancedrenderer-类)
- [Environment 模块接口](#environment-模块接口)
  - [Skybox 类](#skybox-类)
  - [SkyboxLoader 类](#skyboxloader-类)
  - [AmbientLighting 类](#ambientlighting-类)
- [几何体接口](#几何体接口)
- [使用示例](#使用示例)

---

## Core 模块接口

### Window 类

窗口管理类，封装GLFW窗口操作。

```cpp
namespace Core {
class Window {
public:
    // 构造函数与析构函数
    Window(int width, int height, const std::string& title);
    ~Window();

    // 窗口生命周期管理
    void Init();                                    // 初始化GLFW窗口和OpenGL上下文
    void PollEvents() const;                       // 处理窗口事件队列
    void SwapBuffers() const;                      // 交换前后缓冲区
    bool ShouldClose() const;                      // 检查窗口是否应该关闭
    void SetWindowShouldClose() const;             // 设置窗口关闭标志

    // 属性访问
    int GetWidth() const;                          // 获取窗口宽度
    int GetHeight() const;                         // 获取窗口高度
    void SetSize(int width, int height);           // 设置窗口尺寸（内部使用）
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Init()` | 无 | void | 初始化GLFW窗口，创建OpenGL上下文，设置窗口属性 |
| `PollEvents()` | 无 | void | 处理所有待处理的窗口事件 |
| `SwapBuffers()` | 无 | void | 交换前后缓冲区，实现双缓冲渲染 |
| `ShouldClose()` | 无 | bool | 检查用户是否请求关闭窗口 |
| `GetWidth()` | 无 | int | 返回当前窗口宽度 |
| `GetHeight()` | 无 | int | 返回当前窗口高度 |

---

### Camera 类

3D摄像机类，封装摄像机位置、方向、移动和矩阵计算。

```cpp
namespace Core {
class Camera {
public:
    // 移动方向枚举
    enum class MovementDirection {
        FORWARD,   // 前进
        BACKWARD,  // 后退
        LEFT,      // 左移
        RIGHT,     // 右移
        UP,        // 上升
        DOWN       // 下降
    };

    // 投影类型枚举
    enum class ProjectionType {
        PERSPECTIVE,  // 透视投影（近大远小）
        ORTHO         // 正交投影（无透视效果）
    };

    // 构造函数
    Camera(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f),
           const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f,
           float pitch = 0.0f);

    // 矩阵获取
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(float aspect, float nearPlane = 0.1f, float farPlane = 100.0f) const;

    // 输入处理
    void ProcessKeyboard(MovementDirection direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

    // 位置和方向
    const glm::vec3& GetPosition() const;
    void SetPosition(const glm::vec3& position);
    const glm::vec3& GetFront() const;
    const glm::vec3& GetUp() const;
    const glm::vec3& GetRight() const;
    const glm::vec3& GetWorldUp() const;

    // 欧拉角
    float GetYaw() const;
    float GetPitch() const;
    void SetYaw(float yaw);
    void SetPitch(float pitch);

    // FOV和速度
    float GetFOV() const;
    void SetFOV(float fov);
    float GetMovementSpeed() const;
    void SetMovementSpeed(float speed);
    float GetMouseSensitivity() const;
    void SetMouseSensitivity(float sensitivity);

    // 投影类型
    ProjectionType GetProjectionType() const;
    void SetProjectionType(ProjectionType type);

    // 工具方法
    void Reset(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f),
               const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
               float yaw = -90.0f,
               float pitch = 0.0f);
    void LookAt(const glm::vec3& target);
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Camera()` | position, up, yaw, pitch | - | 构造摄像机，设置初始位置和朝向 |
| `GetViewMatrix()` | 无 | glm::mat4 | 获取视图矩阵（世界坐标→摄像机坐标） |
| `GetProjectionMatrix()` | aspect, nearPlane, farPlane | glm::mat4 | 获取投影矩阵（摄像机坐标→裁剪坐标） |
| `ProcessKeyboard()` | direction, deltaTime | void | 处理键盘输入，移动摄像机 |
| `ProcessMouseMovement()` | xoffset, yoffset, constrainPitch | void | 处理鼠标移动，更新摄像机方向 |
| `ProcessMouseScroll()` | yoffset | void | 处理滚轮滚动，调整FOV实现缩放 |
| `GetPosition()` | 无 | const glm::vec3& | 获取摄像机位置 |
| `SetPosition()` | position | void | 设置摄像机位置 |
| `GetFront()` | 无 | const glm::vec3& | 获取摄像机前向向量 |
| `GetFOV()` | 无 | float | 获取视场角 |
| `SetFOV()` | fov | void | 设置视场角（限制在1-120度） |
| `GetMovementSpeed()` | 无 | float | 获取移动速度 |
| `SetMovementSpeed()` | speed | void | 设置移动速度 |
| `Reset()` | position, up, yaw, pitch | void | 重置摄像机到初始状态 |
| `LookAt()` | target | void | 让摄像机观察指定目标点 |

#### 功能特性

**六自由度移动**：

- WASD: 前后左右移动
- Q/E: 垂直上下移动
- 所有移动都基于摄像机坐标系

**视角控制**：

- 鼠标移动: 更新摄像机朝向
- 自动限制俯仰角（防止万向节死锁）
- 滚轮缩放: 调整FOV

**矩阵计算**：

- View Matrix: 使用glm::lookAt计算
- Projection Matrix: 支持透视和正交投影

#### 使用示例

```cpp
// 1. 创建摄像机
Core::Camera camera(
    glm::vec3(0.0f, 15.0f, 40.0f),  // 位置
    glm::vec3(0.0f, 1.0f, 0.0f),    // 世界上向量
    -90.0f,                          // 初始偏航角
    0.0f                             // 初始俯仰角
);

// 2. 在渲染循环中处理输入
float deltaTime = 0.016f; // 假设60FPS

// WASD移动
if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(Core::Camera::MovementDirection::FORWARD, deltaTime);
if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(Core::Camera::MovementDirection::BACKWARD, deltaTime);
if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(Core::Camera::MovementDirection::LEFT, deltaTime);
if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(Core::Camera::MovementDirection::RIGHT, deltaTime);
if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    camera.ProcessKeyboard(Core::Camera::MovementDirection::DOWN, deltaTime);
if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    camera.ProcessKeyboard(Core::Camera::MovementDirection::UP, deltaTime);

// 鼠标旋转（在鼠标回调函数中）
void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    // 计算偏移...
    camera.ProcessMouseMovement(xoffset, yoffset);
}

// 滚轮缩放（在滚轮回调函数中）
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

// 3. 获取矩阵并传给着色器
float aspect = (float)windowWidth / (float)windowHeight;
glm::mat4 view = camera.GetViewMatrix();
glm::mat4 projection = camera.GetProjectionMatrix(aspect);

shader.Use();
shader.SetMat4("view", view);
shader.SetMat4("projection", projection);
```

#### 设计说明

**与MouseController的配合**：

- `Camera`: 负责摄像机状态管理、矩阵计算、移动逻辑
- `MouseController`: 负责捕获GLFW鼠标事件
- 建议将MouseController的鼠标偏移传递给Camera.ProcessMouseMovement()

**坐标系统**：

- 使用右手坐标系
- Y轴向上为正方向
- 初始朝向为-Z方向（通过yaw=-90.0f实现）

**性能优化**：

- 矩阵计算使用惰性求值（调用GetViewMatrix时才计算）
- 方向向量只在欧拉角改变时更新
- 移动使用deltaTime归一化，保证不同帧率下速度一致

---

### MouseController 类

鼠标输入控制类，处理鼠标移动、滚轮和捕获状态。

```cpp
namespace Core {
class MouseController {
public:
    // 构造函数与析构函数
    MouseController();
    ~MouseController() = default;

    // 初始化
    void Initialize(GLFWwindow* window);

    // 状态查询
    float GetYaw() const;                          // 获取水平旋转角度
    float GetPitch() const;                         // 获取垂直旋转角度
    float GetFOV() const;                          // 获取视场角
    bool IsFirstMouse() const;                     // 检查是否为第一次鼠标移动
    bool IsMouseCaptured() const;                  // 检查鼠标是否被捕获

    // 摄像机控制
    glm::vec3 GetCameraFront() const;              // 获取摄像机前向向量
    void UpdateCameraVectors();                    // 更新摄像机方向向量

    // 事件设置
    static void SetMouseCallback(GLFWwindow* window);
    static void SetScrollCallback(GLFWwindow* window);

    // 配置方法
    void SetMouseSensitivity(float sensitivity);   // 设置鼠标灵敏度
    void SetScrollSensitivity(float sensitivity);  // 设置滚轮灵敏度
    void ToggleMouseCapture();                     // 切换鼠标捕获状态
    void SetMouseCapture(bool captured);           // 设置鼠标捕获状态
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Initialize()` | GLFWwindow* | void | 将控制器绑定到指定窗口 |
| `GetYaw()` | 无 | float | 返回当前水平旋转角度（度） |
| `GetPitch()` | 无 | float | 返回当前垂直旋转角度（度） |
| `GetFOV()` | 无 | float | 返回当前视场角（度） |
| `GetCameraFront()` | 无 | glm::vec3 | 返回摄像机前向方向向量 |
| `SetMouseSensitivity()` | float | void | 设置鼠标移动灵敏度 |

---

### Logger 类

日志记录系统，支持分级日志输出和文件保存。

```cpp
namespace Core {
// 日志级别枚举
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

// 轮转类型枚举
enum class RotationType {
    NONE,       ///< 不轮转
    SIZE,       ///< 按文件大小轮转
    DAILY,      ///< 每日轮转
    HOURLY      ///< 每小时轮转
};

// 轮转配置结构体
struct LogRotationConfig {
    RotationType type = RotationType::NONE;    ///< 轮转类型
    size_t maxFileSize = 10 * 1024 * 1024;     ///< 最大文件大小（字节，默认10MB）
    int maxFiles = 5;                          ///< 最大历史文件数量
    bool compressOldLogs = false;              ///< 是否压缩旧日志文件
};

class Logger {
public:
    // 获取单例实例
    static Logger& GetInstance();

    // 初始化（支持异步和轮转配置）
    void Initialize(const std::string& logFilePath = "logs/application.log",
                   bool consoleOutput = true,
                   LogLevel minLevel = LogLevel::DEBUG,
                   bool async = true,
                   const LogRotationConfig& rotationConfig = LogRotationConfig());

    // 配置方法
    void SetMinLevel(LogLevel level);
    void SetConsoleOutput(bool enabled);

    // 日志记录方法
    void Debug(const std::string& message);
    void Info(const std::string& message);
    void Warning(const std::string& message);
    void Error(const std::string& message);

    // 清理资源
    void Shutdown();
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `GetInstance()` | 无 | Logger& | 获取Logger单例实例 |
| `Initialize()` | string logFilePath, bool consoleOutput, LogLevel minLevel, bool async, LogRotationConfig rotationConfig | void | 初始化日志系统，支持异步写入和轮转配置 |
| `SetMinLevel()` | LogLevel level | void | 设置最小日志级别，低于此级别的日志将被过滤 |
| `SetConsoleOutput()` | bool enabled | void | 启用或禁用控制台输出 |
| `Debug()` | string message | void | 记录DEBUG级别日志 |
| `Info()` | string message | void | 记录INFO级别日志 |
| `Warning()` | string message | void | 记录WARNING级别日志 |
| `Error()` | string message | void | 记录ERROR级别日志 |
| `Shutdown()` | 无 | void | 关闭日志系统并清理资源 |

#### 高级功能说明

**异步写入**: 默认启用异步模式，使用后台线程写入日志，避免阻塞主线程。

**日志轮转**: 支持三种轮转模式：

- `SIZE`: 文件大小超过限制时轮转
- `DAILY`: 每日轮转
- `HOURLY`: 每小时轮转

**配置示例**:

```cpp
// 基本配置（异步，不轮转）
Core::Logger::GetInstance().Initialize("logs/app.log", true, Core::LogLevel::INFO);

// 带轮转配置（按大小轮转，最大5个文件）
Core::LogRotationConfig rotationConfig;
rotationConfig.type = Core::RotationType::SIZE;
rotationConfig.maxFileSize = 10 * 1024 * 1024; // 10MB
rotationConfig.maxFiles = 5;

Core::Logger::GetInstance().Initialize("logs/app.log", true, Core::LogLevel::DEBUG,
                                       true, rotationConfig);

// 同步模式（适合调试）
Core::Logger::GetInstance().Initialize("logs/debug.log", true, Core::LogLevel::DEBUG,
                                       false); // 同步模式
```

---

### 使用示例

### KeyboardController 类

键盘输入控制类，支持按键状态查询和事件回调。

```cpp
namespace Core {
class KeyboardController {
public:
    // 构造函数与析构函数
    KeyboardController();
    ~KeyboardController();

    // 初始化
    void Initialize(GLFWwindow* window);

    // 按键状态查询
    bool IsKeyPressed(int key) const;              // 检查按键是否正在按下
    bool IsKeyJustPressed(int key) const;          // 检查按键是否刚刚按下
    bool IsKeyJustReleased(int key) const;         // 检查按键是否刚刚释放

    // 事件注册
    void RegisterKeyCallback(int key, std::function<void()> callback,
                             bool repeat = false, float repeatDelay = 0.1f);
    void UnregisterKeyCallback(int key);

    // 配置方法
    void SetKeyRepeatEnabled(bool enabled);        // 启用/禁用按键重复
    void SetKeyRepeatDelay(float delay);           // 设置重复延迟时间

    // 更新方法
    void Update(float deltaTime);                  // 每帧调用以更新状态
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `IsKeyPressed()` | int key | bool | 返回按键当前是否被按下 |
| `IsKeyJustPressed()` | int key | bool | 返回按键是否在当前帧被按下 |
| `IsKeyJustReleased()` | int key | bool | 返回按键是否在当前帧被释放 |
| `RegisterKeyCallback()` | int key, function, bool, float | void | 注册按键事件回调函数 |
| `Update()` | float deltaTime | void | 更新按键状态，必须每帧调用 |

---

## Lighting 模块接口

### LightWithAttenuation 类 ⭐ NEW

**文件**: `include/Renderer/Lighting/Light.hpp`

**描述**: 带衰减的光源基类，消除 PointLight 和 SpotLight 之间的代码重复。

**设计要点**:
- 遵循 DRY 原则（Don't Repeat Yourself）
- 提供位置和衰减参数的公共实现
- 支持虚函数多态调用 GetEffectiveRange()

```cpp
class LightWithAttenuation : public Light {
public:
    struct Attenuation {
        float constant, linear, quadratic;
        static Attenuation Range7();
        static Attenuation Range13();
        static Attenuation Range20();
        static Attenuation Range32();
        static Attenuation Range50();
        static Attenuation Range65();
        static Attenuation Range100();
    };

    // 位置和衰减（公共属性）
    const glm::vec3 &GetPosition() const;
    void SetPosition(const glm::vec3 &position);
    const Attenuation &GetAttenuation() const;
    void SetAttenuation(const Attenuation &attenuation);

    // ⭐ 虚函数：支持多态
    virtual float GetEffectiveRange() const;

protected:
    glm::vec3 m_position;
    Attenuation m_attenuation;
};
```

**使用示例**:
```cpp
// 多态调用
LightWithAttenuation* lights[] = {
    new PointLight(/* attenuation = Range32 */),
    new SpotLight(/* attenuation = Range32, cutOff = 12.5° */)
};

for (auto* light : lights) {
    float range = light->GetEffectiveRange();  // ⭐ 虚函数调用
}
```

---

### LightHandle 类 ⭐ NEW

**文件**: `include/Renderer/Lighting/Light.hpp`

**描述**: 光源句柄，提供稳定的引用机制，替代容易失效的索引系统。

**设计要点**:
- 使用稳定的 `id + generation` 机制（避免索引失效问题）
- 禁用拷贝，仅可移动（避免意外复制）
- 类型安全，包含光源类型标签
- 线程安全支持

```cpp
class LightHandle {
public:
    LightHandle();  // 默认构造无效句柄

    // 访问器
    size_t GetId() const;           // 获取稳定ID
    size_t GetGeneration() const;   // 获取代数标记
    LightType GetType() const;      // 获取光源类型

    // 有效性检查
    bool IsValid() const;           // 检查句柄是否有效

    // 禁用拷贝，仅可移动
    LightHandle(const LightHandle&) = delete;
    LightHandle& operator=(const LightHandle&) = delete;
    LightHandle(LightHandle&&) noexcept = default;
    LightHandle& operator=(LightHandle&&) noexcept = default;
};
```

**使用示例**:
```cpp
// 添加光源，返回LightHandle
auto pointLight = std::make_shared<PointLight>(...);
LightHandle handle = lightManager.AddPointLight(pointLight);

// 使用句柄获取光源
auto light = lightManager.GetPointLight(handle);
if (light) {
    light->SetIntensity(10.0f);
}

// 使用句柄移除光源
lightManager.RemovePointLight(handle);
```

**线程安全**: 是（所有操作都是只读的，完全线程安全）

---

### Light 类

光照系统基类，定义了所有光源的通用属性和接口。

```cpp
namespace Renderer {
namespace Lighting {

enum class LightType {
    DIRECTIONAL,  // 平行光（方向光，如太阳光）
    POINT,        // 点光源（从一个点向所有方向发光，如灯泡）
    SPOT          // 聚光灯（从一个点向特定方向锥形发光）
};

class Light {
public:
    // 构造函数
    Light(
        const glm::vec3 &color = glm::vec3(1.0f),
        float intensity = 1.0f,
        float ambient = 0.1f,
        float diffuse = 0.8f,
        float specular = 0.5f);

    virtual ~Light() = default;

    // 通用属性访问
    const glm::vec3 &GetColor() const;
    void SetColor(const glm::vec3 &color);

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

    // 虚函数接口（派生类实现）
    virtual LightType GetType() const = 0;
    virtual void ApplyToShader(Shader &shader, int index = 0) const = 0;
    virtual std::string GetDescription() const = 0;
};

} // namespace Lighting
} // namespace Renderer
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Light()` | color, intensity, ambient, diffuse, specular | - | 构造光源，设置颜色和光照分量 |
| `GetColor()` | 无 | const glm::vec3& | 获取光源颜色 |
| `SetColor()` | color | void | 设置光源颜色 |
| `GetIntensity()` | 无 | float | 获取光照强度 |
| `SetIntensity()` | intensity | void | 设置光照强度 |
| `IsEnabled()` | 无 | bool | 检查光源是否开启 |
| `SetEnabled()` | enabled | void | 设置光源开关 |
| `Toggle()` | 无 | void | 切换光源开关状态 |
| `GetAmbient()` | 无 | float | 获取环境光分量 |
| `SetAmbient()` | ambient | void | 设置环境光分量 |
| `GetDiffuse()` | 无 | float | 获取漫反射分量 |
| `SetDiffuse()` | diffuse | void | 设置漫反射分量 |
| `GetSpecular()` | 无 | float | 获取镜面反射分量 |
| `SetSpecular()` | specular | void | 设置镜面反射分量 |
| `GetType()` | 无 | LightType | 获取光源类型（纯虚函数） |
| `ApplyToShader()` | shader, index | void | 将光源数据传递给着色器（纯虚函数） |
| `GetDescription()` | 无 | string | 获取光源描述（纯虚函数） |

---

### DirectionalLight 类

平行光（方向光），如太阳光。

```cpp
namespace Renderer {
namespace Lighting {

class DirectionalLight : public Light {
public:
    DirectionalLight(
        const glm::vec3 &direction = glm::vec3(0.0f, -1.0f, 0.0f),
        const glm::vec3 &color = glm::vec3(1.0f),
        float intensity = 1.0f);

    // 方向控制
    const glm::vec3 &GetDirection() const;
    void SetDirection(const glm::vec3 &direction);

    // 接口实现
    LightType GetType() const override;
    void ApplyToShader(Shader &shader, int index = 0) const override;
    std::string GetDescription() const override;
};

} // namespace Lighting
} // namespace Renderer
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `DirectionalLight()` | direction, color, intensity | - | 构造平行光，设置方向和颜色 |
| `GetDirection()` | 无 | const glm::vec3& | 获取光照方向 |
| `SetDirection()` | direction | void | 设置光照方向 |

---

### PointLight 类 ⭐ UPDATED

点光源，从一个点向所有方向发光（如灯泡）。

**架构更新**: 继承 `LightWithAttenuation` 基类，消除代码重复。

```cpp
namespace Renderer {
namespace Lighting {

class PointLight : public LightWithAttenuation {
public:
    using Attenuation = LightWithAttenuation::Attenuation;  // 类型别名

    PointLight(
        const glm::vec3 &position,
        const glm::vec3 &color = glm::vec3(1.0f),
        float intensity = 1.0f,
        float ambient = 0.0f,
        float diffuse = 0.8f,
        float specular = 1.0f,
        const Attenuation &attenuation = Attenuation());

    // 位置和衰减（继承自 LightWithAttenuation）
    // GetPosition() / SetPosition()
    // GetAttenuation() / SetAttenuation()

    // ⭐ 重写虚函数（支持多态）
    float GetEffectiveRange() const override;

    // Light接口实现
    LightType GetType() const override;
    void ApplyToShader(Shader &shader, int index = 0) const override;
    std::string GetDescription() const override;
};

} // namespace Lighting
} // namespace Renderer
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `PointLight()` | position, color, intensity, ... | - | 构造点光源，设置位置和衰减参数 |
| `GetPosition()` | 无 | const glm::vec3& | 获取光源位置（继承自 LightWithAttenuation） |
| `SetPosition()` | position | void | 设置光源位置（继承自 LightWithAttenuation） |
| `GetAttenuation()` | 无 | const Attenuation& | 获取衰减参数（继承自 LightWithAttenuation） |
| `SetAttenuation()` | attenuation | void | 设置衰减参数（继承自 LightWithAttenuation） |
| `GetEffectiveRange()` | 无 | float | ⭐ 虚函数：计算有效距离（重写） |
| `Range7()` ~ `Range100()` | 无 | Attenuation | 预设衰减范围（静态方法） |

---

### SpotLight 类 ⭐ UPDATED

聚光灯，从一个点向特定方向锥形发光。

**架构更新**: 继承 `LightWithAttenuation` 基类（消除代码重复），支持多态。

```cpp
namespace Renderer {
namespace Lighting {

class SpotLight : public LightWithAttenuation {
public:
    using Attenuation = LightWithAttenuation::Attenuation;  // 类型别名

    SpotLight(
        const glm::vec3 &position,
        const glm::vec3 &direction,
        const glm::vec3 &color = glm::vec3(1.0f),
        float intensity = 1.0f,
        float ambient = 0.0f,
        float diffuse = 0.8f,
        float specular = 1.0f,
        const Attenuation &attenuation = LightWithAttenuation::Attenuation(),
        float cutOff = glm::radians(12.5f),      // ⚠️ 注意：现在使用弧度
        float outerCutOff = glm::radians(17.5f));

    // ========================================
    // 位置和衰减（继承自 LightWithAttenuation）
    // ========================================
    // ⭐ 不再需要重复实现，直接使用基类的：
    // - GetPosition() / SetPosition()
    // - GetAttenuation() / SetAttenuation()

    // ========================================
    // 方向属性（SpotLight 特有）
    // ========================================
    const glm::vec3 &GetDirection() const;
    void SetDirection(const glm::vec3 &direction);

    // ========================================
    // 截止角度（SpotLight 特有）
    // ========================================
    float GetCutOff() const;
    void SetCutOff(float cutOff);

    float GetOuterCutOff() const;
    void SetOuterCutOff(float outerCutOff);

    float GetCutOffDegrees() const;
    void SetCutOffDegrees(float degrees);

    float GetOuterCutOffDegrees() const;
    void SetOuterCutOffDegrees(float degrees);

    // ========================================
    // ⭐ 重写虚函数（支持多态）
    // ========================================
    float GetEffectiveRange() const override;

    // 接口实现
    LightType GetType() const override;
    void ApplyToShader(Shader &shader, int index = 0) const override;
    std::string GetDescription() const override;
};

} // namespace Lighting
} // namespace Renderer
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `SpotLight()` | position, direction, color, ... | - | 构造聚光灯，设置位置、方向和锥角（弧度） |
| `GetPosition()` | 无 | const glm::vec3& | 获取光源位置（继承自 LightWithAttenuation） |
| `SetPosition()` | position | void | 设置光源位置（继承自 LightWithAttenuation） |
| `GetDirection()` | 无 | const glm::vec3& | 获取光照方向 |
| `SetDirection()` | direction | void | 设置光照方向 |
| `GetEffectiveRange()` | 无 | float | ⭐ 虚函数：计算有效距离（距离+角度衰减） |
| `GetCutOff()` | 无 | float | 获取内锥角（弧度） |
| `SetCutOff()` | cutOff | void | 设置内锥角（弧度） |
| `GetCutOffDegrees()` | 无 | float | 获取内锥角（度数） |
| `SetCutOffDegrees()` | degrees | void | 设置内锥角（度数） |
| `GetOuterCutOff()` | 无 | float | 获取外锥角（余弦值） |
| `SetOuterCutOff()` | outerCutOff | void | 设置外锥角（余弦值） |

---

### LightManager 类 ⭐ UPDATED

光照管理器，统一管理所有光源（单例模式）。

**重大更新**:
- ✅ 线程安全：使用 `std::shared_mutex` 支持读写并发
- ✅ 稳定引用：使用 `LightHandle` 替代容易失效的索引
- ✅ 修复ODR违规：使用 `inline static constexpr`
- ✅ 修复Uniform未初始化：禁用光源时设置零值

```cpp
namespace Renderer {
namespace Lighting {

class LightManager {
public:
    // 获取单例实例
    static LightManager &GetInstance();

    // 光源数量限制（与着色器中的数组大小对应）
    inline static constexpr int MAX_DIRECTIONAL_LIGHTS = 4;
    inline static constexpr int MAX_POINT_LIGHTS = 48;
    inline static constexpr int MAX_SPOT_LIGHTS = 8;

    // ⭐ 添加光源（返回LightHandle）
    LightHandle AddDirectionalLight(const DirectionalLightPtr &light);
    LightHandle AddPointLight(const PointLightPtr &light);
    LightHandle AddSpotLight(const SpotLightPtr &light);

    // ⭐ 移除光源（使用LightHandle）
    bool RemoveDirectionalLight(const LightHandle &handle);
    bool RemovePointLight(const LightHandle &handle);
    bool RemoveSpotLight(const LightHandle &handle);

    // ⭐ 获取光源（使用LightHandle）
    DirectionalLightPtr GetDirectionalLight(const LightHandle &handle);
    PointLightPtr GetPointLight(const LightHandle &handle);
    SpotLightPtr GetSpotLight(const LightHandle &handle);

    // 清空所有光源
    void ClearAll();

    // 查询光源数量
    int GetDirectionalLightCount() const;
    int GetPointLightCount() const;
    int GetSpotLightCount() const;
    int GetTotalLightCount() const;

    // 应用所有光源到着色器（线程安全）
    void ApplyToShader(Shader &shader) const;

    // 调试信息
    std::string GetStatistics() const;
    void PrintAllLights() const;

private:
    LightManager() = default;  // 私有构造函数（单例）
};

} // namespace Lighting
} // namespace Renderer
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `GetInstance()` | 无 | LightManager& | 获取单例实例 |
| `AddDirectionalLight()` | shared_ptr<DirectionalLight> | LightHandle | 添加平行光，返回稳定句柄 |
| `AddPointLight()` | shared_ptr<PointLight> | LightHandle | 添加点光源，返回稳定句柄 |
| `AddSpotLight()` | shared_ptr<SpotLight> | LightHandle | 添加聚光灯，返回稳定句柄 |
| `RemoveDirectionalLight()` | LightHandle | bool | 移除平行光（使用句柄） |
| `RemovePointLight()` | LightHandle | bool | 移除点光源（使用句柄） |
| `RemoveSpotLight()` | LightHandle | bool | 移除聚光灯（使用句柄） |
| `GetDirectionalLight()` | LightHandle | shared_ptr | 获取平行光（使用句柄） |
| `GetPointLight()` | LightHandle | shared_ptr | 获取点光源（使用句柄） |
| `GetSpotLight()` | LightHandle | shared_ptr | 获取聚光灯（使用句柄） |
| `ClearAll()` | 无 | void | 清空所有光源 |
| `GetDirectionalLightCount()` | 无 | int | 获取平行光数量 |
| `GetPointLightCount()` | 无 | int | 获取点光源数量 |
| `GetSpotLightCount()` | 无 | int | 获取聚光灯数量 |
| `GetTotalLightCount()` | 无 | int | 获取总光源数量 |
| `ApplyToShader()` | shader | void | 将所有光源应用到着色器（线程安全） |
| `GetStatistics()` | 无 | string | 获取统计信息 |
| `PrintAllLights()` | 无 | void | 打印所有光源信息 |

**线程安全**: 所有公共方法都是线程安全的
- 读操作使用共享锁（允许并发读）
- 写操作使用独占锁
- `ApplyToShader` 可在渲染线程并发调用

#### 使用示例 ⭐ UPDATED

```cpp
// 1. 创建光照管理器
auto& lightManager = Renderer::Lighting::LightManager::GetInstance();

// 2. 添加平行光（太阳光），返回LightHandle
auto dirLight = std::make_shared<Renderer::Lighting::DirectionalLight>(
    glm::vec3(0.0f, -1.0f, -0.3f),  // 方向
    glm::vec3(1.0f, 0.95f, 0.9f),     // 颜色（暖白光）
    0.5f                             // 强度
);
Renderer::Lighting::LightHandle dirHandle = lightManager.AddDirectionalLight(dirLight);

// 3. 添加点光源（彩色灯球），返回LightHandle
auto pointLight1 = std::make_shared<Renderer::Lighting::PointLight>(
    glm::vec3(0.0f, 8.0f, 0.0f),     // 位置
    glm::vec3(1.0f, 0.1f, 0.1f),     // 红色
    8.0f,                            // 强度
    0.0f, 0.8f, 1.0f,                // ambient, diffuse, specular
    Renderer::Lighting::PointLight::Attenuation::Range32()  // 32米衰减
);
Renderer::Lighting::LightHandle pointHandle = lightManager.AddPointLight(pointLight1);

// 4. 使用句柄获取光源并修改
auto light = lightManager.GetPointLight(pointHandle);
if (light) {
    light->SetIntensity(10.0f);
}

// 5. 添加聚光灯（手电筒）
auto flashlight = std::make_shared<Renderer::Lighting::SpotLight>(
    camera.GetPosition(),            // 位置（跟随摄像机）
    camera.GetFront(),               // 方向（摄像机朝向）
    glm::vec3(1.0f, 1.0f, 1.0f),    // 白色
    1.0f,                            // 强度
    0.0f, 0.8f, 1.0f,                // ambient, diffuse, specular
    Renderer::Lighting::PointLight::Attenuation::Range50(),  // 50米衰减
    glm::radians(12.5f),            // ⚠️ 注意：现在使用弧度
    glm::radians(17.5f)
);
Renderer::Lighting::LightHandle spotHandle = lightManager.AddSpotLight(flashlight);

// 6. 使用句柄移除光源
lightManager.RemovePointLight(pointHandle);

// 7. 在渲染循环中应用光源（线程安全）
shader.Use();
lightManager.ApplyToShader(shader);
```

---

## Renderer 模块接口

### ⚠️ IMesh 抽象接口 - 已废弃（2026-01-02）

**状态**：❌ 已删除
**原因**：Geometry 模块已重构为纯静态工具类

**历史说明**：
- IMesh 接口曾定义网格的基本操作（`Create()`, `Draw()` 等）
- 所有几何体类（Cube, Sphere, Plane, Torus, OBJModel）继承此接口
- 2026-01-02 重构后，所有几何体类改为纯静态工具类，不再需要此接口

**新架构**：
- 几何体类只负责数据生成（纯静态方法）
- MeshDataFactory 负责创建 GPU 资源
- InstancedRenderer 负责渲染逻辑

```cpp
// ❌ 旧接口（已删除）
class IMesh {
    virtual void Create() = 0;
    virtual void Draw() const = 0;
    virtual ~IMesh() = default;
    // ... 其他方法
};

// ✅ 新接口（纯静态类）
class Cube {
    Cube() = delete;  // 禁止实例化
    static std::vector<float> GetVertexData();
    static void GetVertexLayout(...);
};
```

---

### IRenderer 抽象接口

渲染器的抽象基类，定义了渲染器的统一接口。

```cpp
namespace Renderer {
class IRenderer {
public:
    virtual ~IRenderer() = default;

    // 初始化渲染器（创建 OpenGL 缓冲区等）
    virtual void Initialize() = 0;

    // 执行渲染
    virtual void Render() const = 0;

    // 获取渲染器名称（用于调试）
    virtual std::string GetName() const = 0;
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Initialize()` | 无 | void | 初始化渲染器，创建OpenGL缓冲区 |
| `Render()` | 无 | void | 执行渲染操作 |
| `GetName()` | 无 | string | 获取渲染器名称（用于调试） |

#### 设计说明

当前架构采用**分离关注点**设计：

- **几何体类（静态）**：只负责数据生成（Cube, Sphere, Plane, Torus, OBJModel）
- **MeshDataFactory**：负责创建 GPU 资源（MeshBuffer）
- **IRenderer 实现**：负责渲染逻辑（InstancedRenderer）

---

### MeshFactory 工厂类 - 已废弃（2026-01-02）

**状态**：❌ 已删除
**原因**：几何体类改为纯静态类，不再需要运行时工厂

**历史说明**：
- MeshFactory 支持运行时注册和创建不同类型的网格
- 2026-01-02 重构后，通过编译时工厂方法（MeshDataFactory）创建对象

**新方式**：

```cpp
// ❌ 旧方式（已删除）
auto mesh = MeshFactory::Create("Cube");

// ✅ 新方式：直接使用静态方法
auto cubeData = Cube::GetVertexData();

// ✅ 或使用 MeshDataFactory
auto cubeBuffer = MeshDataFactory::CreateCubeBuffer();
```

---

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Register()` | string type, function creator | void | 注册新的网格类型及其创建函数 |
| `Create()` | string type | unique_ptr<IMesh> | 创建指定类型的网格对象 |

---

### Shader 类

OpenGL着色器程序的管理类。

```cpp
namespace Renderer {
class Shader {
public:
    Shader() = default;
    ~Shader();

    // 着色器加载
    void Load(const std::string& vertexPath, const std::string& fragmentPath);
    void Use() const;

    // Uniform 设置 - 基础类型
    void SetMat4(const std::string& name, const glm::mat4& mat) const;
    void SetVec3(const std::string& name, const glm::vec3& vec) const;
    void SetFloat(const std::string& name, float value) const;
    void SetInt(const std::string& name, int value) const;
    void SetBool(const std::string& name, bool value) const;

    // 资源管理
    unsigned int GetID() const;
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Load()` | string vertexPath, string fragmentPath | void | 编译并链接顶点和片段着色器 |
| `Use()` | 无 | void | 激活当前着色器程序 |
| `SetMat4()` | string name, glm::mat4 | void | 设置4x4矩阵uniform变量 |
| `SetVec3()` | string name, glm::vec3 | void | 设置3D向量uniform变量 |
| `GetID()` | 无 | unsigned int | 返回OpenGL着色器程序ID |

---

### Texture 类

纹理加载和管理的封装类。

```cpp
namespace Renderer {
class Texture {
public:
    Texture();
    ~Texture();

    // 纹理加载
    bool LoadFromFile(const std::string& filepath);

    // 纹理操作
    void Bind(GLenum textureUnit = GL_TEXTURE0) const;
    void Unbind() const;
    static void UnbindStatic();

    // 属性查询
    GLuint GetID() const;
    bool IsLoaded() const;
    const std::string& GetFilePath() const;
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `LoadFromFile()` | string filepath | bool | 从文件加载纹理，支持PNG、JPG、BMP格式 |
| `Bind()` | GLenum textureUnit | void | 将纹理绑定到指定的纹理单元 |
| `Unbind()` | 无 | void | 解绑当前纹理 |
| `GetID()` | 无 | GLuint | 返回OpenGL纹理对象ID |
| `IsLoaded()` | 无 | bool | 检查纹理是否成功加载 |

---

### Cube 类

namespace Renderer {
class Shader {
public:
    Shader() = default;
    ~Shader();

    // 着色器加载
    void Load(const std::string& vertexPath, const std::string& fragmentPath);
    void Use() const;

    // Uniform 设置 - 基础类型
    void SetMat4(const std::string& name, const glm::mat4& mat) const;
    void SetVec3(const std::string& name, const glm::vec3& vec) const;
    void SetFloat(const std::string& name, float value) const;
    void SetInt(const std::string& name, int value) const;
    void SetBool(const std::string& name, bool value) const;

    // 资源管理
    unsigned int GetID() const;
};
}

```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Load()` | string vertexPath, string fragmentPath | void | 编译并链接顶点和片段着色器 |
| `Use()` | 无 | void | 激活当前着色器程序 |
| `SetMat4()` | string name, glm::mat4 | void | 设置4x4矩阵uniform变量 |
| `SetVec3()` | string name, glm::vec3 | void | 设置3D向量uniform变量 |
| `GetID()` | 无 | unsigned int | 返回OpenGL着色器程序ID |

---

### Texture 类

纹理加载和管理的封装类。

```cpp
namespace Renderer {
class Texture {
public:
    Texture();
    ~Texture();

    // 纹理加载
    bool LoadFromFile(const std::string& filepath);

    // 纹理操作
    void Bind(GLenum textureUnit = GL_TEXTURE0) const;
    void Unbind() const;
    static void UnbindStatic();

    // 属性查询
    GLuint GetID() const;
    bool IsLoaded() const;
    const std::string& GetFilePath() const;
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `LoadFromFile()` | string filepath | bool | 从文件加载纹理，支持PNG、JPG、BMP格式 |
| `Bind()` | GLenum textureUnit | void | 将纹理绑定到指定的纹理单元 |
| `Unbind()` | 无 | void | 解绑当前纹理 |
| `GetID()` | 无 | GLuint | 返回OpenGL纹理对象ID |
| `IsLoaded()` | 无 | bool | 检查纹理是否成功加载 |

---

### Cube 类 ⭐ UPDATED (2026-01-02)

**纯静态工具类** - 提供立方体顶点数据生成。

**架构更新**：
- ❌ 删除实例方法和成员变量
- ✅ 纯静态方法，禁止实例化
- ✅ 只负责数据生成，不涉及GPU操作

```cpp
namespace Renderer {
class Cube {
public:
    Cube() = delete;  // 禁止实例化

    // 获取顶点数据
    static std::vector<float> GetVertexData();

    // 获取顶点布局
    static void GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes);
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `GetVertexData()` | 无 | vector<float> | 获取立方体顶点数据（24个顶点，每个8个float：位置3+法线3+UV2） |
| `GetVertexLayout()` | offsets, sizes (引用) | void | 获取顶点属性布局（offsets={0,3,6}, sizes={3,3,2}） |

#### 使用示例

```cpp
// ✅ 新方式：使用静态方法获取数据
auto vertices = Renderer::Cube::GetVertexData();

std::vector<size_t> offsets;
std::vector<int> sizes;
Renderer::Cube::GetVertexLayout(offsets, sizes);

// 创建 MeshData 并上传到GPU
Renderer::MeshData cubeData;
cubeData.SetVertices(std::move(vertices), 8);
cubeData.SetVertexLayout(offsets, sizes);

Renderer::MeshBuffer cubeBuffer;
cubeBuffer.UploadToGPU(std::move(cubeData));

// ✅ 或使用工厂方法（推荐）
auto cubeBuffer = Renderer::MeshBufferFactory::CreateCubeBuffer();
```

---

### Sphere 类 ⭐ UPDATED (2026-01-02)

**纯静态工具类** - 提供球体顶点和索引数据生成（支持参数化）。

**架构更新**：
- ❌ 删除实例方法（Create, Draw, SetPosition等）
- ✅ 纯静态方法，禁止实例化
- ✅ 支持参数化（半径、分段数）

```cpp
namespace Renderer {
class Sphere {
public:
    Sphere() = delete;  // 禁止实例化

    // 获取顶点数据（支持参数化）
    static std::vector<float> GetVertexData(float radius = 1.0f, int stacks = 32, int slices = 32);

    // 获取索引数据（支持参数化）
    static std::vector<unsigned int> GetIndexData(int stacks = 32, int slices = 32);

    // 获取顶点布局
    static void GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes);
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `GetVertexData()` | radius, stacks, slices | vector<float> | 获取球体顶点数据（位置3+法线3+UV2） |
| `GetIndexData()` | stacks, slices | vector<uint> | 获取球体索引数据 |
| `GetVertexLayout()` | offsets, sizes (引用) | void | 获取顶点属性布局 |

#### 使用示例

```cpp
// ✅ 新方式：使用静态方法（支持参数化）
auto vertices = Renderer::Sphere::GetVertexData(1.0f, 64, 64);
auto indices = Renderer::Sphere::GetIndexData(64, 64);

std::vector<size_t> offsets;
std::vector<int> sizes;
Renderer::Sphere::GetVertexLayout(offsets, sizes);

// 创建 MeshData
Renderer::MeshData sphereData;
sphereData.SetVertices(std::move(vertices), 8);
sphereData.SetIndices(std::move(indices));
sphereData.SetVertexLayout(offsets, sizes);

// ✅ 或使用工厂方法（推荐）
auto sphereBuffer = Renderer::MeshBufferFactory::CreateSphereBuffer(32, 32, 1.0f);
```

---

### Torus 类 ⭐ UPDATED (2026-01-02)

**纯静态工具类** - 提供圆环体顶点和索引数据生成（支持参数化）。

**架构更新**：
- ❌ 删除实例方法（Create, Draw, SetPosition等）
- ✅ 纯静态方法，禁止实例化
- ✅ 支持参数化（主半径、管半径、分段数）

```cpp
namespace Renderer {
class Torus {
public:
    Torus() = delete;  // 禁止实例化

    // 获取顶点数据（支持参数化）
    static std::vector<float> GetVertexData(
        float majorRadius = 1.0f,
        float minorRadius = 0.3f,
        int majorSegments = 32,
        int minorSegments = 24
    );

    // 获取索引数据（支持参数化）
    static std::vector<unsigned int> GetIndexData(
        int majorSegments = 32,
        int minorSegments = 24
    );

    // 获取顶点布局
    static void GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes);
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `GetVertexData()` | majorRadius, minorRadius, majorSegments, minorSegments | vector<float> | 获取圆环顶点数据（位置3+法线3+UV2） |
| `GetIndexData()` | majorSegments, minorSegments | vector<uint> | 获取圆环索引数据 |
| `GetVertexLayout()` | offsets, sizes (引用) | void | 获取顶点属性布局 |

#### 使用示例

```cpp
// ✅ 新方式：使用静态方法（支持参数化）
auto vertices = Renderer::Torus::GetVertexData(2.0f, 0.5f, 64, 48);
auto indices = Renderer::Torus::GetIndexData(64, 48);

std::vector<size_t> offsets;
std::vector<int> sizes;
Renderer::Torus::GetVertexLayout(offsets, sizes);

// 创建 MeshData
Renderer::MeshData torusData;
torusData.SetVertices(std::move(vertices), 8);
torusData.SetIndices(std::move(indices));
torusData.SetVertexLayout(offsets, sizes);

// ✅ 或使用工厂方法（推荐）
auto torusBuffer = Renderer::MeshBufferFactory::CreateTorusBuffer(1.0f, 0.3f, 32, 24);
```

---

### Plane 类 ⭐ UPDATED (2026-01-02)

**纯静态工具类** - 提供平面顶点和索引数据生成（支持参数化）。

**架构更新**：
- ❌ 删除实例方法（Create, Draw, SetPosition等）
- ✅ 纯静态方法，禁止实例化
- ✅ 支持参数化（宽度、高度、分段数）

```cpp
namespace Renderer {
class Plane {
public:
    Plane() = delete;  // 禁止实例化

    // 获取顶点数据（支持参数化）
    static std::vector<float> GetVertexData(
        float width = 1.0f,
        float height = 1.0f,
        int widthSegments = 1,
        int heightSegments = 1
    );

    // 获取索引数据（支持参数化）
    static std::vector<unsigned int> GetIndexData(
        int widthSegments = 1,
        int heightSegments = 1
    );

    // 获取顶点布局
    static void GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes);
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `GetVertexData()` | width, height, widthSegments, heightSegments | vector<float> | 获取平面顶点数据（位置3+法线3+UV2） |
| `GetIndexData()` | widthSegments, heightSegments | vector<uint> | 获取平面索引数据 |
| `GetVertexLayout()` | offsets, sizes (引用) | void | 获取顶点属性布局 |

#### 使用示例

```cpp
// ✅ 新方式：使用静态方法（支持参数化）
auto vertices = Renderer::Plane::GetVertexData(10.0f, 10.0f, 10, 10);
auto indices = Renderer::Plane::GetIndexData(10, 10);

std::vector<size_t> offsets;
std::vector<int> sizes;
Renderer::Plane::GetVertexLayout(offsets, sizes);

// 创建 MeshData
Renderer::MeshData planeData;
planeData.SetVertices(std::move(vertices), 8);
planeData.SetIndices(std::move(indices));
planeData.SetVertexLayout(offsets, sizes);

// ✅ 或使用工厂方法（推荐）
auto planeBuffer = Renderer::MeshBufferFactory::CreatePlaneBuffer(20.0f, 20.0f, 1, 1);
```

---

### InstanceData 类

实例数据容器，存储多个实例的变换和颜色信息。支持脏标记机制以优化 GPU 更新。

```cpp
namespace Renderer {

class InstanceData {
public:
    InstanceData() = default;

    // ============================================================
    // 实例管理接口
    // ============================================================

    // 添加单个实例
    void Add(const glm::vec3& position, const glm::vec3& rotation,
             const glm::vec3& scale, const glm::vec3& color);

    // 批量添加实例
    void AddBatch(const std::vector<glm::mat4>& matrices, const std::vector<glm::vec3>& colors);

    // 清除所有实例
    void Clear();

    // 获取实例数量
    size_t GetCount() const;

    // 判断是否为空
    bool IsEmpty() const;

    // ============================================================
    // 数据访问接口
    // ============================================================

    // 获取模型矩阵数组（const 版本）
    const std::vector<glm::mat4>& GetModelMatrices() const;

    // 获取模型矩阵数组（非 const 版本，可直接修改）
    std::vector<glm::mat4>& GetModelMatrices();

    // 获取颜色数组（const 版本）
    const std::vector<glm::vec3>& GetColors() const;

    // 获取颜色数组（非 const 版本，可直接修改）
    std::vector<glm::vec3>& GetColors();

    // ============================================================
    // 性能优化：脏标记机制（2026-01-02）
    // ============================================================

    // 检查数据是否被修改（需要更新到 GPU）
    bool IsDirty() const;

    // 清除脏标记（数据已同步到 GPU）
    void ClearDirty();

    // 手动标记为脏（数据已修改，需要同步）
    void MarkDirty();

    // 便捷方法：直接设置单个实例的矩阵（自动标记脏）
    void SetModelMatrix(size_t index, const glm::mat4& matrix);

    // 便捷方法：直接设置单个实例的颜色（自动标记脏）
    void SetColor(size_t index, const glm::vec3& color);
};

}
```

#### 接口说明

##### 实例管理接口

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Add()` | position, rotation, scale, color | void | 添加单个实例，自动标记脏 |
| `AddBatch()` | matrices, colors | void | 批量添加实例，自动标记脏 |
| `Clear()` | 无 | void | 清除所有实例，自动标记脏 |
| `GetCount()` | 无 | size_t | 返回实例数量 |
| `IsEmpty()` | 无 | bool | 判断是否为空 |

##### 数据访问接口

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `GetModelMatrices()` | 无 | const vector<glm::mat4>& | 获取模型矩阵数组（只读） |
| `GetModelMatrices()` | 无 | vector<glm::mat4>& | 获取模型矩阵数组（可修改）**⚠️ 需手动标记脏** |
| `GetColors()` | 无 | const vector<glm::vec3>& | 获取颜色数组（只读） |
| `GetColors()` | 无 | vector<glm::vec3>& | 获取颜色数组（可修改）**⚠️ 需手动标记脏** |

##### 脏标记接口（✨ 新增）

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `IsDirty()` | 无 | bool | 检查数据是否被修改，需要更新到 GPU |
| `ClearDirty()` | 无 | void | 清除脏标记，表示数据已同步到 GPU |
| `MarkDirty()` | 无 | void | 手动标记为脏，表示数据已修改，需要同步 |
| `SetModelMatrix()` | index, matrix | void | 直接设置单个实例的矩阵，**自动标记脏** ✨ |
| `SetColor()` | index, color | void | 直接设置单个实例的颜色，**自动标记脏** ✨ |

#### 设计原则

- ✅ 纯数据容器，无 GPU 资源（无 VAO/VBO/EBO）
- ✅ 无渲染能力（无 Draw/Render）
- ✅ **脏标记机制优化**：避免冗余的 GPU 数据传输
- ✅ **自动标记**：`Add()`, `Clear()`, `SetModelMatrix()`, `SetColor()` 自动标记脏
- ✅ **手动控制**：支持手动 `MarkDirty()` 和 `ClearDirty()`

#### 使用示例

```cpp
// 示例 1：基本使用（自动脏标记）
auto instances = std::make_shared<InstanceData>();
instances->Add(glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(1), glm::vec3(1,1,1));
// ✅ 自动标记脏，UpdateInstanceData() 会更新 GPU

InstancedRenderer renderer;
renderer.SetInstances(instances);
renderer.Initialize();

// 渲染循环
while (true) {
    // 如果数据未修改，UpdateInstanceData() 自动跳过 GPU 更新
    renderer.UpdateInstanceData();  // ✅ 脏检查
    renderer.Render();
}
```

```cpp
// 示例 2：手动修改矩阵（需要手动标记脏）
auto& matrices = instances->GetModelMatrices();
matrices[0] = glm::mat4(1.0f);  // ⚠️ 直接修改不标记脏
instances->MarkDirty();          // ✅ 手动标记脏

// 或者使用便捷方法（推荐）
instances->SetModelMatrix(0, glm::mat4(1.0f));  // ✅ 自动标记脏
```

```cpp
// 示例 3：动画更新（批量标记脏）
void UpdateAnimation(InstanceData& instances, float time) {
    auto& matrices = instances.GetModelMatrices();

    // 修改所有矩阵
    for (size_t i = 0; i < instances.GetCount(); ++i) {
        matrices[i] = ComputeAnimationMatrix(i, time);
    }

    // ✅ 批量修改后统一标记脏
    instances.MarkDirty();
}
```

#### 性能优化说明

**脏标记机制**（2026-01-02 实现）：

1. **自动标记**：
   - `Add()`, `AddBatch()`, `Clear()` 自动设置脏标记
   - `SetModelMatrix()`, `SetColor()` 自动设置脏标记
   - 确保数据修改后标记为脏

2. **条件更新**：
   - `InstancedRenderer::UpdateInstanceData()` 检查 `IsDirty()`
   - 如果为 `false`，跳过 GPU 数据传输（节省 40-60% 带宽）
   - 如果为 `true`，更新 GPU 后自动清除脏标记

3. **性能收益**：
   - 动画运行时：性能相同（数据确实在变化）
   - 动画暂停时：帧率 +50%，GPU 带宽节省 100%
   - 静态几何体：完全跳过传输

4. **注意事项**：
   - ⚠️ 直接修改 `GetModelMatrices()` 返回的引用不会自动标记脏
   - ✅ 使用 `SetModelMatrix()` / `SetColor()` 便捷方法
   - ✅ 批量修改后调用 `MarkDirty()`

详见：`docs/fixs/DIRTY_FLAG_OPTIMIZATION_2026.md`

---

### MeshData 类

纯数据容器 - 存储网格的顶点和索引数据（CPU 内存）。

```cpp
namespace Renderer {

class MeshData {
public:
    MeshData() = default;
    ~MeshData() = default;

    // ============================================================
    // 数据设置接口
    // ============================================================

    // 设置顶点数据（左值引用版本）
    void SetVertices(const std::vector<float>& vertices, size_t stride);

    // 设置顶点数据（右值引用版本，移动语义）
    void SetVertices(std::vector<float>&& vertices, size_t stride);

    // 设置索引数据（左值引用版本）
    void SetIndices(const std::vector<unsigned int>& indices);

    // 设置索引数据（右值引用版本，移动语义）
    void SetIndices(std::vector<unsigned int>&& indices);

    // 设置顶点属性布局
    // offsets: 每个属性在顶点中的偏移（float 索引）
    // sizes: 每个属性的大小（float 数量）
    // 例如：位置(3) + 法线(3) + UV(2) => offsets = {0, 3, 6}, sizes = {3, 3, 2}
    void SetVertexLayout(const std::vector<size_t>& offsets, const std::vector<int>& sizes);

    // 设置材质颜色
    void SetMaterialColor(const glm::vec3& color);

    // ============================================================
    // 数据访问接口
    // ============================================================

    const std::vector<float>& GetVertices() const;
    const std::vector<unsigned int>& GetIndices() const;
    size_t GetVertexStride() const;
    size_t GetVertexCount() const;
    size_t GetIndexCount() const;
    bool HasIndices() const;
    const glm::vec3& GetMaterialColor() const;

    const std::vector<size_t>& GetAttributeOffsets() const;
    const std::vector<int>& GetAttributeSizes() const;

    // ============================================================
    // 工具方法
    // ============================================================

    void Clear();
    bool IsEmpty() const;
    size_t GetVertexDataSizeBytes() const;
    size_t GetIndexDataSizeBytes() const;
};

}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `SetVertices()` | vector<float>& vertices, size_t stride | void | 设置顶点数据（左值版本，会拷贝） |
| `SetVertices()` | vector<float>&& vertices, size_t stride | void | 设置顶点数据（右值版本，移动语义，避免拷贝） |
| `SetIndices()` | vector<uint>& indices | void | 设置索引数据（左值版本） |
| `SetIndices()` | vector<uint>&& indices | void | 设置索引数据（右值版本，移动语义） |
| `SetVertexLayout()` | vector<size_t>& offsets, vector<int>& sizes | void | 设置顶点属性布局 |
| `SetMaterialColor()` | glm::vec3& color | void | 设置材质颜色 |
| `GetVertices()` | 无 | const vector<float>& | 获取顶点数据（只读） |
| `GetIndices()` | 无 | const vector<uint>& | 获取索引数据（只读） |
| `GetVertexStride()` | 无 | size_t | 获取顶点步长（float数量） |
| `GetVertexCount()` | 无 | size_t | 获取顶点数量 |
| `GetIndexCount()` | 无 | size_t | 获取索引数量 |
| `HasIndices()` | 无 | bool | 是否有索引数据 |
| `GetMaterialColor()` | 无 | glm::vec3 | 获取材质颜色 |
| `Clear()` | 无 | void | 清空所有数据 |
| `IsEmpty()` | 无 | bool | 检查是否为空 |
| `GetVertexDataSizeBytes()` | 无 | size_t | 计算顶点数据的字节大小 |
| `GetIndexDataSizeBytes()` | 无 | size_t | 计算索引数据的字节大小 |

#### 设计原则

- ✅ 纯数据容器，类似 InstanceData
- ✅ 无 GPU 资源（无 VAO/VBO/EBO）
- ✅ 无渲染能力（无 Draw/Render）
- ✅ 可序列化，可传递，可复制

#### 使用场景

- 作为数据交换格式
- 用于序列化/反序列化
- 传递给 MeshBuffer 上传到 GPU

#### 使用示例

```cpp
// 创建网格数据
Renderer::MeshData meshData;

// 设置顶点数据（位置+法线+UV）
std::vector<float> vertices = {
    // 位置(x,y,z)         法线(nx,ny,nz)      UV(u,v)
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
     0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
     0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.5f, 1.0f
};
meshData.SetVertices(std::move(vertices), 8);  // 8个float每顶点

// 设置索引数据
std::vector<unsigned int> indices = {0, 1, 2};
meshData.SetIndices(std::move(indices));

// 设置顶点布局
std::vector<size_t> offsets = {0, 3, 6};  // 位置、法线、UV的偏移
std::vector<int> sizes = {3, 3, 2};       // 位置3、法线3、UV2
meshData.SetVertexLayout(offsets, sizes);

// 设置材质颜色
meshData.SetMaterialColor(glm::vec3(1.0f, 0.8f, 0.6f));

// 传递给 MeshBuffer 上传到 GPU
Renderer::MeshBuffer meshBuffer;
meshBuffer.UploadToGPU(std::move(meshData));
```

---

### MeshBuffer 类

GPU 资源包装器 - 管理网格的 OpenGL 缓冲区。

```cpp
namespace Renderer {

class MeshBuffer {
public:
    MeshBuffer() = default;
    ~MeshBuffer();

    // ============================================================
    // 拷贝语义（已删除，防止误用）
    // ============================================================

    MeshBuffer(const MeshBuffer& other) = delete;
    MeshBuffer& operator=(const MeshBuffer& other) = delete;

    // ============================================================
    // 移动语义（高效转移资源）
    // ============================================================

    MeshBuffer(MeshBuffer&& other) noexcept;
    MeshBuffer& operator=(MeshBuffer&& other) noexcept;

    // ============================================================
    // GPU 操作
    // ============================================================

    // 上传数据到 GPU（左值引用版本）
    void UploadToGPU(const MeshData& data);

    // 上传数据到 GPU（右值引用版本，移动语义）
    void UploadToGPU(MeshData&& data);

    // 释放 GPU 资源
    void ReleaseGPU();

    // ============================================================
    // 访问接口
    // ============================================================

    unsigned int GetVAO() const;
    size_t GetVertexCount() const;
    size_t GetIndexCount() const;
    bool HasIndices() const;
    const glm::vec3& GetMaterialColor() const;

    // ============================================================
    // 纹理管理
    // ============================================================

    void SetTexture(std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> GetTexture() const;
    bool HasTexture() const;

    // ============================================================
    // 数据访问
    // ============================================================

    const MeshData& GetData() const;
};

}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `MeshBuffer()` | 无 | - | 默认构造函数 |
| `~MeshBuffer()` | 无 | - | 析构函数，自动释放 GPU 资源 |
| `UploadToGPU()` | MeshData& data | void | 上传数据到GPU（左值版本） |
| `UploadToGPU()` | MeshData&& data | void | 上传数据到GPU（右值版本，移动语义） |
| `ReleaseGPU()` | 无 | void | 手动释放 GPU 资源 |
| `GetVAO()` | 无 | unsigned int | 获取 OpenGL VAO ID |
| `GetVertexCount()` | 无 | size_t | 获取顶点数量 |
| `GetIndexCount()` | 无 | size_t | 获取索引数量 |
| `HasIndices()` | 无 | bool | 是否有索引数据 |
| `GetMaterialColor()` | 无 | glm::vec3 | 获取材质颜色 |
| `SetTexture()` | shared_ptr<Texture> | void | 设置纹理 |
| `GetTexture()` | 无 | shared_ptr<Texture> | 获取纹理 |
| `HasTexture()` | 无 | bool | 是否有纹理 |
| `GetData()` | 无 | const MeshData& | 获取底层数据（只读） |

#### 设计原则

- ✅ 持有 CPU 数据副本（支持深拷贝）
- ✅ 管理 GPU 资源（VAO/VBO/EBO）
- ✅ 不继承任何接口（不是可渲染对象）
- ✅ 不提供 Draw/Render 方法（由 Renderer 负责）
- ✅ 只提供 GetVAO() 访问接口
- ❌ 拷贝构造和拷贝赋值已删除（防止意外深拷贝）
- ✅ 支持移动语义（高效转移资源）

#### 使用场景

- 作为 InstancedRenderer 的网格模板
- 提供已上传到 GPU 的 VAO
- 管理网格的纹理资源

#### 使用示例

```cpp
// 示例1：使用 MeshData 创建 MeshBuffer
Renderer::MeshData meshData;
// ... 设置 meshData ...

Renderer::MeshBuffer meshBuffer;
meshBuffer.UploadToGPU(std::move(meshData));  // 移动语义，避免拷贝

// 示例2：设置纹理
auto texture = std::make_shared<Renderer::Texture>();
texture->LoadFromFile("assets/textures/wood.png");
meshBuffer.SetTexture(texture);

// 示例3：传递给 InstancedRenderer
auto meshBufferPtr = std::make_shared<Renderer::MeshBuffer>(std::move(meshBuffer));
Renderer::InstancedRenderer renderer;
renderer.SetMesh(meshBufferPtr);
renderer.SetInstances(instances);
renderer.Initialize();

// 示例4：移动语义
Renderer::MeshBuffer buffer1 = MeshBufferFactory::CreateCubeBuffer();
Renderer::MeshBuffer buffer2 = std::move(buffer1);  // 转移所有权，buffer1变为无效状态
```

---

### ~~SimpleMesh 类~~ （已废弃）

**⚠️ 警告：SimpleMesh 类已被废弃，请使用 MeshData + MeshBuffer 替代**

废弃原因：
- SimpleMesh 混合了数据存储和渲染职责
- 新架构使用 MeshData（CPU数据）+ MeshBuffer（GPU资源）+ InstancedRenderer（渲染逻辑）的分离设计
- 新设计更清晰、更易维护、性能更好

迁移指南：
- 使用 `MeshData` 存储顶点和索引数据
- 使用 `MeshBuffer` 管理 GPU 资源（VAO/VBO/EBO）
- 使用 `MeshBufferFactory::CreateXXXBuffer()` 创建网格缓冲区
- 使用 `InstancedRenderer` 进行渲染

---

### MeshDataFactory 类

网格数据工厂 - 创建各种几何体的 MeshData（CPU 纯数据）。

```cpp
namespace Renderer {

class MeshDataFactory {
public:
    // ============================================================
    // 基础几何体
    // ============================================================

    // 创建立方体数据
    static MeshData CreateCubeData();

    // 创建球体数据
    static MeshData CreateSphereData(int stacks = 32, int slices = 32, float radius = 1.0f);

    // 创建圆环体数据
    static MeshData CreateTorusData(float majorRadius = 1.0f, float minorRadius = 0.3f,
                                   int majorSegments = 32, int minorSegments = 24);

    // 创建平面数据
    static MeshData CreatePlaneData(float width = 1.0f, float height = 1.0f,
                                   int widthSegments = 1, int heightSegments = 1);

    // ============================================================
    // OBJ 模型
    // ============================================================

    // 从 OBJ 文件创建网格数据
    // 返回每个材质对应的 MeshData
    static std::vector<MeshData> CreateOBJData(const std::string& objPath);

    // ============================================================
    // 工具方法
    // ============================================================

    // 从 Cube 对象提取 MeshData
    static MeshData ExtractFromCube(const class Cube& cube);

    // 从 Sphere 对象提取 MeshData
    static MeshData ExtractFromSphere(const class Sphere& sphere);
};

}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `CreateCubeData()` | 无 | MeshData | 创建立方体的顶点数据 |
| `CreateSphereData()` | stacks, slices, radius | MeshData | 创建球体的顶点和索引数据 |
| `CreateTorusData()` | majorRadius, minorRadius, majorSegments, minorSegments | MeshData | 创建圆环体的顶点和索引数据 |
| `CreatePlaneData()` | width, height, widthSegments, heightSegments | MeshData | 创建平面的顶点和索引数据 |
| `CreateOBJData()` | objPath | vector<MeshData> | 从OBJ文件创建网格数据（每个材质一个） |
| `ExtractFromCube()` | Cube& | MeshData | 从Cube对象提取数据 |
| `ExtractFromSphere()` | Sphere& | MeshData | 从Sphere对象提取数据 |

#### 使用示例

```cpp
// 示例1：创建立方体数据
Renderer::MeshData cubeData = Renderer::MeshDataFactory::CreateCubeData();

// 示例2：创建高精度球体数据
Renderer::MeshData sphereData = Renderer::MeshDataFactory::CreateSphereData(64, 64, 1.0f);

// 示例3：从OBJ文件加载
std::vector<Renderer::MeshData> modelData = Renderer::MeshDataFactory::CreateOBJData("assets/models/car.obj");

// 示例4：传递给 MeshBuffer
Renderer::MeshBuffer meshBuffer;
meshBuffer.UploadToGPU(std::move(cubeData));
```

---

### MeshBufferFactory 类

网格缓冲区工厂 - 创建已上传到 GPU 的 MeshBuffer。

```cpp
namespace Renderer {

class MeshBufferFactory {
public:
    // ============================================================
    // 基础几何体（自动上传到 GPU）
    // ============================================================

    // 创建立方体缓冲区（已上传到 GPU）
    static MeshBuffer CreateCubeBuffer();

    // 创建球体缓冲区（已上传到 GPU）
    static MeshBuffer CreateSphereBuffer(int stacks = 32, int slices = 32, float radius = 1.0f);

    // 创建圆环体缓冲区（已上传到 GPU）
    static MeshBuffer CreateTorusBuffer(float majorRadius = 1.0f, float minorRadius = 0.3f,
                                       int majorSegments = 32, int minorSegments = 24);

    // 创建平面缓冲区（已上传到 GPU）
    static MeshBuffer CreatePlaneBuffer(float width = 1.0f, float height = 1.0f,
                                      int widthSegments = 1, int heightSegments = 1);

    // ============================================================
    // OBJ 模型（自动上传到 GPU）
    // ============================================================

    // 从 OBJ 文件创建网格缓冲区（已上传到 GPU）
    static std::vector<MeshBuffer> CreateOBJBuffers(const std::string& objPath);

    // ============================================================
    // 从 MeshData 创建
    // ============================================================

    // 从 MeshData 创建 MeshBuffer 并上传到 GPU（左值引用版本）
    static MeshBuffer CreateFromMeshData(const MeshData& data);

    // 从 MeshData 创建 MeshBuffer 并上传到 GPU（右值引用版本，移动语义）
    static MeshBuffer CreateFromMeshData(MeshData&& data);

    // 批量创建 MeshBuffer 并上传到 GPU
    static std::vector<MeshBuffer> CreateFromMeshDataList(const std::vector<MeshData>& dataList);
};

}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `CreateCubeBuffer()` | 无 | MeshBuffer | 创建立方体GPU缓冲区（已上传） |
| `CreateSphereBuffer()` | stacks, slices, radius | MeshBuffer | 创建球体GPU缓冲区（已上传） |
| `CreateTorusBuffer()` | majorRadius, minorRadius, majorSegments, minorSegments | MeshBuffer | 创建圆环体GPU缓冲区（已上传） |
| `CreatePlaneBuffer()` | width, height, widthSegments, heightSegments | MeshBuffer | 创建平面GPU缓冲区（已上传） |
| `CreateOBJBuffers()` | objPath | vector<MeshBuffer> | 从OBJ创建GPU缓冲区（已上传） |
| `CreateFromMeshData()` | MeshData& | MeshBuffer | 从MeshData创建GPU缓冲区（左值） |
| `CreateFromMeshData()` | MeshData&& | MeshBuffer | 从MeshData创建GPU缓冲区（右值，移动语义） |
| `CreateFromMeshDataList()` | vector<MeshData>& | vector<MeshBuffer> | 批量创建GPU缓冲区 |

#### 使用示例

```cpp
// 示例1：创建立方体缓冲区（最常用）
Renderer::MeshBuffer cubeBuffer = Renderer::MeshBufferFactory::CreateCubeBuffer();

// 示例2：创建球体缓冲区
Renderer::MeshBuffer sphereBuffer = Renderer::MeshBufferFactory::CreateSphereBuffer(32, 32, 1.0f);

// 示例3：从OBJ文件创建并自动上传
std::vector<Renderer::MeshBuffer> carBuffers = Renderer::MeshBufferFactory::CreateOBJBuffers("assets/models/car.obj");

// 示例4：直接用于 InstancedRenderer
auto cubeBufferPtr = std::make_shared<Renderer::MeshBuffer>(
    Renderer::MeshBufferFactory::CreateCubeBuffer()
);
Renderer::InstancedRenderer renderer;
renderer.SetMesh(cubeBufferPtr);
renderer.SetInstances(instances);
renderer.Initialize();

// 示例5：使用移动语义（性能最优）
Renderer::MeshData data = Renderer::MeshDataFactory::CreateCubeData();
Renderer::MeshBuffer buffer = Renderer::MeshBufferFactory::CreateFromMeshData(std::move(data));
```

---

### OBJModel 类 ⭐ UPDATED (2026-01-02)

**纯静态工具类** - 提供OBJ模型数据加载和生成（支持多材质）。

**架构更新**：
- ❌ 删除实例方法（Create, Draw, LoadFromFile等）
- ✅ 纯静态方法，禁止实例化
- ✅ 支持材质分离（每个材质独立的顶点数据）

```cpp
namespace Renderer {
class OBJModel {
public:
    OBJModel() = delete;  // 禁止实例化

    // 材质顶点数据结构
    struct MaterialVertexData {
        std::vector<float> vertices;      // 顶点数据（位置3+法线3+UV2）
        std::vector<unsigned int> indices; // 索引数据
        OBJMaterial material;              // 材质信息
        std::string texturePath;           // 纹理路径
    };

    // 获取材质分离的顶点数据（用于多材质渲染）
    static std::vector<MaterialVertexData> GetMaterialVertexData(const std::string& objPath);

    // 获取单个MeshData（不分离材质）
    static MeshData GetMeshData(const std::string& objPath);

    // 获取材质列表
    static std::vector<OBJMaterial> GetMaterials(const std::string& objPath);

    // 检查是否有材质
    static bool HasMaterials(const std::string& objPath);

    // 获取顶点布局
    static void GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes);
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `GetMaterialVertexData()` | objPath | vector<MaterialVertexData> | 获取材质分离的顶点数据（推荐用于多材质模型） |
| `GetMeshData()` | objPath | MeshData | 获取单个网格数据（合并所有材质） |
| `GetMaterials()` | objPath | vector<OBJMaterial> | 获取材质列表 |
| `HasMaterials()` | objPath | bool | 检查模型是否包含材质 |
| `GetVertexLayout()` | offsets, sizes (引用) | void | 获取顶点属性布局（offsets={0,3,6}, sizes={3,3,2}） |

#### 使用示例

```cpp
// ✅ 新方式：获取材质分离的数据（推荐）
std::string carPath = "assets/models/cars/sportsCar.obj";
auto materialDataList = Renderer::OBJModel::GetMaterialVertexData(carPath);

// 为每个材质创建 MeshBuffer
std::vector<Renderer::MeshBuffer> meshBuffers;
for (const auto& materialData : materialDataList) {
    Renderer::MeshData meshData;
    meshData.SetVertices(std::move(materialData.vertices), 8);
    meshData.SetIndices(std::move(materialData.indices));

    std::vector<size_t> offsets = {0, 3, 6};
    std::vector<int> sizes = {3, 3, 2};
    meshData.SetVertexLayout(offsets, sizes);
    meshData.SetMaterialColor(glm::vec3(
        materialData.material.diffuse[0],
        materialData.material.diffuse[1],
        materialData.material.diffuse[2]
    ));

    Renderer::MeshBuffer buffer;
    buffer.UploadToGPU(std::move(meshData));

    // 加载纹理（如果有）
    if (!materialData.texturePath.empty()) {
        auto texture = std::make_shared<Renderer::Texture>();
        texture->LoadFromFile(materialData.texturePath);
        buffer.SetTexture(texture);
    }

    meshBuffers.push_back(std::move(buffer));
}

// ✅ 或使用工厂方法（推荐）
auto meshBuffers = Renderer::MeshBufferFactory::CreateOBJBuffers(carPath);
```

---

### InstancedRenderer 类

实例化渲染器 - 负责批量渲染多个相同几何体，大幅提升渲染性能。继承IRenderer接口，采用职责分离设计：MeshBuffer 管理GPU资源，InstanceData 存储实例数据，InstancedRenderer 执行渲染逻辑。使用shared_ptr管理资源生命周期。

```cpp
namespace Renderer {

class InstancedRenderer : public IRenderer {
public:
    InstancedRenderer();
    ~InstancedRenderer();

    // IRenderer接口实现
    void Initialize() override;
    void Render() const override;
    std::string GetName() const override;

    // 设置网格模板（使用 shared_ptr 管理所有权）
    void SetMesh(std::shared_ptr<MeshBuffer> mesh);

    // 设置实例数据（使用 shared_ptr 避免拷贝）
    void SetInstances(const std::shared_ptr<InstanceData>& data);

    // 信息查询
    size_t GetInstanceCount() const;

    // 更新实例数据到GPU（用于动画）
    void UpdateInstanceData();

    // 静态工厂方法：为 Cube 创建实例化渲染器
    static InstancedRenderer CreateForCube(const std::shared_ptr<InstanceData>& instances);

    // 静态工厂方法：为 OBJ 模型创建实例化渲染器（返回多个渲染器，每个材质一个）
    static std::tuple<std::vector<InstancedRenderer>,
                      std::vector<std::shared_ptr<MeshBuffer>>,
                      std::shared_ptr<InstanceData>>
    CreateForOBJ(const std::string& objPath, const std::shared_ptr<InstanceData>& instances);

    // 禁用拷贝（但允许移动，用于放入vector）
    InstancedRenderer(const InstancedRenderer&) = delete;
    InstancedRenderer& operator=(const InstancedRenderer&) = delete;

    // 允许移动构造和移动赋值
    InstancedRenderer(InstancedRenderer&&) noexcept = default;
    InstancedRenderer& operator=(InstancedRenderer&&) noexcept = default;
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Initialize()` | 无 | void | 初始化实例化渲染器，上传实例数据并设置实例化属性 |
| `Render()` | 无 | void | 执行实例化渲染（glDrawElementsInstanced 或 glDrawArraysInstanced） |
| `SetMesh()` | shared_ptr<MeshBuffer> mesh | void | 设置网格模板（使用 shared_ptr 管理所有权） |
| `SetInstances()` | shared_ptr<InstanceData> data | void | 设置实例数据（使用 shared_ptr 避免拷贝） |
| `UpdateInstanceData()` | 无 | void | 更新实例数据到GPU，用于动画（使用glBufferSubData） |
| `CreateForCube()` | shared_ptr<InstanceData> instances | InstancedRenderer | 静态方法：创建立方体实例化渲染器 |
| `CreateForOBJ()` | string objPath, shared_ptr<InstanceData> instances | tuple<渲染器vector, meshBuffer的shared_ptrvector, instanceData的shared_ptr> | 静态方法：从OBJ模型创建多个材质渲染器 |

#### 职责分离设计

**新架构（职责完全分离）**：

- ✅ **MeshBuffer**: GPU资源包装器，管理VAO/VBO/EBO和CPU数据副本
- ✅ **InstanceData**: 实例数据容器，负责存储实例变换和颜色
- ✅ **InstancedRenderer**: 渲染逻辑，负责批量渲染多个实例，继承IRenderer接口

**所有权管理**：

- `InstancedRenderer` 使用 `shared_ptr<MeshBuffer>` 管理网格生命周期
- `InstancedRenderer` 使用 `shared_ptr<InstanceData>` 避免拷贝
- 纹理由 `MeshBuffer` 持有（shared_ptr<Texture>）
- `CreateForOBJ()` 返回 tuple<渲染器vector, meshBuffer的shared_ptrvector, instanceData的shared_ptr>
- 主程序需保持 meshBuffer 和 instanceData 的 shared_ptr 存活
- 自动内存管理，消除悬空指针风险

#### 功能特性

**实例化渲染**：

- 单次绘制调用渲染数百个相同几何体
- 每个实例独立的变换矩阵（位置、旋转、缩放）
- 每个实例独立的颜色属性
- 使用 glVertexAttribDivisor 实现属性实例化

**材质支持**：

- 支持纹理映射（由 MeshBuffer 管理）
- 支持材质颜色（从OBJ文件的.mtl文件读取）
- 多材质OBJ模型：为每个材质创建独立的渲染器

**内存管理**：

- 使用 shared_ptr 自动管理 MeshBuffer 生命周期
- 主程序需要保持 meshBuffer 的 shared_ptr 存活
- 实例数据存储在 InstanceData 中
- 支持动态更新实例缓冲（UpdateInstanceData）

**动画支持**：

- `UpdateInstanceData()` 方法支持运行时更新实例数据到GPU
- 使用 glBufferSubData 高效更新（不重新分配内存）
- 典型流程：
  1. 修改 `InstanceData` 中的模型矩阵（`instanceData->GetModelMatrices()[i] = newMatrix`）
  2. 调用 `renderer->UpdateInstanceData()` 上传到GPU
  3. 调用 `renderer->Render()` 渲染更新后的实例
- 应用场景：
  - 自转动画：更新每个实例的旋转变换
  - 公转动画：更新每个实例的位置变换
  - 缩放动画：更新每个实例的缩放变换
  - 复杂动画：组合多个变换（Disco舞台）

#### 使用示例

```cpp
// 示例1：从立方体模板创建（推荐方式）
// 1. 创建网格缓冲区（使用工厂）
auto cubeBuffer = std::make_shared<Renderer::MeshBuffer>(
    Renderer::MeshBufferFactory::CreateCubeBuffer()
);

// 2. 准备实例数据（使用 shared_ptr 避免拷贝）
auto cubeInstances = std::make_shared<Renderer::InstanceData>();
for (int x = 0; x < 10; ++x) {
    for (int z = 0; z < 10; ++z) {
        glm::vec3 position(x * 2.0f, 0.0f, z * 2.0f);
        glm::vec3 rotation(0.0f, 0.0f, 0.0f);
        glm::vec3 scale(1.0f, 1.0f, 1.0f);
        glm::vec3 color(1.0f, 0.5f, 0.3f); // 橙色
        cubeInstances->Add(position, rotation, scale, color);
    }
}

// 3. 创建渲染器并初始化
Renderer::InstancedRenderer cubeRenderer;
cubeRenderer.SetMesh(cubeBuffer);  // 传递 shared_ptr
cubeRenderer.SetInstances(cubeInstances);  // 传递 shared_ptr
cubeRenderer.Initialize();

// 4. 渲染
shader.Use();
shader.SetBool("useTexture", false);
shader.SetBool("useInstanceColor", true);
cubeRenderer.Render();

// 示例2：从OBJ模型创建（多材质）
// 1. 准备实例数据（使用 shared_ptr）
auto carInstances = std::make_shared<Renderer::InstanceData>();
for (int i = 0; i < 12; ++i) {
    float angle = (float)i / 12.0f * 3.14159f * 2.0f;
    glm::vec3 position(std::cos(angle) * 15.0f, 0.0f, std::sin(angle) * 15.0f);
    glm::vec3 rotation(0.0f, -angle * 57.2958f + 90.0f, 0.0f);
    glm::vec3 scale(0.5f, 0.5f, 0.5f);
    glm::vec3 color(1.0f, 1.0f, 1.0f); // 白色（使用材质颜色）
    carInstances->Add(position, rotation, scale, color);
}

// 2. 创建渲染器（静态方法自动处理多材质）
std::string carPath = "assets/models/cars/sportsCar.obj";
auto [carRenderers, carMeshBuffers, carInstanceData] =  // 接收渲染器、meshBuffer和instanceData的shared_ptr
    Renderer::InstancedRenderer::CreateForOBJ(carPath, carInstances);

// 3. 渲染（每个材质一个draw call）
for (const auto& carRenderer : carRenderers) {
    shader.Use();
    shader.SetBool("useTexture", carRenderer.HasTexture());
    shader.SetVec3("objectColor", carRenderer.GetMaterialColor());
    shader.SetBool("useInstanceColor", false); // 使用材质颜色
    carRenderer.Render();
}
// 注意：carMeshBuffers 和 carInstanceData 必须保持存活，直到渲染结束
```

#### 示例3：动画效果（自转+公转）

```cpp
// 1. 创建Disco舞台的球体组合
auto sphereInstances = std::make_shared<Renderer::InstanceData>();
auto cubeInstances = std::make_shared<Renderer::InstanceData>();

// 中央球体（核心 + 500个立方体层）
sphereInstances->Add(glm::vec3(0, 8, 0), glm::vec3(0), glm::vec3(1.8), glm::vec3(1.0f));
for (int i = 0; i < 500; ++i) {
    // Fibonacci球算法分布立方体
    float theta = 2.0f * glm::pi<float>() * i / goldenRatio;
    float phi = std::acos(1.0f - 2.0f * (i + 0.5f) / 500);
    glm::vec3 offset = glm::vec3(
        2.5f * std::sin(phi) * std::cos(theta),
        2.5f * std::sin(phi) * std::sin(theta),
        2.5f * std::cos(phi)
    );
    cubeInstances->Add(glm::vec3(0, 8, 0) + offset, glm::vec3(0), glm::vec3(0.35f), glm::vec3(1.0f));
}

// 8个彩色球体（每个包含核心 + 100个立方体）
for (int i = 0; i < 8; ++i) {
    float angle = i * 45.0f;
    float radius = 10.0f;
    glm::vec3 lightCenter(radius * cosf(glm::radians(angle)), 5.0f, radius * sinf(glm::radians(angle)));
    glm::vec3 color = (i % 4 == 0) ? glm::vec3(1, 0.1f, 0.1f) :
                      (i % 4 == 1) ? glm::vec3(0.1f, 1, 0.1f) :
                      (i % 4 == 2) ? glm::vec3(0.1f, 0.1f, 1) : glm::vec3(1, 1, 0.1f);

    sphereInstances->Add(lightCenter, glm::vec3(0), glm::vec3(1.0f + (i % 3) * 0.2f), color * 1.2f);

    for (int j = 0; j < 100; ++j) {
        // Fibonacci球算法
        float theta = 2.0f * glm::pi<float>() * j / goldenRatio;
        float phi = std::acos(1.0f - 2.0f * (j + 0.5f) / 100);
        float lightRadius = 1.0f + (i % 3) * 0.2f;
        glm::vec3 localOffset = glm::vec3(
            lightRadius * std::sin(phi) * std::cos(theta),
            lightRadius * std::sin(phi) * std::sin(theta),
            lightRadius * std::cos(phi)
        );
        cubeInstances->Add(lightCenter + localOffset, glm::vec3(0), glm::vec3(0.2f), color);
    }
}

// 2. 创建网格缓冲区（使用工厂）
auto cubeBufferPtr = std::make_shared<Renderer::MeshBuffer>(
    Renderer::MeshBufferFactory::CreateCubeBuffer()
);
auto sphereBufferPtr = std::make_shared<Renderer::MeshBuffer>(
    Renderer::MeshBufferFactory::CreateSphereBuffer()
);

// 3. 创建渲染器
Renderer::InstancedRenderer cubeRenderer, sphereRenderer;
cubeRenderer.SetMesh(cubeBufferPtr);
cubeRenderer.SetInstances(cubeInstances);
cubeRenderer.Initialize();

sphereRenderer.SetMesh(sphereBufferPtr);
sphereRenderer.SetInstances(sphereInstances);
sphereRenderer.Initialize();

// 3. 渲染循环中更新动画
float time = glfwGetTime();
auto& cubeMatrices = cubeInstances->GetModelMatrices();
auto& sphereMatrices = sphereInstances->GetModelMatrices();

// 更新中央球体（自转）
float centerRotX = std::sin(time * 0.3f) * 360.0f;
float centerRotY = time * 20.0f;
float centerRotZ = std::cos(time * 0.2f) * 360.0f;
// ... 计算并更新 sphereMatrices[0] 和 cubeMatrices[0-499]

// 更新8个彩色球体（自转 + 公转）
for (int i = 0; i < 8; ++i) {
    // 公转位置
    float orbitAngle = glm::radians(i * 45.0f + time * 50.0f);
    glm::vec3 lightCenter(10.0f * std::cos(orbitAngle), 5.0f, 10.0f * std::sin(orbitAngle));

    // 自转角度
    float selfRotX = std::sin(time * (0.5f + i * 0.2f) + i) * 180.0f;
    float selfRotY = time * (50.0f + i * 15.0f);
    float selfRotZ = std::cos(time * (0.5f + i * 0.2f) * 0.7f + i * 2.0f) * 180.0f;

    // ... 计算并更新 sphereMatrices[i+1] 和 cubeMatrices[500+i*100 : 600+i*100]
}

// 4. 更新GPU数据并渲染
cubeRenderer.UpdateInstanceData();  // 更新立方体到GPU
sphereRenderer.UpdateInstanceData(); // 更新球体到GPU

cubeRenderer.Render();
sphereRenderer.Render();
```

#### 着色器要求

**顶点着色器**（`assets/shader/instanced.vert`）：

```glsl
layout (location = 3) in mat4 aInstanceMatrix;  // 实例变换矩阵
layout (location = 7) in vec3 aInstanceColor;   // 实例颜色

void main() {
    mat4 model = aInstanceMatrix;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    InstanceColor = aInstanceColor;  // 传递到片段着色器
}
```

**片段着色器**（`assets/shader/instanced.frag`）：

```glsl
uniform bool useTexture;        // 是否使用纹理
uniform bool useInstanceColor;  // 是否使用实例颜色
uniform vec3 objectColor;       // 材质颜色
uniform sampler2D textureSampler;

in vec3 InstanceColor;

void main() {
    vec3 baseColor;
    if (useTexture) {
        baseColor = texture(textureSampler, TexCoord).rgb;
    } else if (useInstanceColor) {
        baseColor = InstanceColor;
    } else {
        baseColor = objectColor;
    }

    // 应用光照...
    FragColor = vec4(result, 1.0);
}
```

#### 性能优势

- **传统渲染**：12辆车 × 38个材质 = 456次绘制调用
- **实例化渲染**：38个材质 = 38次绘制调用
- **性能提升**：约12倍（取决于场景复杂度）

---

## Environment 模块接口

### Skybox 类

天空盒渲染器，负责渲染立方体环境贴图。

```cpp
namespace Renderer {

class Skybox {
public:
    Skybox();
    ~Skybox();

    // 禁止拷贝，允许移动
    Skybox(const Skybox&) = delete;
    Skybox& operator=(const Skybox&) = delete;
    Skybox(Skybox&& other) noexcept;
    Skybox& operator=(Skybox&& other) noexcept;

    // 初始化
    bool Initialize();

    // 加载天空盒纹理
    bool Load(const std::string& right, const std::string& left,
              const std::string& top, const std::string& bottom,
              const std::string& back, const std::string& front);
    bool LoadFromConfig(const SkyboxConfig& config);

    // 着色器
    bool LoadShaders(const std::string& vertexPath, const std::string& fragmentPath);

    // 渲染
    void Render(const glm::mat4& projection, const glm::mat4& view);

    // 纹理操作
    void BindTexture(unsigned int textureUnit = 0) const;
    unsigned int GetTextureID() const;

    // 状态查询
    bool IsLoaded() const;

    // 旋转控制
    void SetRotation(float rotationDegrees);
    float GetRotation() const;
};

} // namespace Renderer
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Initialize()` | 无 | bool | 初始化天空盒，创建立方体网格 |
| `Load()` | 6个面的文件路径 | bool | 从6个纹理文件加载天空盒 |
| `LoadFromConfig()` | SkyboxConfig | bool | 从配置加载天空盒 |
| `LoadShaders()` | vertexPath, fragmentPath | bool | 加载天空盒着色器 |
| `Render()` | projection, view | void | 渲染天空盒 |
| `BindTexture()` | textureUnit | void | 绑定天空盒纹理到指定纹理单元 |
| `GetTextureID()` | 无 | unsigned int | 获取天空盒纹理ID |
| `IsLoaded()` | 无 | bool | 检查天空盒是否已加载 |
| `SetRotation()` | rotationDegrees | void | 设置旋转角度 |
| `GetRotation()` | 无 | float | 获取旋转角度 |

#### 使用示例

```cpp
// 1. 创建并初始化天空盒
Renderer::Skybox skybox;
skybox.Initialize();
skybox.LoadShaders("assets/shader/skybox.vert", "assets/shader/skybox.frag");

// 2. 使用SkyboxLoader加载配置
auto config = Renderer::SkyboxLoader::CreateCustomConfig(
    "assets/textures/skybox",
    {"corona_rt.png", "corona_lf.png", "corona_up.png",
     "corona_dn.png", "corona_bk.png", "corona_ft.png"},
    Renderer::CubemapConvention::OPENGL
);
skybox.LoadFromConfig(config);

// 3. 在渲染循环中（先渲染天空盒）
glm::mat4 view = camera.GetViewMatrix();
glm::mat4 projection = camera.GetProjectionMatrix(aspect);

// 渲染天空盒（背景层）
glDepthFunc(GL_LEQUAL);  // 深度测试改为<=
glDepthMask(GL_FALSE);   // 禁止深度写入
skybox.Render(projection, view);
glDepthMask(GL_TRUE);    // 恢复深度写入
glDepthFunc(GL_LESS);    // 恢复默认深度测试

// 渲染场景物体
// ...

// 4. 设置环境光照
ambientLighting.LoadFromSkybox(skybox.GetTextureID(), 0.3f);
ambientLighting.ApplyToShader(shader);
```

---

### SkyboxLoader 类

天空盒加载工具类，支持多种cubemap约定和灵活的命名格式。

```cpp
namespace Renderer {

// Cubemap约定枚举
enum class CubemapConvention {
    OPENGL,        // OpenGL标准: right, left, top, bottom, front(+Z), back(-Z)
    DIRECTX,       // DirectX: left, right, top, bottom, front, back
    MAYA,          // Maya/Corona: right, left, top, bottom, back(+Z), front(-Z)
    BLENDER,       // Blender: right, left, top, bottom, front, back
    CUSTOM         // 自定义映射
};

// 天空盒配置结构
struct SkyboxConfig {
    std::string directory;                           // 天空盒纹理目录
    std::vector<std::string> faceFilenames;         // 6个面的文件名（按OpenGL顺序）
    CubemapConvention convention;                   // 使用的约定
    bool flipVertically;                            // 是否垂直翻转纹理
    bool generateMipmaps;                           // 是否生成mipmaps
};

// Cubemap面命名方案
struct FaceNamingScheme {
    std::string right;   // +X 面
    std::string left;    // -X 面
    std::string top;     // +Y 面
    std::string bottom;  // -Y 面
    std::string back;    // +Z 面
    std::string front;   // -Z 面

    FaceNamingScheme(const std::string& r = "right",
                     const std::string& l = "left",
                     const std::string& t = "top",
                     const std::string& b = "bottom",
                     const std::string& bk = "back",
                     const std::string& f = "front");

    std::vector<std::string> ToArray() const;
};

class SkyboxLoader {
public:
    // 从标准命名约定创建配置
    static SkyboxConfig CreateConfig(
        const std::string& directory,
        CubemapConvention convention,
        const std::string& basename = "",
        const std::string& extension = ".png"
    );

    // 从自定义文件名创建配置
    static SkyboxConfig CreateCustomConfig(
        const std::string& directory,
        const std::vector<std::string>& filenames,
        CubemapConvention convention = CubemapConvention::OPENGL
    );

    // 从文件模式创建配置（支持通配符）
    static SkyboxConfig CreateFromPattern(
        const std::string& directory,
        const std::string& pattern,
        CubemapConvention convention,
        const std::string& extension = ".png"
    );

    // 从自定义面命名方案创建配置（完全自定义）
    static SkyboxConfig CreateFromCustomScheme(
        const std::string& directory,
        const std::string& pattern,
        const FaceNamingScheme& namingScheme,
        CubemapConvention convention,
        const std::string& extension = ".png"
    );

    // 获取约定对应的OpenGL标准顺序的文件名
    static std::vector<std::string> ConvertToOpenGL(
        CubemapConvention convention,
        const std::vector<std::string>& inputNames
    );

    // 获取常用命名方案的预设
    static FaceNamingScheme GetOpenGLScheme();
    static FaceNamingScheme GetMayaScheme();
    static FaceNamingScheme GetDirectXScheme();
    static FaceNamingScheme GetHDRLabScheme();
};

} // namespace Renderer
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `CreateConfig()` | directory, convention, basename, extension | SkyboxConfig | 从标准命名约定创建配置 |
| `CreateCustomConfig()` | directory, filenames, convention | SkyboxConfig | 从自定义文件名创建配置 |
| `CreateFromPattern()` | directory, pattern, convention, extension | SkyboxConfig | 从文件模式创建配置 |
| `CreateFromCustomScheme()` | directory, pattern, namingScheme, convention, extension | SkyboxConfig | 从自定义面命名方案创建配置 |
| `ConvertToOpenGL()` | convention, inputNames | vector<string> | 转换到OpenGL标准顺序 |
| `GetOpenGLScheme()` | 无 | FaceNamingScheme | 获取OpenGL标准命名方案 |
| `GetMayaScheme()` | 无 | FaceNamingScheme | 获取Maya命名方案 |
| `GetDirectXScheme()` | 无 | FaceNamingScheme | 获取DirectX命名方案 |
| `GetHDRLabScheme()` | 无 | FaceNamingScheme | 获取HDR Lab命名方案 |

#### 使用示例

```cpp
// 方式1：使用CreateCustomConfig（最直接）
auto config1 = Renderer::SkyboxLoader::CreateCustomConfig(
    "assets/textures/skybox",
    {"corona_rt.png", "corona_lf.png", "corona_up.png",
     "corona_dn.png", "corona_bk.png", "corona_ft.png"},
    Renderer::CubemapConvention::OPENGL
);

// 方式2：使用CreateFromPattern（基于预设约定）
auto config2 = Renderer::SkyboxLoader::CreateFromPattern(
    "assets/textures/skybox",
    "corona_{face}",
    Renderer::CubemapConvention::MAYA,
    ".png"
);

// 方式3：使用CreateFromCustomScheme（完全自定义）
Renderer::FaceNamingScheme customScheme(
    "rt", "lf", "up", "dn", "bk", "ft"
);
auto config3 = Renderer::SkyboxLoader::CreateFromCustomScheme(
    "assets/textures/skybox",
    "corona_{face}",
    customScheme,
    Renderer::CubemapConvention::OPENGL,
    ".png"
);

// 方式4：使用预设命名方案
auto config4 = Renderer::SkyboxLoader::CreateFromCustomScheme(
    "assets/textures/skybox",
    "{face}",
    Renderer::SkyboxLoader::GetHDRLabScheme(),  // "px", "nx", "py", "ny", "pz", "nz"
    Renderer::CubemapConvention::OPENGL,
    ".hdr"
);
```

---

### AmbientLighting 类

轻量级环境光照系统（非PBR），支持三种环境光模式。

```cpp
namespace Renderer {

class AmbientLighting {
public:
    // 环境光照模式
    enum class Mode {
        SOLID_COLOR,      // 固定颜色环境光（传统Phong）
        SKYBOX_SAMPLE,    // 从天空盒采样环境光（IBL）
        HEMISPHERE        // 半球渐变环境光（天空/地面颜色插值）
    };

    AmbientLighting();
    ~AmbientLighting() = default;

    // 初始化
    void Initialize();

    // 从天空盒加载
    bool LoadFromSkybox(unsigned int skyboxTextureID, float intensity = 0.3f);

    // 设置模式
    void SetMode(Mode mode);
    Mode GetMode() const;

    // 设置颜色
    void SetSkyColor(const glm::vec3& color);
    void SetGroundColor(const glm::vec3& color);
    void SetIntensity(float intensity);
    float GetIntensity() const;

    // 应用到着色器
    void ApplyToShader(Shader& shader) const;

private:
    void BindTexture(unsigned int textureUnit) const;
};

} // namespace Renderer
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Initialize()` | 无 | void | 初始化环境光照系统 |
| `LoadFromSkybox()` | skyboxTextureID, intensity | bool | 从天空盒加载环境光照 |
| `SetMode()` | mode | void | 设置环境光照模式 |
| `GetMode()` | 无 | Mode | 获取当前模式 |
| `SetSkyColor()` | color | void | 设置天空颜色（用于HEMISPHERE模式） |
| `SetGroundColor()` | color | void | 设置地面颜色（用于HEMISPHERE模式） |
| `SetIntensity()` | intensity | void | 设置环境光强度 |
| `GetIntensity()` | 无 | float | 获取环境光强度 |
| `ApplyToShader()` | shader | void | 将环境光照设置应用到着色器 |

#### 使用示例

```cpp
// 1. 创建并初始化环境光照
Renderer::AmbientLighting ambientLighting;
ambientLighting.Initialize();

// 2. 从天空盒加载
ambientLighting.LoadFromSkybox(skybox.GetTextureID(), 0.3f);

// 3. 在渲染循环中应用
ambientLighting.SetMode(Renderer::AmbientLighting::Mode::SKYBOX_SAMPLE);
ambientLighting.ApplyToShader(shader);

// 4. 运行时切换模式（键盘控制）
if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    ambientLighting.SetMode(Renderer::AmbientLighting::Mode::SOLID_COLOR);
if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    ambientLighting.SetMode(Renderer::AmbientLighting::Mode::SKYBOX_SAMPLE);
if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    ambientLighting.SetMode(Renderer::AmbientLighting::Mode::HEMISPHERE);

// 5. 调整强度
if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
    ambientLighting.SetIntensity(ambientLighting.GetIntensity() + 0.05f);
if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
    ambientLighting.SetIntensity(ambientLighting.GetIntensity() - 0.05f);
```

#### 着色器集成

在片段着色器中使用环境光照：

```glsl
// ambient_ibl.frag
uniform int ambientMode;        // 0=SOLID_COLOR, 1=SKYBOX_SAMPLE, 2=HEMISPHERE
uniform float ambientIntensity;
uniform vec3 skyColor;           // 天空颜色（HEMISPHERE模式）
uniform vec3 groundColor;        // 地面颜色（HEMISPHERE模式）
uniform samplerCube skybox;      // 天空盒纹理（SKYBOX_SAMPLE模式）

vec3 CalcAmbientLight(vec3 normal) {
    vec3 ambient = vec3(0.0);

    if (ambientMode == 0) {
        // 固定颜色环境光
        ambient = vec3(ambientIntensity);
    }
    else if (ambientMode == 1) {
        // 从天空盒采样
        ambient = texture(skybox, normal).rgb * ambientIntensity;
    }
    else if (ambientMode == 2) {
        // 半球渐变
        float hemi = normal.y * 0.5 + 0.5;  // 将[-1,1]映射到[0,1]
        ambient = mix(groundColor, skyColor, hemi) * ambientIntensity;
    }

    return ambient;
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 ambient = CalcAmbientLight(norm);
    vec3 directLighting = /* 计算Phong光照 */;
    vec3 result = (ambient + directLighting) * baseColor;
    FragColor = vec4(result, 1.0);
}
```

---

## 几何体接口

### 统一几何体创建接口

```cpp
// 通过工厂模式创建几何体
auto cube = MeshFactory::Create("cube");
auto sphere = MeshFactory::Create("sphere");
auto objModel = MeshFactory::Create("obj:path/to/model.obj");

// 注册自定义几何体类型
MeshFactory::Register("myShape", []() {
    return std::make_unique<MyCustomShape>();
});
```

### 几何体操作接口

所有几何体都实现以下统一接口：

- `Create()`: 初始化几何体数据和OpenGL缓冲区
- `Draw()`: 执行几何体的渲染操作

---

## 📚 相关文档

- [TinyOBJLoader API 参考](TINYOBJ_API_REFERENCE.md) - 第三方OBJ加载库接口
- [架构设计文档](../ARCHITECTURE.md) - 项目整体架构分析
- [优化指南](../OPTIMIZATION_GUIDE.md) - 性能优化和扩展建议

---

*本文档描述了OpenGL学习项目的所有核心接口定义和使用方法* 🔧
