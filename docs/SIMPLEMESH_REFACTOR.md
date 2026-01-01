# SimpleMesh 重构总结

## 📋 重构概述

**目标**：废弃 SimpleMesh，建立统一的三层架构设计

**原则**：与 InstanceData/InstancedRenderer 的设计保持完全一致

**日期**：2026-01-01

---

## 🎯 重构动机

### SimpleMesh 的设计问题

1. **职责过重**：同时承担数据存储、GPU 资源管理、渲染能力、工厂方法
2. **命名混乱**："Simple" 不能体现真实角色，且代码量最大（342 行）
3. **接口继承问题**：继承 IMesh 但不需要多态能力
4. **与架构不一致**：与 InstanceData/InstancedRenderer 的设计模式不统一

### 与 InstanceData 的对比

| 维度 | InstanceData | SimpleMesh（旧） | 问题 |
|------|-------------|----------------|------|
| **角色** | 纯数据容器 | 数据+GPU资源+渲染 | 职责不清 |
| **CPU 数据** | ✅ vector<mat4> | ✅ vector<float> | 一致 |
| **GPU 资源** | ❌ 无 | ✅ VAO/VBO/EBO | 不一致 |
| **渲染能力** | ❌ 无 | ❌ Draw()（不需要） | 冗余 |
| **继承接口** | ❌ 无 | ✅ IMesh（不需要） | 不必要 |

---

## ✨ 新的设计

### 三层架构

```
┌─────────────────────────────────────────────────────────────┐
│                    统一的三层架构                              │
└─────────────────────────────────────────────────────────────┘

第1层：纯数据容器（CPU 内存）
┌──────────────────────┐         ┌──────────────────────┐
│   InstanceData       │         │   MeshData           │
│   (实例数据)          │         │   (网格数据)           │
├──────────────────────┤         ├──────────────────────┤
│ • vector<mat4>       │         │ • vector<float>      │
│ • vector<vec3>       │         │ • vector<uint>       │
│ • 无 OpenGL 对象     │         │ • 无 OpenGL 对象     │
│ • Add/Clear/Get...   │         │ • Set.../Get...      │
└──────────────────────┘         └──────────────────────┘
            │                               │
            │ shared_ptr                    │ 数据传递
            ▼                               ▼

第2层：GPU 资源包装
┌──────────────────────┐         ┌──────────────────────┐
│ InstancedRenderer    │         │   MeshBuffer         │
│ (实例化渲染器)        │         │   (网格缓冲区)         │
├──────────────────────┤         ├──────────────────────┤
│ • InstanceData*      │────────▶│ • MeshData          │
│ • MeshBuffer*        │────────▶│ • VAO/VBO/EBO        │
│ • instanceVBO        │         │ • UploadToGPU()      │
│ • Initialize()       │         │ • GetVAO()           │
│ • Render()           │         │ • GetVertexCount()   │
└──────────────────────┘         └──────────────────────┘
            │                               │
            │                               │
            └──────────────┬────────────────┘
                           ▼
                      glDrawElementsInstanced()
```

---

## 📦 新增的类

### 1. MeshData（纯数据容器）

**文件**：
- `include/Renderer/MeshData.hpp`
- `src/Renderer/MeshData.cpp`

**职责**：
- ✅ 存储顶点数据（vertices）
- ✅ 存储索引数据（indices）
- ✅ 存储顶点属性布局（offsets, sizes）
- ✅ 存储材质颜色
- ✅ 纯数据，无 GPU 资源
- ✅ 可序列化，可复制

**接口**：
```cpp
class MeshData {
    // 数据设置
    void SetVertices(const std::vector<float>& vertices, size_t stride);
    void SetIndices(const std::vector<unsigned int>& indices);
    void SetVertexLayout(const std::vector<size_t>& offsets, const std::vector<int>& sizes);
    void SetMaterialColor(const glm::vec3& color);

    // 数据访问
    const std::vector<float>& GetVertices() const;
    const std::vector<unsigned int>& GetIndices() const;
    size_t GetVertexCount() const;
    size_t GetIndexCount() const;
    bool HasIndices() const;
    const glm::vec3& GetMaterialColor() const;

    // ❌ 无 Create()，无 Draw()，无 VAO/VBO/EBO
};
```

**使用场景**：
```cpp
MeshData data;
data.SetVertices(vertices, 8);  // stride = 8 (pos:3 + normal:3 + uv:2)
data.SetIndices(indices);
data.SetVertexLayout({0, 3, 6}, {3, 3, 2});  // pos, normal, uv
data.SetMaterialColor(glm::vec3(1.0f, 0.5f, 0.3f));
```

---

### 2. MeshBuffer（GPU 资源包装器）

**文件**：
- `include/Renderer/MeshBuffer.hpp`
- `src/Renderer/MeshBuffer.cpp`

**职责**：
- ✅ 持有 CPU 数据副本（MeshData）
- ✅ 管理 GPU 资源（VAO/VBO/EBO）
- ✅ 上传数据到 GPU（UploadToGPU）
- ✅ 提供 VAO 访问（GetVAO）
- ✅ 支持纹理（shared_ptr<Texture>）
- ✅ 支持深拷贝（拷贝构造/赋值）
- ✅ 支持移动语义（移动构造/赋值）
- ✅ ❌ 不继承任何接口
- ✅ ❌ 没有 Draw() 方法

