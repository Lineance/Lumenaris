# Lumenaris 架构文档

## 🏗️ 架构设计

### 1. 模块化架构

项目采用CMake构建系统，将功能模块化为独立的静态库和对象库：

#### 1.1 Core 模块 (静态库)

**Camera 类** (`src/Core/Camera.cpp`)

- 6自由度（6DOF）3D摄像机系统，支持透视和正交投影
- 欧拉角控制（偏航角、俯仰角），View/Projection Matrix计算
- 移动速度和鼠标灵敏度配置，LookAt功能

**Window 类** (`src/Core/Window.cpp`)

- 封装GLFW窗口管理，提供OpenGL上下文初始化和事件处理
- 支持动态窗口大小调整和投影矩阵更新

**MouseController 类** (`src/Core/MouseController.cpp`)

- 3D摄像机鼠标控制（旋转、缩放）
- 灵活的鼠标捕获和灵敏度配置
- 集成GLFW事件系统，支持回调机制

**KeyboardController 类** (`src/Core/KeyboardController.cpp`)

- 多键同时按下和按键状态查询
- 事件驱动的回调注册机制
- 按键重复和防抖功能

**Logger 类** (`src/Core/Logger.cpp`)

- 线程安全的异步日志系统
- 分级日志：DEBUG（Release版零开销）、INFO、WARNING、ERROR
- 日志轮转：按大小/天/小时，上下文感知（渲染阶段、批次索引等）
- 统计功能：着色器激活次数、纹理绑定次数、DrawCall次数、FPS监控

#### 1.2 Renderer 模块 (静态库)

**IRenderer 接口** (`include/Renderer/Core/IRenderer.hpp`)

- 统一抽象接口：Initialize()、Render()、GetName()
- 渲染器的抽象基类，定义渲染器的统一接口
- 实现类：InstancedRenderer（批量实例化渲染）

**光照系统** (`include/Renderer/Lighting/`)

- **Light 类**: 光源基类，支持DirectionalLight/PointLight/SpotLight
- **LightWithAttenuation 类**: 带衰减的光源基类，消除PointLight和SpotLight代码重复
- **LightHandle 类**: 光源句柄系统，使用`id + generation`机制，类型安全，仅可移动
- **LightManager 类**: 可实例化的光照管理器（非单例），每个RenderContext独立实例
  - 支持最多48个点光源、4个平行光、8个聚光灯
  - 使用`std::shared_mutex`实现线程安全的读写并发

**RenderContext 类** (`include/Renderer/Core/RenderContext.hpp`)

- 多Context架构核心，每个渲染场景（主场景、ImGui层、预览窗口）拥有独立Context
- 包含独立的光照、环境、渲染状态，避免全局状态污染

**环境渲染系统** (`include/Renderer/Environment/`)

- **Skybox 类**: 立方体贴图渲染，视差移除，无深度写入
- **SkyboxLoader 类**: 支持多种cubemap约定（OpenGL/DirectX/Maya/Blender），自动转换面顺序
- **AmbientLighting 类**: 轻量级环境光照（SOLID_COLOR/SKYBOX_SAMPLE/HEMISPHERE三种模式）

**数据容器** (`src/Renderer/Data/`)

- **MeshData**: CPU端网格数据容器（顶点、索引、顶点布局）
- **MeshBuffer**: GPU端网格缓冲区（VAO/VBO/EBO），UploadToGPU()上传数据
- **InstanceData**: 实例数据容器（变换矩阵、颜色），支持批量操作

**工厂模式** (`src/Renderer/Factory/`)

- **MeshDataFactory**: 创建CPU端MeshData和GPU端MeshBuffer
  - 参数化工厂方法：`CreateTorusData()`、`CreatePlaneData()`等参数正确传递

**InstancedRenderer 类** (`src/Renderer/Renderer/InstancedRenderer.cpp`)

