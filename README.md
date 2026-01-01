# DarkRoomEngine

一个基于OpenGL 3.3的轻量级3D渲染引擎，专注OBJ模型加载、实例化渲染与风格化着色。采用现代C++17模块化架构，适合3D图形学习与实时渲染演示。

---

## ✨ 核心特性

- **高性能实例化渲染**：单次绘制调用渲染1000+物体，性能提升10-100倍
- **完整OBJ工作流**：支持多材质/纹理/变换，自动解析.mtl材质文件
- **8种风格化着色器**：卡通、玻璃、墨水、霓虹、像素噪点、素描
- **智能资源管理**：`shared_ptr`自动管理`MeshBuffer`/纹理生命周期，零内存泄漏
- **异步日志系统**：后台线程写入，Release编译零开销

- 测试场景为超级迪斯科舞台，enjoy

---

## 🛠️ 技术栈

| 技术 | 版本 | 用途 |
|------|------|------|
| OpenGL | 3.3 Core | 渲染API |
| C++ | 17 | 开发语言 |
| GLFW | 3.x | 窗口与输入管理 |
| GLM |  | 3D数学运算 |
| GLAD |  | OpenGL函数加载 |
| TinyOBJLoader |  | OBJ模型解析 |
| STB Image |  | 纹理加载 |

---

## 🚀 快速开始

### 环境要求

- CMake 3.15+
- 支持OpenGL 3.3的显卡
- C++17编译器（GCC 7+, Clang 5+, MSVC 2017+）

### 构建项目

```bash
git clone <repository-url>
cd DarkRoomEngine
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### 运行演示

```bash
./HelloWindow
```

**操作说明**：

- `WASD`：前后左右移动
- `QE`：垂直飞行
- `鼠标`：3D视角旋转
- `TAB`：切换鼠标捕获
- `ESC`：退出

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
DarkRoomEngine/
├── assets/                    # 资源文件
│   ├── models/               # 3D模型（跑车、云朵、康奈尔盒）
│   ├── shader/              # 着色器（8种风格）
│   └── picture/             # 纹理资源
├── include/                  # 头文件
│   ├── Core/                # 核心系统（窗口、摄像机、输入）
│   └── Renderer/            # 渲染系统
│       ├── Core/            # IRenderer接口
│       ├── Data/            # MeshData/InstanceData
│       ├── Geometry/        # 几何体实现
│       ├── Lighting/        # 光照系统
│       └── Resources/       # 资源加载器
├── src/                      # 源代码
├── vendor/                  # 第三方库
├── docs/                    # 文档
│   ├── ARCHITECTURE.md     # 架构详解
│   └── interfaces/         # 接口文档
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

**优化成果**：

- **实例化渲染**：减少90% DrawCall
- **异步日志**：主线程无阻塞
- **智能指针**：零内存泄漏，自动回收

---

## 🛣️ 开发路线图

### P0 - 核心组件（计划实现）

- [ ] **场景图系统**（`SceneNode`层级管理）
- [ ] **PBR材质系统**（金属度/粗糙度工作流）
- [ ] **资源管理器**（自动缓存+异步加载）

### P1 - 重要增强

- [ ] **阴影系统**（方向光/点光源阴影贴图）
- [ ] **天空盒/IBL**（环境光照）
- [ ] **渲染管线抽象**（前向/延迟渲染切换）

### P2 - 锦上添花（明确放弃）

- ❌ **粒子系统**（与实例化架构冲突）
- ❌ **骨骼动画**（超出学习范围）
- ❌ **PBR真实光照**（项目定位风格化渲染）
- ❌ **音频/网络**（非渲染核心）

---

## 🤝 贡献指南

1. **代码规范**：遵循C++17标准，使用现代智能指针
2. **架构约束**：新增功能需符合`IRenderer`接口，保持职责分离
3. **性能优先**：渲染循环避免堆分配，优先使用实例化
4. **文档同步**：修改接口需更新`docs/interfaces/INTERFACES.md`

---

## 📄 许可证

MIT License - 详见 `LICENSE` 文件

---

## 📞 联系与反馈

这是一个学习导向的渲染引擎项目，欢迎提交Issue讨论架构设计、性能优化与渲染技术。