**接口**：
```cpp
class MeshBuffer {
    // GPU 操作
    void UploadToGPU(const MeshData& data);
    void ReleaseGPU();

    // 访问接口
    unsigned int GetVAO() const;
    size_t GetVertexCount() const;
    size_t GetIndexCount() const;
    bool HasIndices() const;
    const glm::vec3& GetMaterialColor() const;

    // 纹理管理
    void SetTexture(std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> GetTexture() const;
    bool HasTexture() const;

    // 数据访问
    const MeshData& GetData() const;

    // ❌ 无 Draw() 方法
    // ❌ 不继承 IMesh
};
```

**使用场景**：
```cpp
MeshBuffer buffer;
buffer.UploadToGPU(data);  // 上传到 GPU
unsigned int vao = buffer.GetVAO();  // 获取 VAO（用于 InstancedRenderer）
```

---

### 3. MeshDataFactory & MeshBufferFactory（工厂类）

**文件**：
- `include/Renderer/MeshDataFactory.hpp`
- `src/Renderer/MeshDataFactory.cpp`

**MeshDataFactory（生成数据）**：
```cpp
class MeshDataFactory {
    // 基础几何体
    static MeshData CreateCubeData();
    static MeshData CreateSphereData(int stacks, int slices, float radius);

    // OBJ 模型
    static std::vector<MeshData> CreateOBJData(const std::string& objPath);
};
```

**MeshBufferFactory（生成 GPU 资源）**：
```cpp
class MeshBufferFactory {
    // 基础几何体（自动上传到 GPU）
    static MeshBuffer CreateCubeBuffer();
    static MeshBuffer CreateSphereBuffer(int stacks, int slices, float radius);

    // OBJ 模型（自动上传到 GPU）
    static std::vector<MeshBuffer> CreateOBJBuffers(const std::string& objPath);

    // 从 MeshData 创建
    static MeshBuffer CreateFromMeshData(const MeshData& data);
    static std::vector<MeshBuffer> CreateFromMeshDataList(const std::vector<MeshData>& dataList);
};
```

---

## 🔄 InstancedRenderer 更新

### 主要变更

| 旧代码 | 新代码 | 说明 |
|--------|--------|------|
| `std::shared_ptr<SimpleMesh> m_mesh` | `std::shared_ptr<MeshBuffer> m_meshBuffer` | 变量重命名 |
| `SetMesh(std::shared_ptr<SimpleMesh>)` | `SetMesh(std::shared_ptr<MeshBuffer>)` | 参数类型更新 |
| `m_mesh->GetVAO()` | `m_meshBuffer->GetVAO()` | 调用更新 |
| `CreateForOBJ()` 返回 `vector<shared_ptr<SimpleMesh>>` | 返回 `vector<shared_ptr<MeshBuffer>>` | 返回类型更新 |

### 使用示例对比

**旧代码（使用 SimpleMesh）**：
```cpp
// 1. 创建 SimpleMesh
auto simpleMesh = std::make_shared<SimpleMesh>(
    SimpleMesh::CreateFromCube()
);
simpleMesh->Create();

// 2. 创建实例数据
auto instances = std::make_shared<InstanceData>();
instances->Add(pos, rot, scale, color);

// 3. 创建渲染器
InstancedRenderer renderer;
renderer.SetMesh(simpleMesh);
renderer.SetInstances(instances);
renderer.Initialize();
renderer.Render();
```

**新代码（使用 MeshBuffer）**：
```cpp
// 1. 创建 MeshBuffer（已上传到 GPU）
MeshBuffer cubeBuffer = MeshBufferFactory::CreateCubeBuffer();
auto meshBufferPtr = std::make_shared<MeshBuffer>(std::move(cubeBuffer));

// 2. 创建实例数据
auto instances = std::make_shared<InstanceData>();
instances->Add(pos, rot, scale, color);

// 3. 创建渲染器
InstancedRenderer renderer;
renderer.SetMesh(meshBufferPtr);
renderer.SetInstances(instances);
renderer.Initialize();
renderer.Render();
```

---

## 📊 架构统一性

### 设计模式对比

| 层级 | InstanceData/InstancedRenderer | MeshData/MeshBuffer/InstancedRenderer |
|------|-------------------------------|---------------------------------------|
| **数据层** | `InstanceData`（纯数据） | `MeshData`（纯数据） |
| **资源层** | `InstancedRenderer::instanceVBO` | `MeshBuffer`（VAO/VBO/EBO） |
| **渲染层** | `InstancedRenderer`（渲染逻辑） | `InstancedRenderer`（渲染逻辑） |

### 统一性验证

| 维度 | InstanceData | MeshData | ✅/❌ |
|------|-------------|----------|-----|
| 纯数据容器 | ✅ | ✅ | ✅ 一致 |
| 无 GPU 资源 | ✅ | ✅ | ✅ 一致 |
| 无渲染能力 | ✅ | ✅ | ✅ 一致 |
| 不继承接口 | ✅ | ✅ | ✅ 一致 |
| 数据访问方法 | GetModelMatrices() | GetVertices() | ✅ 一致 |

