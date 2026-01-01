# MeshBuffer 使用指南

## 快速上手

### 替换映射表

| 旧代码（SimpleMesh） | 新代码（MeshBuffer + MeshData） |
|-------------------|--------------------------------|
| `#include "Renderer/SimpleMesh.hpp"` | `#include "Renderer/MeshBuffer.hpp"`<br>`#include "Renderer/MeshDataFactory.hpp"` |
| `SimpleMesh::CreateFromCube()` | `MeshBufferFactory::CreateCubeBuffer()` |
| `SimpleMesh::CreateFromMaterialData(data)` | `MeshBufferFactory::CreateFromMeshData(data)` |
| `mesh.Create()` | `buffer.UploadToGPU(data)` (工厂方法自动调用) |
| `auto mesh = std::make_shared<SimpleMesh>(...)` | `auto buffer = std::make_shared<MeshBuffer>(...)` |

### 基础用法

```cpp
// ========================================
// 场景1：创建立方体实例化渲染
// ========================================

// 旧代码
auto simpleMesh = std::make_shared<SimpleMesh>(
    SimpleMesh::CreateFromCube()
);
simpleMesh->Create();

auto instances = std::make_shared<InstanceData>();
instances->Add(pos, rot, scale, color);

InstancedRenderer renderer;
renderer.SetMesh(simpleMesh);
renderer.SetInstances(instances);
renderer.Initialize();
renderer.Render();

// 新代码
MeshBuffer cubeBuffer = MeshBufferFactory::CreateCubeBuffer();
auto meshBufferPtr = std::make_shared<MeshBuffer>(std::move(cubeBuffer));

auto instances = std::make_shared<InstanceData>();
instances->Add(pos, rot, scale, color);

InstancedRenderer renderer;
renderer.SetMesh(meshBufferPtr);
renderer.SetInstances(instances);
renderer.Initialize();
renderer.Render();
```

### OBJ 模型用法

```cpp
// ========================================
// 场景2：从 OBJ 创建实例化渲染
// ========================================

// 旧代码
auto carInstances = std::make_shared<InstanceData>();
// ... 添加实例 ...

auto [carRenderers, carMeshes, carInstanceData] =
    InstancedRenderer::CreateForOBJ("assets/models/car.obj", carInstances);

for (const auto& carRenderer : carRenderers) {
    shader.SetBool("useTexture", carRenderer.HasTexture());
    shader.SetVec3("objectColor", carRenderer.GetMaterialColor());
    carRenderer.Render();
}
// carMeshes 必须保持存活

// 新代码（几乎相同！）
auto carInstances = std::make_shared<InstanceData>();
// ... 添加实例 ...

auto [carRenderers, carMeshBuffers, carInstanceData] =
    InstancedRenderer::CreateForOBJ("assets/models/car.obj", carInstances);

for (const auto& carRenderer : carRenderers) {
    shader.SetBool("useTexture", carRenderer.HasTexture());
    shader.SetVec3("objectColor", carRenderer.GetMaterialColor());
    carRenderer.Render();
}
// carMeshBuffers 必须保持存活
```

### 高级用法

```cpp
// ========================================
// 场景3：手动创建自定义网格
// ========================================

// 1. 创建数据
MeshData data;
data.SetVertices(myVertices, 8);  // stride = 8
data.SetIndices(myIndices);
data.SetVertexLayout({0, 3, 6}, {3, 3, 2});  // pos, normal, uv
data.SetMaterialColor(glm::vec3(1.0f, 0.5f, 0.3f));

// 2. 上传到 GPU
MeshBuffer buffer;
buffer.UploadToGPU(data);

// 3. 使用
auto meshBufferPtr = std::make_shared<MeshBuffer>(std::move(buffer));
InstancedRenderer renderer;
renderer.SetMesh(meshBufferPtr);
// ...
```

## 主要区别

### 1. 数据和资源分离

**SimpleMesh（旧）**：
```cpp
SimpleMesh mesh;
mesh.SetVertexData(vertices, stride);  // 设置数据
mesh.Create();                        // 创建 GPU 资源
mesh.Draw();                          // 渲染（不需要）
```