- 实例化渲染器，继承IRenderer接口
- 职责分离：MeshBuffer（GPU资源）+ InstanceData（实例数据）+ InstancedRenderer（渲染逻辑）
- 每个实例独立变换矩阵和颜色属性，使用`glVertexAttribDivisor`实现
- 多材质支持：每个材质创建独立实例化渲染器
- 内存安全：`shared_ptr`管理SimpleMesh、InstanceData、Texture生命周期
- **动态更新**: `UpdateInstanceData()`使用`glBufferSubData`高效更新GPU缓冲区

#### 1.3 Geometry 模块 (对象库)

**纯静态工具类设计**（2026-01-02重构）

所有几何体类已重构为纯静态工具类，禁止实例化，只负责数据生成。

- **Cube 类**: 立方体顶点数据生成
  - `GetVertexData()`: 获取24个顶点数据（位置3+法线3+UV2）
  - `GetVertexLayout()`: 获取顶点属性布局

- **Sphere 类**: 球体顶点和索引数据生成（支持参数化）
  - `GetVertexData(radius, stacks, slices)`: 参数化生成顶点数据
  - `GetIndexData(stacks, slices)`: 生成索引数据
  - `GetVertexLayout()`: 获取顶点属性布局

- **Torus 类**: 圆环体顶点和索引数据生成（支持参数化）
  - `GetVertexData(majorRadius, minorRadius, majorSegments, minorSegments)`: 参数化生成
  - `GetIndexData(majorSegments, minorSegments)`: 生成索引数据
  - `GetVertexLayout()`: 获取顶点属性布局

- **Plane 类**: 平面顶点和索引数据生成（支持参数化）
  - `GetVertexData(width, height, widthSegments, heightSegments)`: 参数化生成
  - `GetIndexData(widthSegments, heightSegments)`: 生成索引数据
  - `GetVertexLayout()`: 获取顶点属性布局

- **OBJModel 类**: OBJ模型数据加载和生成（纯静态类）
  - `GetMaterialVertexData(objPath)`: 按材质分离的顶点数据（用于多材质渲染）
  - `GetMeshData(objPath)`: 获取单个MeshData（合并所有材质）
  - `GetMaterials(objPath)`: 获取材质列表
  - `HasMaterials(objPath)`: 检查模型是否包含材质
  - `GetVertexLayout()`: 获取顶点属性布局

- **OBJLoader 类** (`src/Renderer/Resources/OBJLoader.cpp`)
  - 完整的OBJ文件格式解析器（顶点、法线、UV）
  - 集成TinyOBJLoader，自动加载.mtl材质文件
  - 支持多材质模型的面索引管理

**架构优势**：

- ✅ **职责分离**: 几何体类只生成数据，不涉及GPU操作
- ✅ **无实例状态**: 所有方法都是静态的，线程安全
- ✅ **参数化支持**: Sphere、Torus、Plane 支持运行时参数调整
- ✅ **代码精简**: 相比实例类，代码量减少54-62%

**使用方式**:

```cpp
// ✅ 推荐：通过 MeshDataFactory 创建GPU缓冲区
auto cubeBuffer = MeshDataFactory::CreateCubeBuffer();
auto sphereBuffer = MeshDataFactory::CreateSphereBuffer(32, 32, 1.0f);
auto objBuffers = MeshDataFactory::CreateOBJBuffers("models/car.obj");

// ✅ 或直接使用静态方法获取数据
auto vertices = Renderer::Cube::GetVertexData();
auto meshData = Renderer::OBJModel::GetMeshData("models/bunny.obj");
```

#### 1.4 主程序 (`src/main.cpp`)

- 应用程序入口，渲染循环，系统初始化
- **🎪✨ 超级宇宙迪斯科舞台**：
  - 📦 1600立方体 + 🍩 5圆环 + 🎯 39平台 + 🐇 狂舞兔子
  - 🌐 9球体 + 💡48动态光源 + 🌌 宇宙天空盒
  - 🚗 1辆行驶的跑车（真实物理运动）
  - 🎉 Enjoy the chaos! 🎆
- **天空盒系统**: Corona天空盒加载，环境光照集成

### 2. 构建系统

**CMakeLists.txt** 配置：

