# OpenGL学习项目接口文档

## 📋 目录

- [Core 模块接口](#core-模块接口)
  - [Window 类](#window-类)
  - [MouseController 类](#mousecontroller-类)
  - [KeyboardController 类](#keyboardcontroller-类)
- [Renderer 模块接口](#renderer-模块接口)
  - [IMesh 抽象接口](#imesh-抽象接口)
  - [MeshFactory 工厂类](#meshfactory-工厂类)
  - [Shader 类](#shader-类)
  - [Texture 类](#texture-类)
  - [Cube 类](#cube-类)
  - [Sphere 类](#sphere-类)
  - [OBJModel 类](#objmodel-类)
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
};
}
```

#### 接口说明

| 方法 | 参数 | 返回值 | 说明 |
|------|------|--------|------|
| `Create()` | 无 | void | 初始化网格的顶点缓冲对象和数组对象 |
| `Draw()` | 无 | void | 执行网格的渲染操作 |

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

---

## 📚 相关文档

- [TinyOBJLoader API 参考](TINYOBJ_API_REFERENCE.md) - 第三方OBJ加载库接口
- [架构设计文档](../ARCHITECTURE.md) - 项目整体架构分析
- [优化指南](../OPTIMIZATION_GUIDE.md) - 性能优化和扩展建议

---

*本文档描述了OpenGL学习项目的所有核心接口定义和使用方法* 🔧