| 维度 | InstancedRenderer | InstancedRenderer | ✅/❌ |
|------|-------------------|-------------------|-----|
| 继承 IRenderer | ✅ | ✅ | ✅ 一致 |
| 持有 shared_ptr<数据> | ✅ (InstanceData) | ✅ (MeshBuffer) | ✅ 一致 |
| 管理 instanceVBO | ✅ | ✅ | ✅ 一致 |
| Initialize/Render | ✅ | ✅ | ✅ 一致 |

---

## 📝 文件变更清单

### 新增文件

1. `include/Renderer/MeshData.hpp` - 网格数据容器
2. `include/Renderer/MeshBuffer.hpp` - 网格缓冲区
3. `include/Renderer/MeshDataFactory.hpp` - 工厂类
4. `src/Renderer/MeshData.cpp` - 数据容器实现
5. `src/Renderer/MeshBuffer.cpp` - 缓冲区实现
6. `src/Renderer/MeshDataFactory.cpp` - 工厂实现

### 修改文件

1. `include/Renderer/InstancedRenderer.hpp` - 更新为使用 MeshBuffer
2. `src/Renderer/InstancedRenderer.cpp` - 更新为使用 MeshBuffer
3. `CMakeLists.txt` - 添加新的源文件

### 待删除文件

1. `include/Renderer/SimpleMesh.hpp` - ❌ 废弃
2. `src/Renderer/SimpleMesh.cpp` - ❌ 废弃

---

## 🎯 重构收益

### 1. 架构统一性

**重构前**：
```
InstanceData: 纯数据 ✅
SimpleMesh: 数据+GPU+渲染 ❌
InstancedRenderer: 渲染逻辑 ✅
```

**重构后**：
```
InstanceData: 纯数据 ✅
MeshData: 纯数据 ✅
MeshBuffer: GPU 资源 ✅
InstancedRenderer: 渲染逻辑 ✅
```

### 2. 单一职责原则

| 类 | 单一职责 | 说明 |
|---|---------|------|
| **MeshData** | ✅ 数据存储 | 只存储顶点和索引数据 |
| **MeshBuffer** | ✅ GPU 资源管理 | 只管理 VAO/VBO/EBO |
| **InstancedRenderer** | ✅ 渲染逻辑 | 只负责渲染 |

### 3. 易于测试

```cpp
// 可以单独测试数据
TEST(MeshData, SetVertices) {
    MeshData data;
    data.SetVertices({...}, 8);
    EXPECT_EQ(data.GetVertexCount(), 100);
}

// 可以单独测试 GPU 资源管理
TEST(MeshBuffer, UploadToGPU) {
    MeshData data = CreateTestData();
    MeshBuffer buffer;
    buffer.UploadToGPU(data);
    EXPECT_NE(buffer.GetVAO(), 0);
}

// 可以 Mock 数据测试渲染器
TEST(InstancedRenderer, Render) {
    auto mockBuffer = std::make_shared<MeshBuffer>();
    InstancedRenderer renderer;
    renderer.SetMesh(mockBuffer);
    // 测试渲染逻辑
}
```

### 4. 易于扩展

```cpp
// 添加新的渲染后端
class VulkanMeshBuffer {
    MeshData m_data;
    VkBuffer m_vertexBuffer;  // Vulkan 对象
    void UploadToGPU(const MeshData& data);
    unsigned int GetVAO() const;  // 适配层
};

// MeshData 完全不需要改变！
```

### 5. 易于序列化

```cpp
// 纯数据容器可以直接序列化
void SaveMesh(const MeshData& data, const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    size_t vertexCount = data.GetVertexCount();
    size_t indexCount = data.GetIndexCount();
    file.write(reinterpret_cast<const char*>(&vertexCount), sizeof(size_t));
    file.write(reinterpret_cast<const char*>(&indexCount), sizeof(size_t));
    file.write(reinterpret_cast<const char*>(data.GetVertices().data()),
              data.GetVertices().size() * sizeof(float));
    file.write(reinterpret_cast<const char*>(data.GetIndices().data()),
              data.GetIndices().size() * sizeof(unsigned int));
}

MeshData LoadMesh(const std::string& path) {
    // 直接加载，不需要创建 OpenGL 对象
}
```

---

## 🚀 下一步行动

### 立即可执行

1. ✅ 更新主程序使用新类
2. ❌ 删除 SimpleMesh 文件
3. ✅ 编译测试

### 后续优化

1. 添加 MeshData 序列化/反序列化
2. 添加 MeshData 单元测试
3. 添加 MeshBuffer 单元测试
4. 更新文档（ARCHITECTURE.md, INTERFACES.md）

---

## 📚 参考资料

- **架构文档**：`docs/ARCHITECTURE.md`
- **接口文档**：`docs/interfaces/INTERFACES.md`
- **设计原则**：SOLID 原则，单一职责原则

---

*重构完成日期：2026-01-01*
*重构负责人：Claude Code*