- **Core 库**: 静态库（窗口和输入系统）
- **Renderer 库**: 静态库（着色器和纹理）
- **Geometry 对象库**: 所有几何体实现
- **HelloWindow 可执行文件**: 主程序

依赖管理：GLFW、GLM、GLAD、STB、TinyOBJLoader

### 3. 设计模式应用

#### 3.1 工厂模式 (MeshDataFactory)

```cpp
// 编译时工厂，类型安全
auto cubeBuffer = MeshDataFactory::CreateCubeBuffer();
auto sphereData = MeshDataFactory::CreateSphereData(64, 64, 1.0f);
```

**优势**: 编译时类型检查，性能优于运行时工厂

#### 3.2 观察者模式 (输入系统)

```cpp
keyboardController.RegisterKeyCallback(GLFW_KEY_W, []() { /* 处理W键 */ });
```

**优势**: 解耦事件产生者和消费者

#### 3.3 策略模式 (渲染风格)

```cpp
// 运行时切换着色器
std::vector<Renderer::Shader> shaders;
for (const auto& fragPath : fragShaders) {
    Renderer::Shader shader;
    shader.Load("assets/shader/basic.vert", fragPath);
    shaders.push_back(std::move(shader));
}
```

**优势**: 支持多种渲染策略，提高代码复用性

### 3.4 脏标记模式（动画优化 2026-01-02）

**多材质模型动画的脏标记管理**：

```cpp
// 正确的多材质动画更新流程
for (auto& renderer : car.renderers) {
    renderer.UpdateInstanceData();  // 所有38个渲染器都更新
}

// ⚠️ 关键：调用者统一清除脏标记
if (car.instanceData->IsDirty()) {
    car.instanceData->ClearDirty();
}
```

**设计原则**：
- 脏标记由调用者管理，不在被调用者中自动清除
- 避免第一个渲染器清除标记后，其余渲染器跳过更新
- 适用于多个消费者共享同一个状态标志的场景

**应用场景**：
- 多材质OBJ模型的动画（38个材质共享1个InstanceData）
- 批量渲染的动态更新
- 缓存失效同步

## 🔍 架构分析与优化

### 1. 当前架构的优势

#### 1.1 模块化设计优势

- **清晰的职责分离**: Core/Renderer/Geometry 模块各司其职
  - Core: 窗口、输入、摄像机、日志
  - Renderer: GPU资源、光照、环境、渲染器
  - Geometry: 纯静态数据生成
- **低耦合高内聚**: 各模块接口简洁，依赖关系清晰
- **便于扩展**: 新的几何体和渲染功能可轻松集成

#### 1.2 几何体架构重构优势（2026-01-02）

- **职责分离**: 几何体类只负责数据生成，不涉及GPU操作
- **无状态设计**: 纯静态类，线程安全，无实例开销
- **参数化支持**: Sphere、Torus、Plane 支持运行时参数
- **代码精简**: 相比重构前，代码量减少54-62%
  - Cube: 130行 → 59行（54%减少）
  - Sphere: 231行 → 88行（62%减少）
  - Plane: 216行 → 88行（59%减少）
  - Torus: 180行 → 99行（45%减少）
  - OBJModel: 390行 → 177行（55%减少）

#### 1.3 设计模式应用效果

- **工厂模式**: MeshDataFactory 编译时工厂，类型安全，性能优越
- **观察者模式**: 输入系统采用事件驱动架构
- **策略模式**: 着色器系统支持多套渲染策略
- **脏标记模式**: 多材质动画优化，调用者管理状态生命周期

#### 1.4 批量渲染优化（RenderBatch 2026-01-02）

**性能优化**：
- 按纹理分组渲染，减少OpenGL状态切换60-70%
- 支持三种重载：指针vector、unique_ptr vector、值类型vector
- 使用`std::map`保持纹理顺序稳定

**性能对比**：
- 12辆车 × 38材质：456次 DrawCall → 38次 DrawCall（12倍提升）
- Disco舞台（46渲染器）：状态切换减少67%


