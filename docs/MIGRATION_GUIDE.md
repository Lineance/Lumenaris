# 架构迁移指南

## 📋 迁移概述

本项目已从旧的**混合职责架构**完全迁移到新的**职责分离架构**（方案C）。

**迁移日期**: 2025年
**旧架构**: `InstancedMesh` (混合职责)
**新架构**: `SimpleMesh` + `InstanceData` + `InstancedRenderer` (职责分离)

---

## 🔄 主要变更

### 1. 文件变更

#### 删除的文件（旧架构）
```
❌ include/Renderer/InstancedRenderer.hpp
❌ src/Renderer/InstancedRenderer.cpp
```

#### 新增的文件（新架构）
```
✅ include/Renderer/IRenderer.hpp              # 渲染器接口
✅ include/Renderer/InstanceData.hpp           # 实例数据容器
✅ include/Renderer/SimpleMesh.hpp             # 简单网格
✅ include/Renderer/InstancedRenderer.hpp     # 新实例化渲染器
✅ src/Renderer/InstanceData.cpp
✅ src/Renderer/SimpleMesh.cpp
✅ src/Renderer/InstancedRenderer.cpp
```

#### 修改的文件
```
📝 src/main.cpp                                # 完全重写
📝 test/test_instanced_rendering.cpp           # 完全重写
📝 CMakeLists.txt                              # 更新文件列表
📝 include/Renderer/Mesh.hpp                   # 扩展 IMesh 接口
📝 include/Renderer/Cube.hpp                   # 实现 IMesh 扩展
📝 include/Renderer/Sphere.hpp                 # 实现 IMesh 扩展
📝 include/Renderer/OBJModel.hpp               # 实现 IMesh 扩展
```

#### 备份的文件
```
📦 archive/main_old_architecture.cpp           # 旧版 main.cpp 备份
```

---

## 🔄 API 变更对照表

### 旧 API（混合职责）

```cpp
// 创建和添加实例（混合在一起）
InstancedMesh mesh = InstancedMesh::CreateFromCube();
mesh.AddInstance(pos, rot, scale, color);
mesh.AddInstance(pos2, rot2, scale2, color2);
mesh.Create();
mesh.Draw();
```

### 新 API（职责分离）

```cpp
// 1. 创建网格（数据）
SimpleMesh cubeMesh = SimpleMesh::CreateFromCube();
cubeMesh.Create();

// 2. 准备实例数据（数据）
InstanceData instances;
instances.Add(pos, rot, scale, color);
instances.Add(pos2, rot2, scale2, color2);

// 3. 创建渲染器（逻辑）
InstancedRenderer renderer;
renderer.SetMesh(cubeMesh);
renderer.SetInstances(instances);
renderer.Initialize();

// 4. 渲染
renderer.Render();
```

---

## 📊 类名映射

| 旧架构类名 | 新架构类名 | 说明 |
|-----------|-----------|------|
| `InstancedMesh` | `SimpleMesh` | 纯网格数据 |
| `InstancedMesh` | `InstanceData` | 实例数据容器 |
| `InstancedMesh` | `InstancedRenderer` | 渲染逻辑 |
| N/A | `IRenderer` | 新增渲染器接口 |

---

## 🎯 迁移步骤

### 第1步：替换头文件

**旧代码**:
```cpp
#include "Renderer/InstancedRenderer.hpp"
```

**新代码**:
```cpp
#include "Renderer/SimpleMesh.hpp"
#include "Renderer/InstancedRenderer.hpp"
#include "Renderer/InstanceData.hpp"
```

---

### 第2步：拆分网格创建

**旧代码**:
```cpp
InstancedMesh mesh = InstancedMesh::CreateFromCube();
mesh.AddInstance(pos, rot, scale, color);
```

**新代码**:
```cpp
SimpleMesh mesh = SimpleMesh::CreateFromCube();
mesh.Create();

InstanceData instances;
instances.Add(pos, rot, scale, color);
```