**MeshBuffer（新）**：
```cpp
MeshData data;                        // 数据层
data.SetVertices(vertices, stride);

MeshBuffer buffer;                    // 资源层
buffer.UploadToGPU(data);             // 上传到 GPU

// 渲染由 InstancedRenderer 负责
```

### 2. 工厂方法

**SimpleMesh（旧）**：
```cpp
SimpleMesh mesh = SimpleMesh::CreateFromCube();
mesh.Create();
```

**MeshBuffer（新）**：
```cpp
// 方式1：使用工厂（自动上传到 GPU）
MeshBuffer buffer = MeshBufferFactory::CreateCubeBuffer();

// 方式2：手动创建
MeshData data = MeshDataFactory::CreateCubeData();
MeshBuffer buffer;
buffer.UploadToGPU(data);
```

### 3. 类型别名

如果不想改动太多代码，可以添加类型别名：

```cpp
// 在项目代码中添加
using SimpleMesh = MeshBuffer;  // 兼容旧代码

// 或者使用宏
#define SimpleMesh MeshBuffer
```

但**不推荐**长期使用别名，应该逐步迁移到新命名。

## 常见问题

### Q: 为什么没有 Draw() 方法？

**A**: 因为 MeshBuffer 不是可渲染对象，它只是 GPU 资源的包装器。渲染由 InstancedRenderer 负责。

### Q: 为什么要分离 MeshData 和 MeshBuffer？

**A**:
- **MeshData**：纯 CPU 数据，可以序列化、传递、复制
- **MeshBuffer**：GPU 资源，管理 VAO/VBO/EBO

分离后可以：
- 单独测试数据
- 序列化 MeshData 而不需要处理 GPU 资源
- 在不同线程加载数据，主线程上传到 GPU

### Q: 深拷贝是如何工作的？

**A**:
```cpp
MeshBuffer buffer1;
buffer1.UploadToGPU(data);

MeshBuffer buffer2 = buffer1;  // 拷贝 CPU 数据，但不拷贝 GPU 资源
// buffer2 的 VAO/VBO/EBO 都是 0，需要调用 UploadToGPU()

MeshBuffer buffer3 = std::move(buffer1);  // 移动，转移 GPU 资源
// buffer1 的 VAO/VBO/EBO 变为 0，buffer3 继承了它们
```

### Q: 纹理如何处理？

**A**:
```cpp
MeshBuffer buffer = MeshBufferFactory::CreateCubeBuffer();

// 设置纹理
auto texture = std::make_shared<Texture>();
texture->LoadFromFile("texture.png");
buffer.SetTexture(texture);  // 使用 shared_ptr

// InstancedRenderer 会自动获取纹理
InstancedRenderer renderer;
renderer.SetMesh(std::make_shared<MeshBuffer>(std::move(buffer)));
// renderer.HasTexture() 返回 true
```

## 性能考虑

### 内存使用

**SimpleMesh（旧）**：
- 持有 CPU 数据（vertices, indices）
- 持有 GPU 资源（VAO, VBO, EBO）
- 总计：CPU + GPU

**MeshBuffer（新）**：
- 持有 CPU 数据（MeshData）
- 持有 GPU 资源（VAO, VBO, EBO）
- 总计：CPU + GPU（相同）

**性能影响**：无显著差异，只是职责更清晰。

### 拷贝性能

**SimpleMesh（旧）**：
```cpp
SimpleMesh mesh1;
mesh1.Create();
SimpleMesh mesh2 = mesh1;  // 深拷贝 CPU 数据，创建新的 GPU 资源
```

**MeshBuffer（新）**：
```cpp
MeshBuffer buffer1;
buffer1.UploadToGPU(data);
MeshBuffer buffer2 = buffer1;  // 深拷贝 CPU 数据，不创建 GPU 资源
buffer2.UploadToGPU(data);     // 需要手动上传
```

**优势**：可以延迟 GPU 资源创建，提高性能。

---

*更新日期：2026-01-01*
