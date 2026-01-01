# OpenGL学习项目接口文档

## 📋 目录

- [Core 模块接口](#core-模块接口)
  - [Window 类](#window-类)
  - [Camera 类](#camera-类)
  - [MouseController 类](#mousecontroller-类)
  - [KeyboardController 类](#keyboardcontroller-类)
  - [Logger 类](#logger-类)
- [Lighting 模块接口](#lighting-模块接口)
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

### PointLight 类

点光源，从一个点向所有方向发光（如灯泡）。

```cpp
namespace Renderer {
namespace Lighting {

class PointLight : public Light {
public:
    struct Attenuation {
        float constant;
        float linear;
        float quadratic;

        static Attenuation Range7();
        static Attenuation Range13();
        static Attenuation Range20();
        static Attenuation Range32();
        static Attenuation Range50();
        static Attenuation Range100();
        static Attenuation Range200();
    };

    PointLight(
        const glm::vec3 &position,
        const glm::vec3 &color = glm::vec3(1.0f),
        float intensity = 1.0f,
        float ambient = 0.0f,
        float diffuse = 0.8f,
        float specular = 1.0f,
        const Attenuation &attenuation = Attenuation());

    // 位置和衰减
    const glm::vec3 &GetPosition() const;
    void SetPosition(const glm::vec3 &position);

    const Attenuation &GetAttenuation() const;
    void SetAttenuation(const Attenuation &attenuation);

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
| `PointLight()` | position, color, intensity, ... | - | 构造点光源，设置位置和衰减参数 |
| `GetPosition()` | 无 | const glm::vec3& | 获取光源位置 |
| `SetPosition()` | position | void | 设置光源位置 |
| `GetAttenuation()` | 无 | const Attenuation& | 获取衰减参数 |
| `SetAttenuation()` | attenuation | void | 设置衰减参数 |
| `Range7()` ~ `Range200()` | 无 | Attenuation | 预设衰减范围（7米/13米/20米/32米/50米/100米/200米） |

---

### SpotLight 类

聚光灯，从一个点向特定方向锥形发光。

```cpp
namespace Renderer {
namespace Lighting {

class SpotLight : public Light {
public:
    SpotLight(
        const glm::vec3 &position,
        const glm::vec3 &direction,
        const glm::vec3 &color = glm::vec3(1.0f),
        float intensity = 1.0f,
        float ambient = 0.0f,
        float diffuse = 0.8f,
        float specular = 1.0f,
        const PointLight::Attenuation &attenuation = PointLight::Attenuation(),
        float cutOff = glm::cos(glm::radians(12.5f)),
        float outerCutOff = glm::cos(glm::radians(17.5f)));

    // 位置、方向和衰减
    const glm::vec3 &GetPosition() const;
    void SetPosition(const glm::vec3 &position);

    const glm::vec3 &GetDirection() const;
    void SetDirection(const glm::vec3 &direction);

    const PointLight::Attenuation &GetAttenuation() const;
    void SetAttenuation(const PointLight::Attenuation &attenuation);

    // 锥形角度
    float GetCutOff() const;
    void SetCutOff(float cutOff);

    float GetOuterCutOff() const;
    void SetOuterCutOff(float outerCutOff);

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
| `SpotLight()` | position, direction, color, ... | - | 构造聚光灯，设置位置、方向和锥角 |
| `GetPosition()` | 无 | const glm::vec3& | 获取光源位置 |
| `SetPosition()` | position | void | 设置光源位置 |
| `GetDirection()` | 无 | const glm::vec3& | 获取光照方向 |
| `SetDirection()` | direction | void | 设置光照方向 |
| `GetCutOff()` | 无 | float | 获取内锥角（余弦值） |
| `SetCutOff()` | cutOff | void | 设置内锥角（余弦值） |
| `GetOuterCutOff()` | 无 | float | 获取外锥角（余弦值） |
| `SetOuterCutOff()` | outerCutOff | void | 设置外锥角（余弦值） |

---

### LightManager 类

光照管理器，统一管理所有光源（单例模式）。

```cpp
namespace Renderer {
namespace Lighting {

class LightManager {
public:
    // 获取单例实例
    static LightManager &GetInstance();

    // 添加光源
    void AddDirectionalLight(std::shared_ptr<DirectionalLight> light);
    void AddPointLight(std::shared_ptr<PointLight> light);
    void AddSpotLight(std::shared_ptr<SpotLight> light);

    // 获取光源列表
    const std::vector<std::shared_ptr<DirectionalLight>>& GetDirectionalLights() const;
    const std::vector<std::shared_ptr<PointLight>>& GetPointLights() const;
    const std::vector<std::shared_ptr<SpotLight>>& GetSpotLights() const;

    // 应用所有光源到着色器
    void ApplyToShader(Shader &shader) const;

    // 清除所有光源
    void Clear();
};

} // namespace Lighting
} // namespace Renderer
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `GetInstance()` | 无 | LightManager& | 获取单例实例 |
| `AddDirectionalLight()` | shared_ptr<DirectionalLight> | void | 添加平行光 |
| `AddPointLight()` | shared_ptr<PointLight> | void | 添加点光源 |
| `AddSpotLight()` | shared_ptr<SpotLight> | void | 添加聚光灯 |
| `GetDirectionalLights()` | 无 | const vector& | 获取所有平行光 |
| `GetPointLights()` | 无 | const vector& | 获取所有点光源 |
| `GetSpotLights()` | 无 | const vector& | 获取所有聚光灯 |
| `ApplyToShader()` | shader | void | 将所有光源应用到着色器 |
| `Clear()` | 无 | void | 清除所有光源 |

#### 使用示例

```cpp
// 1. 创建光照管理器
auto& lightManager = Renderer::Lighting::LightManager::GetInstance();

// 2. 添加平行光（太阳光）
auto dirLight = std::make_shared<Renderer::Lighting::DirectionalLight>(
    glm::vec3(0.0f, -1.0f, -0.3f),  // 方向
    glm::vec3(1.0f, 0.95f, 0.9f),     // 颜色（暖白光）
    0.5f                             // 强度
);
lightManager.AddDirectionalLight(dirLight);

// 3. 添加点光源（彩色灯球）
auto pointLight1 = std::make_shared<Renderer::Lighting::PointLight>(
    glm::vec3(0.0f, 8.0f, 0.0f),     // 位置
    glm::vec3(1.0f, 0.1f, 0.1f),     // 红色
    8.0f,                            // 强度
    0.0f, 0.8f, 1.0f,                // ambient, diffuse, specular
    Renderer::Lighting::PointLight::Attenuation::Range32()  // 32米衰减
);
lightManager.AddPointLight(pointLight1);

// 4. 添加聚光灯（手电筒）
auto flashlight = std::make_shared<Renderer::Lighting::SpotLight>(
    camera.GetPosition(),            // 位置（跟随摄像机）
    camera.GetFront(),               // 方向（摄像机朝向）
    glm::vec3(1.0f, 1.0f, 1.0f),    // 白色
    1.0f,                            // 强度
    0.0f, 0.8f, 1.0f,                // ambient, diffuse, specular
    Renderer::Lighting::PointLight::Attenuation::Range50(),  // 50米衰减
    glm::cos(glm::radians(12.5f)),   // 内锥角12.5度
    glm::cos(glm::radians(17.5f))    // 外锥角17.5度
);
lightManager.AddSpotLight(flashlight);

// 5. 在渲染循环中应用光源
shader.Use();
lightManager.ApplyToShader(shader);
```

---

## Renderer 模块接口

### IMesh 抽象接口

网格渲染的抽象基类，定义了网格的基本操作。

```cpp
namespace Renderer {
class IMesh {
public:
    virtual void Create() = 0;                      // 创建网格资源
    virtual void Draw() const = 0;                  // 绘制网格
    virtual ~IMesh() = default;                     // 虚析构函数

    // 扩展接口（可选实现）
    virtual unsigned int GetVAO() const { return 0; }
    virtual size_t GetVertexCount() const { return 0; }
    virtual size_t GetIndexCount() const { return 0; }
    virtual bool HasIndices() const { return false; }
    virtual bool HasTexture() const { return false; }
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Create()` | 无 | void | 初始化网格的顶点缓冲对象和数组对象 |
| `Draw()` | 无 | void | 执行网格的渲染操作 |
| `GetVAO()` | 无 | unsigned int | 返回VAO ID（可选实现） |
| `GetVertexCount()` | 无 | size_t | 返回顶点数量（可选实现） |
| `GetIndexCount()` | 无 | size_t | 返回索引数量（可选实现） |
| `HasIndices()` | 无 | bool | 是否有索引数据（可选实现） |
| `HasTexture()` | 无 | bool | 是否有纹理（可选实现） |

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

IRenderer接口与IMesh接口分离：

- **IMesh**: 表示可渲染的几何体数据
- **IRenderer**: 表示渲染逻辑的执行者
- 例如：InstancedRenderer继承IRenderer，Cube/Sphere/Torus继承IMesh

---

### MeshFactory 工厂类

网格对象的工厂类，支持运行时注册和创建不同类型的网格。

```cpp
namespace Renderer {
class MeshFactory {
public:
    // 类型注册
    static void Register(const std::string& type,
                        std::function<std::unique_ptr<IMesh>()> creator);

    // 对象创建
    static std::unique_ptr<IMesh> Create(const std::string& type);
};
}
```

#### 接口说明

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

### Cube 类

立方体网格实现类。

```cpp
namespace Renderer {
class Cube : public IMesh {
public:
    Cube();
    ~Cube() override;

    void Create() override;
    void Draw() const override;
};
}
```

#### 接口说明

继承自 `IMesh`，实现立方体的创建和绘制。

---

### Sphere 类

球体网格实现类，支持自定义半径和分段数。

```cpp
namespace Renderer {
class Sphere : public IMesh {
public:
    // 构造函数
    explicit Sphere(float radius = 1.0f, int stacks = 20, int slices = 20);

    // IMesh 接口实现
    void Create() override;
    void Draw() const override;

    // 变换配置
    void SetPosition(const glm::vec3& pos);
    void SetColor(const glm::vec3& color);
    void SetScale(float scale);
    void SetRadius(float radius);
    void SetSegments(int stacks, int slices);

    // 获取状态
    const glm::vec3& GetColor() const;
    float GetRadius() const;
    glm::mat4 GetModelMatrix() const;

    // 静态方法：获取球体的标准顶点数据（用于实例化渲染）
    static std::vector<float> GetVertexData();
    static std::vector<unsigned int> GetIndexData();
    static void GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes);

    // IMesh 接口扩展
    unsigned int GetVAO() const override;
    size_t GetVertexCount() const override;
    size_t GetIndexCount() const override;
    bool HasIndices() const override;
    bool HasTexture() const override;
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Sphere()` | float radius, int stacks, int slices | - | 构造函数，radius=球半径，stacks=纬度线数，slices=经度线数 |
| `Create()` | 无 | void | 创建球体网格资源 |
| `Draw()` | 无 | void | 绘制球体 |
| `SetPosition()` | glm::vec3 pos | void | 设置位置 |
| `SetColor()` | glm::vec3 color | void | 设置颜色 |
| `SetScale()` | float scale | void | 设置缩放 |
| `SetRadius()` | float radius | void | 设置半径 |
| `SetSegments()` | int stacks, int slices | void | 设置分段数 |
| `GetColor()` | 无 | glm::vec3 | 获取颜色 |
| `GetRadius()` | 无 | float | 获取半径 |
| `GetModelMatrix()` | 无 | glm::mat4 | 获取模型矩阵 |
| `GetVertexData()` | 无 | vector<float> | 静态方法：获取顶点数据（用于工厂模式） |
| `GetIndexData()` | 无 | vector<uint> | 静态方法：获取索引数据 |
| `GetVertexLayout()` | vector<size_t>& offsets, vector<int>& sizes | void | 静态方法：获取顶点布局 |

#### 使用示例

```cpp
// 示例1：创建并渲染单个球体
Renderer::Sphere sphere(1.0f, 32, 32);  // 半径1.0，高精度分段
sphere.SetPosition(glm::vec3(0.0f, 2.0f, 0.0f));
sphere.SetColor(glm::vec3(1.0f, 0.5f, 0.0f));  // 橙色
sphere.Create();

// 在渲染循环中
shader.Use();
shader.SetMat4("model", sphere.GetModelMatrix());
shader.SetVec3("color", sphere.GetColor());
sphere.Draw();

// 示例2：使用静态方法获取顶点数据（用于实例化渲染）
auto vertices = Renderer::Sphere::GetVertexData();
auto indices = Renderer::Sphere::GetIndexData();
std::vector<size_t> offsets;
std::vector<int> sizes;
Renderer::Sphere::GetVertexLayout(offsets, sizes);
```

---

### Torus 类

圆环体（甜甜圈）网格实现类，支持自定义主半径、管半径和分段数。

```cpp
namespace Renderer {
class Torus : public IMesh {
public:
    // 构造函数
    explicit Torus(float majorRadius = 1.0f, float minorRadius = 0.3f,
                   int majorSegments = 32, int minorSegments = 24);

    // IMesh 接口实现
    void Create() override;
    void Draw() const override;

    // 变换配置
    void SetPosition(const glm::vec3& pos);
    void SetScale(float scale);
    void SetRotation(const glm::vec3& rotation);
    void SetColor(const glm::vec3& color);

    // 参数配置
    void SetMajorRadius(float radius);
    void SetMinorRadius(float radius);
    void SetMajorSegments(int segments);
    void SetMinorSegments(int segments);

    // 获取状态
    const glm::vec3& GetColor() const;
    float GetMajorRadius() const;
    float GetMinorRadius() const;
    glm::mat4 GetModelMatrix() const;

    // 静态方法：获取圆环体的标准顶点数据（用于工厂模式）
    static std::vector<float> GetVertexData();
    static std::vector<unsigned int> GetIndexData();
    static void GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes);

    // IMesh 接口扩展
    unsigned int GetVAO() const override;
    size_t GetVertexCount() const override;
    size_t GetIndexCount() const override;
    bool HasIndices() const override;
    bool HasTexture() const override;
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Torus()` | float majorRadius, float minorRadius, int majorSegments, int minorSegments | - | 构造函数，majorRadius=主半径（环半径），minorRadius=管半径 |
| `Create()` | 无 | void | 创建圆环体网格资源 |
| `Draw()` | 无 | void | 绘制圆环体 |
| `SetPosition()` | glm::vec3 pos | void | 设置位置 |
| `SetRotation()` | glm::vec3 rotation | void | 设置旋转（欧拉角） |
| `SetColor()` | glm::vec3 color | void | 设置颜色 |
| `SetMajorRadius()` | float radius | void | 设置主半径（从中心到管中心的距离） |
| `SetMinorRadius()` | float radius | void | 设置管半径（管的粗细） |
| `GetMajorRadius()` | 无 | float | 获取主半径 |
| `GetMinorRadius()` | 无 | float | 获取管半径 |
| `GetModelMatrix()` | 无 | glm::mat4 | 获取模型矩阵 |

#### 使用示例

```cpp
// 创建一个甜甜圈形状
Renderer::Torus torus(1.0f, 0.3f, 48, 32);  // 主半径1.0，管半径0.3，高精度
torus.SetPosition(glm::vec3(3.0f, 0.5f, 0.0f));
torus.SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));  // 绕X轴旋转90度
torus.SetColor(glm::vec3(0.8f, 0.2f, 0.8f));  // 紫色
torus.Create();

// 在渲染循环中
shader.Use();
shader.SetMat4("model", torus.GetModelMatrix());
shader.SetVec3("color", torus.GetColor());
torus.Draw();
```

---

### Plane 类

平面网格实现类，支持自定义宽度和高度，可配置分段数。

```cpp
namespace Renderer {
class Plane : public IMesh {
public:
    // 构造函数
    explicit Plane(float width = 1.0f, float height = 1.0f,
                   int widthSegments = 1, int heightSegments = 1);

    // IMesh 接口实现
    void Create() override;
    void Draw() const override;

    // 变换配置
    void SetPosition(const glm::vec3& pos);
    void SetScale(float scale);
    void SetRotation(const glm::vec3& rotation);
    void SetColor(const glm::vec3& color);

    // 参数配置
    void SetWidth(float width);
    void SetHeight(float height);
    void SetWidthSegments(int segments);
    void SetHeightSegments(int segments);

    // 获取状态
    const glm::vec3& GetColor() const;
    float GetWidth() const;
    float GetHeight() const;
    glm::mat4 GetModelMatrix() const;

    // 静态方法：获取平面的标准顶点数据（用于工厂模式）
    static std::vector<float> GetVertexData();
    static std::vector<unsigned int> GetIndexData();
    static void GetVertexLayout(std::vector<size_t>& offsets, std::vector<int>& sizes);

    // IMesh 接口扩展
    unsigned int GetVAO() const override;
    size_t GetVertexCount() const override;
    size_t GetIndexCount() const override;
    bool HasIndices() const override;
    bool HasTexture() const override;
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Plane()` | float width, float height, int widthSegments, int heightSegments | - | 构造函数，width=宽度，height=高度，segments=分段数 |
| `Create()` | 无 | void | 创建平面网格资源 |
| `Draw()` | 无 | void | 绘制平面 |
| `SetPosition()` | glm::vec3 pos | void | 设置位置 |
| `SetRotation()` | glm::vec3 rotation | void | 设置旋转（欧拉角） |
| `SetWidth()` | float width | void | 设置宽度 |
| `SetHeight()` | float height | void | 设置高度 |
| `SetWidthSegments()` | int segments | void | 设置宽度方向分段数（用于细分） |
| `SetHeightSegments()` | int segments | void | 设置高度方向分段数 |
| `GetWidth()` | 无 | float | 获取宽度 |
| `GetHeight()` | 无 | float | 获取高度 |
| `GetModelMatrix()` | 无 | glm::mat4 | 获取模型矩阵 |

#### 使用示例

```cpp
// 示例1：创建地面平面
Renderer::Plane ground(20.0f, 20.0f, 1, 1);  // 20x20的地面
ground.SetPosition(glm::vec3(0.0f, -1.0f, 0.0f));
ground.SetRotation(glm::vec3(-90.0f, 0.0f, 0.0f));  // 放平
ground.SetColor(glm::vec3(0.5f, 0.5f, 0.5f));  // 灰色
ground.Create();

// 示例2：创建细分平面（用于地形等）
Renderer::Plane terrain(50.0f, 50.0f, 100, 100);  // 100x100细分
terrain.SetPosition(glm::vec3(-25.0f, -2.0f, -25.0f));
terrain.Create();
```

---

### InstanceData 类

实例数据容器，存储多个实例的变换和颜色信息。

```cpp
namespace Renderer {

class InstanceData {
public:
    InstanceData() = default;

    // 添加单个实例
    void Add(const glm::vec3& position, const glm::vec3& rotation,
             const glm::vec3& scale, const glm::vec3& color);

    // 批量添加实例
    void AddBatch(const std::vector<glm::mat4>& matrices, const std::vector<glm::vec3>& colors);

    // 清除所有实例
    void Clear();

    // 获取实例数量
    size_t GetCount() const;

    // 数据访问
    const std::vector<glm::mat4>& GetModelMatrices() const;
    const std::vector<glm::vec3>& GetColors() const;
    std::vector<glm::mat4>& GetModelMatrices();
    std::vector<glm::vec3>& GetColors();

    // 判断是否为空
    bool IsEmpty() const;
};

}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Add()` | position, rotation, scale, color | void | 添加单个实例 |
| `AddBatch()` | matrices, colors | void | 批量添加实例 |
| `Clear()` | 无 | void | 清除所有实例 |
| `GetCount()` | 无 | size_t | 返回实例数量 |
| `GetModelMatrices()` | 无 | const vector<glm::mat4>& | 获取模型矩阵数组（const版本） |
| `GetModelMatrices()` | 无 | vector<glm::mat4>& | 获取模型矩阵数组（非const版本） |
| `GetColors()` | 无 | const vector<glm::vec3>& | 获取颜色数组（const版本） |
| `GetColors()` | 无 | vector<glm::vec3>& | 获取颜色数组（非const版本） |
| `IsEmpty()` | 无 | bool | 判断是否为空 |

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

### OBJModel 类

OBJ模型加载和渲染类。

```cpp
namespace Renderer {
class OBJModel : public IMesh {
public:
    OBJModel(const std::string& filepath);
    ~OBJModel() override;

    void Create() override;
    void Draw() const override;

    // 模型信息查询
    bool IsLoaded() const;
    const std::string& GetFilePath() const;
    size_t GetVertexCount() const;
    size_t GetTriangleCount() const;
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `IsLoaded()` | 无 | bool | 检查模型是否成功加载 |
| `GetVertexCount()` | 无 | size_t | 返回模型顶点数量 |
| `GetTriangleCount()` | 无 | size_t | 返回模型三角形数量 |

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