---

### 第3步：使用渲染器

**旧代码**:
```cpp
mesh.Create();
mesh.Draw();
```

**新代码**:
```cpp
InstancedRenderer renderer;
renderer.SetMesh(mesh);
renderer.SetInstances(instances);
renderer.Initialize();
renderer.Render();
```

---

### 第4步：OBJ 模型渲染

**旧代码**:
```cpp
std::vector<InstancedMesh> meshes =
    InstancedMesh::CreateFromOBJ(path, 0);

for (auto& mesh : meshes) {
    mesh.AddInstance(pos, rot, scale, color);
    mesh.Create();
    mesh.Draw();
}
```

**新代码**:
```cpp
InstanceData instances;
instances.Add(pos, rot, scale, color);

std::vector<InstancedRenderer> renderers =
    InstancedRenderer::CreateForOBJ(path, instances);

for (auto& renderer : renderers) {
    renderer.Render();
}
```

---

## ✅ 迁移检查清单

在完成迁移后，请确保：

- [ ] 所有 `#include "Renderer/InstancedRenderer.hpp"` 已替换
- [ ] 所有 `InstancedMesh` 类型已替换为适当的类型
- [ ] 网格创建使用 `SimpleMesh::CreateFromCube()`
- [ ] 实例数据使用 `InstanceData` 类
- [ ] 渲染使用 `InstancedRenderer` 类
- [ ] 项目成功编译（`cmake -B build && cd build && make`）
- [ ] 程序成功运行并渲染正确

---

## 📈 性能影响

新架构的性能特性：

| 指标 | 旧架构 | 新架构 | 变化 |
|-----|--------|--------|------|
| 绘制调用 | 1次 | 1次 | ✅ 无变化 |
| 内存使用 | 中等 | 中等 | ✅ 无变化 |
| CPU开销 | 低 | 低 | ✅ 无变化 |
| 代码复杂度 | 中 | 低 | ✅ 改善 |
| 可维护性 | 中 | 高 | ✅ 大幅改善 |

---

## 🐛 常见问题

### Q1: 编译错误 "找不到 InstancedRenderer.hpp"

**解决方案**: 确保已替换所有包含路径：
```cpp
// 旧
#include "Renderer/InstancedRenderer.hpp"

// 新
#include "Renderer/InstancedRenderer.hpp"
```

---

### Q2: 链接错误 "undefined reference to InstancedMesh"

**解决方案**:
1. 确保已更新 `CMakeLists.txt`
2. 删除旧的 `src/Renderer/InstancedRenderer.cpp`
3. 重新运行 CMake: `cmake -B build`

---

### Q3: 如何访问材质颜色？

**旧代码**:
```cpp
glm::vec3 color = mesh.GetMaterialColor();
```

**新代码**:
```cpp
glm::vec3 color = renderer.GetMaterialColor();
```

---

### Q4: 如何动态更新实例？

**旧代码**:
```cpp
mesh.ClearInstances();
mesh.AddInstance(...);
mesh.UpdateInstanceBuffers();
```

**新代码**:
```cpp
InstanceData newInstances;
newInstances.Add(...);
renderer.UpdateInstances(newInstances);
```

---

## 📚 进一步阅读

- **新架构文档**: `docs/NEW_ARCHITECTURE.md`
- **接口文档**: `docs/interfaces/INTERFACES.md`
- **实例化渲染指南**: `docs/INSTANCED_RENDERING_GUIDE.md`

---

## 🎓 总结

新架构通过职责分离实现了：

1. **更清晰的代码组织**
   - 网格数据、实例数据、渲染逻辑完全分离

2. **更高的可维护性**
   - 每个类职责明确，易于理解和修改

3. **更好的复用性**
   - 同一个网格可以用于不同的渲染器
   - 同一个实例数据可以用于不同的网格

4. **保持性能**
   - 仍然使用实例化渲染，性能无损失

欢迎来到新架构！🚀
