# Lumenaris

一个基于OpenGL 3.3的轻量级3D渲染引擎，专注OBJ模型加载、实例化渲染与风格化着色。采用现代C++17模块化架构，适合3D图形学习与实时渲染演示。

**Lumenaris** - 源自拉丁语 *lumen*（光）+ *aris*（带来...的），意为"带来光明的"，象征这个OpenGL学习项目为3D图形学之路带来启蒙与指引。

---

## ✨ 核心特性

- **高性能实例化渲染**：单次绘制调用渲染1000+物体
- **完整OBJ工作流**：支持多材质/纹理/变换，自动解析.mtl材质文件
- **8种风格化着色器**：卡通、玻璃、墨水、霓虹、像素噪点、素描
- **天空盒系统**：轻量级环境光照（IBL），与Phong光照系统集成
- **智能资源管理**：`shared_ptr`自动管理`MeshBuffer`/纹理生命周期，零内存泄漏
- **异步日志系统**：后台线程写入，Release编译零开销

<div align="center">
🎪✨ 测试场景：超级宇宙迪斯科舞台 🕺💃 <br>
📦 1600立方体 + 🍩 5圆环 + 🎯 39平台 + 🐇 狂舞兔子 <br>
🌐 9球体 + 💡48动态光源 + 🌌 宇宙天空盒 <br>
🎉 Enjoy the chaos! 🎆
</div>

<div align="center">
  <img src="./assets/picture/EXAMPLE.gif" alt="超级宇宙迪斯科">
</div>

---

## 🛠️ 技术栈

| 技术 | 版本 | 用途 |
|------|------|------|
| OpenGL | 3.3 Core | 渲染API |
| C++ | 17 | 开发语言 |
| GLFW | 3.4 | 窗口与输入管理 |
| GLM |  | 3D数学运算 |
| GLAD | 0.1.36 | OpenGL函数加载 |
| TinyOBJLoader | 1.0.6 | OBJ模型解析 |
| STB Image | 2.30 | 纹理加载 |

---

## 🚀 快速开始

### 环境要求

- CMake 3.15+
- 支持OpenGL 3.3的显卡
- C++17编译器（GCC 7+, Clang 5+, MSVC 2017+）

### 构建项目

```bash
git clone <repository-url>
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

---

## 📖 使用示例

### 1. 基础几何体渲染

```cpp
// 创建立方体缓冲区（工厂模式）
auto cubeBuffer = std::make_shared<Renderer::MeshBuffer>(
    Renderer::MeshBufferFactory::CreateCubeBuffer()
);

// 准备实例数据（100个立方体）
auto instances = std::make_shared<Renderer::InstanceData>();
for (int x = 0; x < 10; ++x) {
    for (int z = 0; z < 10; ++z) {
        instances->Add(
            glm::vec3(x * 2.0f, 0.0f, z * 2.0f), // 位置
            glm::vec3(0.0f),                      // 旋转
            glm::vec3(1.0f),                      // 缩放
            glm::vec3(1.0f, 0.5f, 0.3f)           // 颜色
        );
    }
}

// 创建渲染器并初始化
Renderer::InstancedRenderer renderer;
renderer.SetMesh(cubeBuffer);
renderer.SetInstances(instances);
renderer.Initialize();

// 渲染循环
shader.Use();
renderer.Render(); // 一次绘制调用渲染100个立方体
```

### 2. OBJ模型多材质渲染

```cpp
// 加载跑车模型（38个材质）
std::string carPath = "assets/models/cars/sportsCar.obj";
auto carInstances = std::make_shared<Renderer::InstanceData>();
// ... 添加12辆车实例 ...

// 从OBJ创建渲染器（每个材质独立）
auto [carRenderers, carMeshBuffers, carInstanceData] = 
    Renderer::InstancedRenderer::CreateForOBJ(carPath, carInstances);

// 渲染（38次DrawCall替代传统456次）
for (auto& renderer : carRenderers) {
    shader.SetBool("useTexture", renderer.HasTexture());
    renderer.Render();
}
```

### 3. 动态动画更新

```cpp
// 每帧更新实例变换
float time = glfwGetTime();
auto& matrices = instances->GetModelMatrices();

for (size_t i = 0; i < matrices.size(); ++i) {
    float angle = time + i * 0.1f;
    matrices[i] = glm::translate(glm::mat4(1.0f), positions[i]) *
                  glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
}

