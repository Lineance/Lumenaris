# OpenGL学习项目接口文档

## 📋 目录

- [Core 模块接口](#core-模块接口)
  - [Window 类](#window-类)
  - [MouseController 类](#mousecontroller-类)
  - [KeyboardController 类](#keyboardcontroller-类)
  - [Logger 类](#logger-类)
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
  - [SimpleMesh 类](#simplemesh-类)
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
- 例如：InstancedRenderer继承IRenderer，SimpleMesh继承IMesh

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

球体网格实现类。

```cpp
namespace Renderer {
class Sphere : public IMesh {
public:
    Sphere(int stacks = 32, int slices = 32, float radius = 1.0f);
    ~Sphere() override;

    void Create() override;
    void Draw() const override;

    // 参数设置
    void SetRadius(float radius);
    void SetStacks(int stacks);
    void SetSlices(int slices);
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `SetRadius()` | float radius | void | 设置球体半径 |
| `SetStacks()` | int stacks | void | 设置纬度分段数 |
| `SetSlices()` | int slices | void | 设置经度分段数 |

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

### SimpleMesh 类

简单网格类 - 纯粹的数据容器，用于实例化渲染。SimpleMesh 存储网格几何数据（顶点、索引、纹理），支持深拷贝语义，使用shared_ptr管理纹理。

```cpp
namespace Renderer {

class SimpleMesh : public IMesh {
public:
    SimpleMesh() = default;
    ~SimpleMesh();

    // 拷贝语义（深拷贝）
    SimpleMesh(const SimpleMesh& other);
    SimpleMesh& operator=(const SimpleMesh& other);

    // 移动语义（显式实现）
    SimpleMesh(SimpleMesh&& other) noexcept;
    SimpleMesh& operator=(SimpleMesh&& other) noexcept;

    // IMesh接口实现
    void Create() override;
    void Draw() const override;

    // IMesh接口扩展
    unsigned int GetVAO() const override;
    size_t GetVertexCount() const override;
    size_t GetIndexCount() const override;
    bool HasIndices() const override;
    bool HasTexture() const override;

    // 设置顶点数据
    void SetVertexData(const std::vector<float>& vertices, size_t stride);
    void SetVertexLayout(const std::vector<size_t>& offsets, const std::vector<int>& sizes);

    // 设置索引数据
    void SetIndexData(const std::vector<unsigned int>& indices);

    // 设置纹理（使用 shared_ptr 管理所有权）
    void SetTexture(std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> GetTexture() const;

    // 设置材质颜色
    void SetMaterialColor(const glm::vec3& color);
    const glm::vec3& GetMaterialColor() const;

    // 静态工厂方法
    static SimpleMesh CreateFromCube();
    static SimpleMesh CreateFromMaterialData(const OBJModel::MaterialVertexData& materialData);
};

}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Create()` | 无 | void | 创建OpenGL缓冲对象（VAO、VBO、EBO） |
| `Draw()` | 无 | void | 绑定纹理并执行标准渲染 |
| `SetVertexData()` | vertices, stride | void | 设置顶点数据 |
| `SetVertexLayout()` | offsets, sizes | void | 设置顶点属性布局 |
| `SetIndexData()` | indices | void | 设置索引数据（用于EBO） |
| `SetTexture()` | shared_ptr<Texture> | void | 设置纹理（使用shared_ptr管理所有权） |
| `GetTexture()` | 无 | shared_ptr<Texture> | 获取纹理的shared_ptr |
| `SetMaterialColor()` | glm::vec3 color | void | 设置材质颜色 |
| `CreateFromCube()` | 无 | SimpleMesh | 静态方法：从立方体模板创建 |
| `CreateFromMaterialData()` | MaterialVertexData | SimpleMesh | 静态方法：从OBJ材质数据创建 |

#### 设计特点

**值语义（深拷贝）**：
- 拷贝构造函数和拷贝赋值运算符执行深拷贝
- 拷贝时会创建新的OpenGL缓冲对象（VAO、VBO、EBO）
- 移动构造和移动赋值显式实现以提高效率

**纹理管理**：
- 使用 `shared_ptr<Texture>` 管理纹理所有权
- 多个SimpleMesh可以共享同一个Texture
- 自动管理纹理生命周期

**职责清晰**：
- 纯粹的数据容器，不包含实例化逻辑
- 与 InstancedRenderer 配合使用：SimpleMesh 提供网格数据，InstancedRenderer 提供渲染逻辑

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

实例化渲染器 - 负责批量渲染多个相同几何体，大幅提升渲染性能。继承IRenderer接口，采用职责分离设计：SimpleMesh 存储网格数据，InstanceData 存储实例数据，InstancedRenderer 执行渲染逻辑。使用shared_ptr管理资源生命周期。

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
    void SetMesh(std::shared_ptr<SimpleMesh> mesh);

    // 设置实例数据（使用 shared_ptr 避免拷贝）
    void SetInstances(const std::shared_ptr<InstanceData>& data);

    // 设置材质颜色
    void SetMaterialColor(const glm::vec3& color);
    const glm::vec3& GetMaterialColor() const;

    // 设置纹理（使用 shared_ptr 管理所有权）
    void SetTexture(std::shared_ptr<Texture> texture);
    bool HasTexture() const;

    // 信息查询
    size_t GetInstanceCount() const;
    const std::shared_ptr<SimpleMesh>& GetMesh() const;

    // 静态工厂方法：为 Cube 创建实例化渲染器
    static InstancedRenderer CreateForCube(const std::shared_ptr<InstanceData>& instances);

    // 静态工厂方法：为 OBJ 模型创建实例化渲染器（返回多个渲染器，每个材质一个）
    // 同时返回 mesh 和 instanceData 的 shared_ptr 以保持生命周期
    static std::tuple<std::vector<InstancedRenderer>,
                      std::vector<std::shared_ptr<SimpleMesh>>,
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
| `SetMesh()` | shared_ptr<SimpleMesh> mesh | void | 设置网格模板（使用 shared_ptr 管理所有权） |
| `SetInstances()` | shared_ptr<InstanceData> data | void | 设置实例数据（使用 shared_ptr 避免拷贝） |
| `SetTexture()` | shared_ptr<Texture> texture | void | 设置纹理（使用 shared_ptr 管理所有权） |
| `SetMaterialColor()` | glm::vec3 color | void | 设置材质颜色 |
| `CreateForCube()` | shared_ptr<InstanceData> instances | InstancedRenderer | 静态方法：创建立方体实例化渲染器 |
| `CreateForOBJ()` | string objPath, shared_ptr<InstanceData> instances | tuple<渲染器vector, mesh的shared_ptrvector, instanceData的shared_ptr> | 静态方法：从OBJ模型创建多个材质渲染器 |

#### 职责分离设计

**架构（方案C：职责完全分离）**：
- ✅ **SimpleMesh**: 纯粹的数据容器，负责存储网格数据（顶点、索引、VAO），继承IMesh接口
- ✅ **InstanceData**: 实例数据容器，负责存储实例变换和颜色
- ✅ **InstancedRenderer**: 渲染逻辑，负责批量渲染多个实例，继承IRenderer接口

**所有权管理**：
- `InstancedRenderer` 使用 `shared_ptr<SimpleMesh>` 管理网格生命周期
- `InstancedRenderer` 使用 `shared_ptr<InstanceData>` 避免拷贝
- `InstancedRenderer` 使用 `shared_ptr<Texture>` 管理纹理所有权
- `CreateForOBJ()` 返回 tuple<渲染器vector, mesh的shared_ptrvector, instanceData的shared_ptr>
- 主程序需保持 mesh 和 instanceData 的 shared_ptr 存活
- 自动内存管理，消除悬空指针风险

#### 功能特性

**实例化渲染**：
- 单次绘制调用渲染数百个相同几何体
- 每个实例独立的变换矩阵（位置、旋转、缩放）
- 每个实例独立的颜色属性
- 使用 glVertexAttribDivisor 实现属性实例化

**材质支持**：
- 支持纹理映射（纹理指针由外部管理）
- 支持材质颜色（从OBJ文件的.mtl文件读取）
- 多材质OBJ模型：为每个材质创建独立的渲染器

**内存管理**：
- 使用 shared_ptr 自动管理 SimpleMesh 生命周期
- 主程序需要保持 mesh 的 shared_ptr 存活
- 实例数据存储在 InstanceData 中
- 支持动态更新实例缓冲（UpdateInstances）

#### 使用示例

```cpp
// 示例1：从立方体模板创建
// 1. 创建网格模板（使用 shared_ptr）
auto cubeMesh = std::make_shared<Renderer::SimpleMesh>(
    Renderer::SimpleMesh::CreateFromCube()
);
cubeMesh->Create();  // 创建 OpenGL 对象

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
cubeRenderer.SetMesh(cubeMesh);  // 传递 shared_ptr
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
auto [carRenderers, carMeshes, carInstanceData] =  // 接收渲染器、mesh和instanceData的shared_ptr
    Renderer::InstancedRenderer::CreateForOBJ(carPath, carInstances);

// 3. 渲染（每个材质一个draw call）
for (const auto& carRenderer : carRenderers) {
    shader.SetBool("useTexture", carRenderer.HasTexture());
    shader.SetVec3("objectColor", carRenderer.GetMaterialColor());
    shader.SetBool("useInstanceColor", false); // 使用材质颜色
    carRenderer.Render();
}
// 注意：carMeshes 和 carInstanceData 必须保持存活，直到渲染结束
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

## 使用示例

### 1. 基本窗口和输入设置

```cpp
#include "Core/Window.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp"

// 创建窗口
Core::Window window(1280, 720, "OpenGL Learning");
window.Init();

// 设置输入控制器
Core::MouseController mouseController;
mouseController.Initialize(window.GetGLFWwindow());

Core::KeyboardController keyboardController;
keyboardController.Initialize(window.GetGLFWwindow());

// 注册键盘事件
keyboardController.RegisterKeyCallback(GLFW_KEY_W, [&]() {
    // 处理W键按下
    camera.MoveForward(deltaTime);
});
```

### 2. 着色器和纹理使用

```cpp
#include "Renderer/Shader.hpp"
#include "Renderer/Texture.hpp"

// 加载着色器
Renderer::Shader shader;
shader.Load("assets/shader/basic.vert", "assets/shader/phong.frag");
shader.Use();

// 设置uniform变量
shader.SetMat4("model", modelMatrix);
shader.SetMat4("view", viewMatrix);
shader.SetMat4("projection", projectionMatrix);
shader.SetVec3("lightPos", lightPosition);

// 加载纹理
Renderer::Texture texture;
if (texture.LoadFromFile("assets/texture/brick.png")) {
    texture.Bind(GL_TEXTURE0);
    shader.SetInt("diffuseTexture", 0);
}
```

### 3. 几何体创建和渲染

```cpp
#include "Renderer/MeshFactory.hpp"

// 注册几何体类型
MeshFactory::Register("cube", []() { return std::make_unique<Cube>(); });
MeshFactory::Register("sphere", []() { return std::make_unique<Sphere>(); });

// 创建几何体
auto cube = MeshFactory::Create("cube");
auto sphere = MeshFactory::Create("sphere");

// 初始化
cube->Create();
sphere->Create();

// 渲染循环中
shader.Use();
cube->Draw();
sphere->Draw();
```

### 4. 实例化渲染使用

```cpp
#include "Renderer/InstancedRenderer.hpp"

// 示例1：批量渲染立方体
Renderer::InstancedRenderer instancedCubes = Renderer::InstancedRenderer::CreateFromCube(0);

// 添加实例（10x10网格）
for (int x = 0; x < 10; ++x) {
    for (int z = 0; z < 10; ++z) {
        glm::vec3 position(x * 2.0f, 0.0f, z * 2.0f);
        glm::vec3 rotation(0.0f, 0.0f, 0.0f);
        glm::vec3 scale(1.0f, 1.0f, 1.0f);
        glm::vec3 color(1.0f, 0.5f, 0.3f);
        instancedCubes.AddInstance(position, rotation, scale, color);
    }
}

instancedCubes.Create();

// 渲染（单次绘制调用）
shader.Use();
shader.SetBool("useTexture", false);
shader.SetBool("useInstanceColor", true);
instancedCubes.Draw();

// 示例2：批量渲染OBJ模型（多材质）
std::vector<Renderer::InstancedRenderer> instancedCarMeshes =
    Renderer::InstancedRenderer::CreateFromOBJ("assets/models/car.obj", 0);

// 为每个材质添加实例
for (auto& mesh : instancedCarMeshes) {
    for (int i = 0; i < 12; ++i) {
        float angle = (float)i / 12.0f * 6.28318f;
        glm::vec3 position(std::cos(angle) * 15.0f, 0.0f, std::sin(angle) * 15.0f);
        glm::vec3 rotation(0.0f, -angle * 57.2958f, 0.0f);
        glm::vec3 scale(0.5f, 0.5f, 0.5f);
        glm::vec3 color(1.0f, 1.0f, 1.0f);
        mesh.AddInstance(position, rotation, scale, color);
    }
    mesh.Create();
}

// 渲染（每个材质一次绘制调用）
for (const auto& carMesh : instancedCarMeshes) {
    shader.SetBool("useTexture", carMesh.HasTexture());
    shader.SetVec3("objectColor", carMesh.GetMaterialColor());
    shader.SetBool("useInstanceColor", false);
    carMesh.Draw();
}
```

---

## 📚 相关文档

- [TinyOBJLoader API 参考](TINYOBJ_API_REFERENCE.md) - 第三方OBJ加载库接口
- [架构设计文档](../ARCHITECTURE.md) - 项目整体架构分析
- [优化指南](../OPTIMIZATION_GUIDE.md) - 性能优化和扩展建议

---

*本文档描述了OpenGL学习项目的所有核心接口定义和使用方法* 🔧
