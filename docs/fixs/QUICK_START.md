# 快速入门指南

## 🚀 快速开始

这是一个现代 OpenGL 3D 渲染学习项目，展示了实例化渲染、移动语义优化和模块化架构设计。

### 编译和运行

```bash
# 1. 创建构建目录
mkdir -p build && cd build

# 2. 配置 CMake（Release 模式获得最佳性能）
cmake -DCMAKE_BUILD_TYPE=Release ..

# 3. 编译
make -j$(nproc)

# 4. 运行炫酷演示程序
./HelloWindow
```

### 预期效果

运行后你会看到一个包含 **5000+ 动态立方体** 的炫酷 3D 场景，保持 60+ FPS！

- **场景 1**: 螺旋塔（360 个立方体）
- **场景 2**: 波浪地板（1600 个立方体）
- **场景 3**: 浮动群岛（1200 个立方体）

按 `1`/`2`/`3` 键切换场景，`WASD` 移动，`TAB` 释放鼠标。

---

## 📁 项目结构

```
LearningOpenGL/
├── src/main.cpp                  # ⭐ 主程序（炫酷演示）
├── include/
│   ├── Core/                     # 核心系统（窗口、输入、日志）
│   └── Renderer/                 # 渲染系统
│       ├── MeshBuffer.hpp        # GPU 网格资源（移动语义优化）
│       ├── InstancedRenderer.hpp # 实例化渲染器
│       └── InstanceData.hpp      # 实例数据容器
├── docs/
│   ├── COOL_CUBES_DEMO.md        # 演示程序详细说明
│   └── MESHBUFFER_PERFORMANCE_OPTIMIZATION.md  # 性能优化报告
└── assets/
    ├── shader/                   # GLSL 着色器
    └── models/                   # 3D 模型文件
```

---

## 🎯 核心特性

### 1. 实例化渲染

渲染 **5000+ 个立方体** 只需 **1 次绘制调用**！

```cpp
// 传统方法：5000 次调用
for (int i = 0; i < 5000; i++) {
    drawCube(cube[i]);  // 慢！
}

// 实例化渲染：1 次调用
renderer.Render();  // 快！
```

### 2. 移动语义优化

零拷贝资源传递，性能提升 **30-50%**！

```cpp
// 优化前：拷贝整个 vector（慢）
data.SetVertices(vertices, stride);

// 优化后：移动语义（快）
data.SetVertices(std::move(vertices), stride);
```

### 3. 模块化架构

职责清晰分离，易于扩展和维护：

- **MeshBuffer**: GPU 资源管理
- **InstanceData**: 实例数据存储
- **InstancedRenderer**: 渲染逻辑执行

---

## 📚 学习路径

### 初学者

1. **运行演示程序** - 感受实例化渲染的威力
2. **阅读主程序** - `src/main.cpp`（详细注释）
3. **理解架构** - `docs/ARCHITECTURE.md`

### 进阶开发者

1. **性能优化** - `docs/MESHBUFFER_PERFORMANCE_OPTIMIZATION.md`
2. **实例化渲染** - `docs/fixs/INSTANCED_RENDERING_GUIDE.md`
3. **接口文档** - `docs/interfaces/INTERFACES.md`

### 想要修改代码？

**创建自己的场景**（在 `main.cpp` 中）：

```cpp
std::shared_ptr<Renderer::InstanceData> CreateMyScene()
{
    auto instances = std::make_shared<Renderer::InstanceData>();
    instances->Reserve(1000);  // 预分配内存

    // 添加 1000 个立方体
    for (int i = 0; i < 1000; ++i) {
        glm::vec3 pos(i * 2.0f, 0.0f, 0.0f);
        glm::vec3 rot(0.0f, 0.0f, 0.0f);
        glm::vec3 scale(1.0f);
        glm::vec3 color(1.0f, 0.0f, 0.0f);  // 红色
        instances->Add(pos, rot, scale, color);
    }

    return instances;
}
```

---

## 🎮 控制说明

| 按键 | 功能 |
|------|------|
| `W/A/S/D` | 移动摄像机 |
| `Q/E` | 上下移动 |
| `鼠标` | 旋转视角 |
| `TAB` | 切换鼠标捕获 |
| `1/2/3` | 切换场景 |
| `ESC` | 退出 |

---

## 🔧 常见问题

### Q: 编译出错？

确保安装了依赖（Ubuntu/Debian）：
```bash
sudo apt install build-essential cmake libglfw3-dev
```

### Q: FPS 很低？

使用 Release 模式编译：
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

### Q: 鼠标无法移动？

按 `TAB` 键切换鼠标捕获模式。

### Q: 想要了解更多？

阅读完整文档：
- **演示程序说明**: `docs/COOL_CUBES_DEMO.md`
- **架构设计**: `docs/ARCHITECTURE.md`
- **性能优化**: `docs/MESHBUFFER_PERFORMANCE_OPTIMIZATION.md`

---

## 📊 性能数据

| 场景 | 立方体数量 | FPS | 绘制调用 |
|------|-----------|-----|---------|
| 螺旋塔 | 360 | 120+ | 1 |
| 波浪地板 | 1,600 | 80+ | 1 |
| 浮动群岛 | 1,200 | 90+ | 1 |

**测试环境**: 1920x1080, Release 模式

---

## 🌟 技术亮点

- ✅ **OpenGL 3.3 Core** - 现代管线
- ✅ **实例化渲染** - 单次调用渲染千个对象
- ✅ **移动语义** - 零拷贝优化
- ✅ **模块化架构** - 清晰的职责分离
- ✅ **完整日志系统** - 性能监控
- ✅ **流畅控制** - WASD + 鼠标

---

## 📖 推荐阅读顺序

1. **本文档** - 快速了解项目
2. **运行演示** - 感受性能威力
3. **COOL_CUBES_DEMO.md** - 详细演示说明
4. **main.cpp** - 完整代码实现
5. **性能优化文档** - 深入理解优化

---

**祝学习愉快！** 🎉

如有问题，请查看 `docs/` 目录下的详细文档。