// 高效上传到GPU（不重新分配内存）
renderer.UpdateInstanceData();
renderer.Render();
```

---

## 📁 项目结构

```
Lumenaris/
├── assets/                    # 资源文件
│   ├── models/               # 3D模型（跑车、云朵、康奈尔盒）
│   ├── shader/              # 着色器（8种风格 + 天空盒 + 环境光照）
│   ├── textures/            # 纹理资源（含天空盒）
│   └── picture/             # 纹理资源
├── include/                  # 头文件
│   ├── Core/                # 核心系统（窗口、摄像机、输入）
│   └── Renderer/            # 渲染系统
│       ├── Core/            # IRenderer接口
│       ├── Data/            # MeshData/InstanceData
│       ├── Environment/     # 环境渲染（天空盒、环境光照）
│       ├── Factory/         # 网格工厂
│       ├── Geometry/        # 几何体实现
│       ├── Lighting/        # 光照系统
│       ├── Renderer/        # 渲染器实现
│       └── Resources/       # 资源加载器
├── src/                      # 源代码
│   ├── Core/                # 核心系统实现
│   └── Renderer/            # 渲染系统实现
│       ├── Data/            # 数据容器
│       ├── Environment/     # 环境渲染实现
│       ├── Factory/         # 工厂实现
│       ├── Geometry/        # 几何体实现
│       ├── Lighting/        # 光照系统实现
│       ├── Renderer/        # 渲染器实现
│       └── Resources/       # 资源实现
├── vendor/                  # 第三方库
├── docs/                    # 文档
│   ├── ARCHITECTURE.md     # 架构详解
│   └── INTERFACES.md       # 接口文档
├── CMakeLists.txt          # 构建配置
└── README.md               # 本文件
```

---

## 🎨 着色器风格预览

| 风格 | 文件名 | 效果描述 |
|------|--------|----------|
| 基础 | `basic.frag` | Phong光照 |
| 卡通 | `toon.frag` | 色阶着色 |
| 玻璃 | `glass.frag` | 折射+透明 |
| 墨水 | `ink.frag` | 轮廓描边 |
| 霓虹 | `neon.frag` | 发光边缘 |
| 像素噪点 | `pixelnoise.frag` | 像素化 |
| 素描 | `sketch.frag` | 手绘质感 |

**切换方式**：

```cpp
std::vector<Renderer::Shader> shaders;
for (const auto& path : stylePaths) {
    Renderer::Shader shader;
    shader.Load("basic.vert", path);
    shaders.push_back(std::move(shader));
}
// 运行时切换：shaders[styleIndex].Use();
```

---

## 🎨 天空盒系统

### 支持的Cubemap约定

- **OpenGL**：right, left, top, bottom, back, front
- **DirectX**：left, right, top, bottom, front, back
- **Maya/Corona**：rt, lf, up, dn, bk, ft
- **HDR Lab**：px, nx, py, ny, pz, nz

### 灵活的加载方式

```cpp
// 方式1：完整自定义文件名
auto config = SkyboxLoader::CreateCustomConfig(
    "assets/textures/skybox",
    {"corona_rt.png", "corona_lf.png", "corona_up.png",
     "corona_dn.png", "corona_bk.png", "corona_ft.png"},
    CubemapConvention::OPENGL
);

// 方式2：基于约定和命名模式
auto config = SkyboxLoader::CreateFromPattern(
    "assets/textures/skybox",
    "corona_{face}",
    CubemapConvention::MAYA,
    ".png"
);

// 方式3：完全自定义面名称后缀
FaceNamingScheme custom("rt", "lf", "up", "dn", "bk", "ft");
auto config = SkyboxLoader::CreateFromCustomScheme(
    "assets/textures/skybox",
    "corona_{face}",
    custom,
    CubemapConvention::OPENGL,
    ".png"
);
```

### 环境光照模式

- **SOLID_COLOR**：传统Phong固定颜色环境光
- **SKYBOX_SAMPLE**：从天空盒采样IBL环境光
- **HEMISPHERE**：半球渐变环境光（天空/地面插值）

---

## 🛣️ 不开发路线图

### P-1 - 潜在问题（目前放弃）

- ❌ **天空盒**：存在状态泄露的风险（当有异常产生状态未恢复），需要 RAII管理 OpenGLContext，拓展imgui时需要关注Context的恢复！！！

### P0 - 核心组件（目前放弃）

- ❌ **场景图系统**：`SceneNode`层级管理
- ❌ **PBR材质系统**：金属度/粗糙度工作流
- ❌ **资源管理器**：自动缓存+异步加载
- ❌ **IBL**：环境光照
- ❌ **Framebuffer**

### P1 - 锦上添花（目前放弃）

- ❌ **渲染管线抽象**：前向/延迟渲染切换与Bloom
- ❌ **阴影系统**：强制性多Pass，否则性能损失严重
- ❌ **粒子系统**：与实例化架构冲突
- ❌ **骨骼动画**：超出学习范围
- ❌ **PBR真实光照**：项目定位风格化渲染
- ❌ **音频/网络**：非渲染核心

### P2 - 极致性能（目前放弃）

- ❌ **视锥剔除**：Frustum Culling
- ❌ **渲染状态批处理**：State Batching
- ❌ **纹理压缩与MipMap优化**
- ❌ **Uniform Buffer Object (UBO)**

---

## 特别注意

- 不要管任何archive里面的文件，项目经过重构数次，早已不适配。

---

## 🤝 贡献指南

1. **代码规范**：遵循C++17标准，使用现代智能指针
2. **架构约束**：新增功能需符合`IRenderer`接口，保持职责分离
3. **性能优先**：渲染循环避免堆分配，优先使用实例化
4. **文档同步**：修改接口需更新`docs/INTERFACES.md`

---

## 📄 许可证

MIT License - 详见 `LICENSE` 文件

---

## 📞 联系与反馈

这是一个学习导向的渲染引擎项目，欢迎提交Issue讨论架构设计、性能优化与渲染技术。
