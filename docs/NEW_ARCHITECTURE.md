# 新架构设计文档 - 方案C：职责分离

## 📋 概述

本文档说明了 OpenGL 项目的**新架构设计方案C**，该方案通过职责完全分离实现了更清晰的代码组织。

## 🎯 设计原则

### 单一职责原则 (SRP)

每个类只负责一个明确的职责：

- **IMesh**: 网格数据接口
- **InstanceData**: 实例数据容器
- **InstancedRenderer**: 渲染逻辑执行者
- **IRenderer**: 渲染器抽象接口

## 🏗️ 架构组成

### 1. IMesh 接口 (网格数据接口)

```cpp
class IMesh {
    virtual void Create() = 0;
    virtual void Draw() const = 0;
    virtual unsigned int GetVAO() const = 0;
    virtual size_t GetVertexCount() const = 0;
    virtual size_t GetIndexCount() const = 0;
    virtual bool HasIndices() const = 0;
    virtual bool HasTexture() const { return false; }
};
```

**职责**：
- 定义网格的基本接口
- 提供顶点和索引数据访问
- 提供 VAO 访问（用于实例化渲染）

**实现类**：
- `Cube`: 立方体网格
- `Sphere`: 球体网格
- `OBJModel`: OBJ 模型网格
- `SimpleMesh`: 简单网格（纯数据容器）

---

### 2. InstanceData 类 (实例数据容器)

```cpp
class InstanceData {
public:
    void Add(const glm::vec3& position, const glm::vec3& rotation,
             const glm::vec3& scale, const glm::vec3& color);

    void AddBatch(const std::vector<glm::mat4>& matrices,
                  const std::vector<glm::vec3>& colors);

    void Clear();

    size_t GetCount() const;
    const std::vector<glm::mat4>& GetModelMatrices() const;
    const std::vector<glm::vec3>& GetColors() const;

private:
    std::vector<glm::mat4> m_modelMatrices;
    std::vector<glm::vec3> m_colors;
};
```

**职责**：
- 管理实例的模型矩阵（位置、旋转、缩放）
- 管理实例的颜色属性
- 提供批量添加和清除实例的接口

**设计说明**：
- 纯数据容器，不涉及渲染逻辑
- 可以独立于渲染器进行操作
- 支持实例的动态增删

**文件位置**：
- 头文件：`include/Renderer/InstanceData.hpp`
- 源文件：`src/Renderer/InstanceData.cpp`

---

### 3. IRenderer 接口 (渲染器抽象)

```cpp
class IRenderer {
    virtual void Initialize() = 0;
    virtual void Render() const = 0;
    virtual std::string GetName() const = 0;
};
```

**职责**：
- 定义渲染器的统一接口
- 支持初始化和渲染操作
- 与 IMesh 接口分离，强调"渲染器"的概念

**设计说明**：
- **IMesh**：表示可渲染的几何体
- **IRenderer**：表示渲染逻辑的执行者

---

### 4. InstancedRenderer 类 (实例化渲染器)

```cpp
class InstancedRenderer : public IRenderer {
public:
    // IRenderer 接口实现
    void Initialize() override;
    void Render() const override;
    std::string GetName() const override;

    // 设置网格和实例
    void SetMesh(const IMesh& mesh);
    void SetInstances(const InstanceData& data);
    void UpdateInstances(const InstanceData& data);

    // 材质和纹理
    void SetMaterialColor(const glm::vec3& color);
    void SetTexture(Texture* texture);

    // 静态辅助方法
    static InstancedRenderer CreateForCube(const InstanceData& instances);
    static std::vector<InstancedRenderer> CreateForOBJ(
        const std::string& objPath,
        const InstanceData& instances);

private:
    const IMesh* m_mesh;
    InstanceData m_instances;
    GLuint m_instanceVBO;
    Texture* m_texture;
    glm::vec3 m_materialColor;
};
```

**职责**：
- 接收 IMesh 引用（网格模板）
- 接收 InstanceData（实例数据）
- 管理 OpenGL 实例化缓冲区（instanceVBO）
- 执行实例化渲染

**文件位置**：
- 头文件：`include/Renderer/InstancedRenderer.hpp`
- 源文件：`src/Renderer/InstancedRenderer.cpp`

---

### 5. SimpleMesh 类 (简单网格)

```cpp
class SimpleMesh : public IMesh {
public:
    void SetVertexData(const std::vector<float>& vertices, size_t stride);
    void SetVertexLayout(const std::vector<size_t>& offsets,
                        const std::vector<int>& sizes);
    void SetIndexData(const std::vector<unsigned int>& indices);
    void SetTexture(Texture* texture);

    // IMesh 接口实现
    void Create() override;
    void Draw() const override;

    // 静态辅助方法
    static SimpleMesh CreateFromCube();
    static SimpleMesh CreateFromMaterialData(
        const OBJModel::MaterialVertexData& materialData);

private:
    unsigned int m_vao, m_vbo, m_ebo;
    std::vector<float> m_vertices;
    std::vector<unsigned int> m_indices;
    Texture* m_texture;
    glm::vec3 m_materialColor;
};
```

**职责**：
- 存储顶点数据（位置、法线、UV）
- 存储 VAO/VBO/EBO
- 提供基本的 Create() 和 Draw() 方法

**设计说明**：
- 纯粹的数据容器，用于实例化渲染
- 与 InstancedRenderer 配合使用

**文件位置**：
- 头文件：`include/Renderer/SimpleMesh.hpp`
- 源文件：`src/Renderer/SimpleMesh.cpp`

---

## 📝 使用示例

### 基础用法

```cpp
// 1. 创建网格
SimpleMesh cubeMesh = SimpleMesh::CreateFromCube();
cubeMesh.Create();

// 2. 准备实例数据
InstanceData instances;
for (int i = 0; i < 100; ++i) {
    instances.Add(
        glm::vec3(i * 2.0f, 0.0f, 0.0f),  // 位置
        glm::vec3(0.0f, 0.0f, 0.0f),     // 旋转
        glm::vec3(1.0f),                 // 缩放
        glm::vec3(1.0f, 0.0f, 0.0f)      // 颜色
    );
}

// 3. 创建渲染器
InstancedRenderer renderer;
renderer.SetMesh(cubeMesh);
renderer.SetInstances(instances);
renderer.Initialize();

// 4. 渲染
shader.Use();
shader.SetMat4("projection", projection);
shader.SetMat4("view", view);
renderer.Render();  // 一次调用渲染100个实例
```

### OBJ 模型实例化

```cpp
// 1. 准备实例数据
InstanceData carInstances;
for (int i = 0; i < 50; ++i) {
    carInstances.Add(position, rotation, scale, color);
}

// 2. 创建渲染器（每个材质一个）
std::vector<InstancedRenderer> carRenderers =
    InstancedRenderer::CreateForOBJ("assets/models/car.obj", carInstances);

// 3. 渲染所有材质
for (auto& renderer : carRenderers) {
    renderer.Render();
}
```

---

## ✅ 优点

### 1. 职责清晰分离
- ✅ **IMesh**: 负责网格数据
- ✅ **InstanceData**: 负责实例数据
- ✅ **InstancedRenderer**: 负责渲染逻辑
- ✅ 符合单一职责原则

### 2. 高度复用
- ✅ 同一个网格可以渲染不同的实例集合
- ✅ 网格和实例数据可以独立管理
- ✅ 支持动态更新实例数据

### 3. 易于扩展
- ✅ 可以轻松添加新的网格类型
- ✅ 可以添加新的渲染器类型
- ✅ 接口清晰，便于理解和维护

### 4. 性能优化
- ✅ 实例化渲染减少绘制调用
- ✅ 数据和渲染逻辑分离，便于优化
- ✅ 支持动态更新实例数据

---

## 🔄 与旧架构对比

### 旧架构（InstancedMesh）

```cpp
class InstancedMesh : public IMesh {
    // 问题：
    // 1. 既包含网格数据，又包含实例数据
    // 2. 职责不清晰
    // 3. 继承 IMesh，但不是单个网格

    void AddInstance(...);  // 混合了数据管理
    void Create();          // 和渲染初始化
    void Draw() const;
};
```

### 新架构（方案C）

```cpp
// 网格数据
SimpleMesh mesh;
mesh.Create();

// 实例数据
InstanceData instances;
instances.Add(...);

// 渲染逻辑
InstancedRenderer renderer;
renderer.SetMesh(mesh);
renderer.SetInstances(instances);
renderer.Initialize();
renderer.Render();
```

**改进**：
- ✅ 职责完全分离
- ✅ 概念更清晰
- ✅ 更易于理解和使用

---

## 📂 文件组织

```
include/Renderer/
├── Mesh.hpp                  # IMesh 接口
├── IRenderer.hpp            # IRenderer 接口（新）
├── InstanceData.hpp         # 实例数据容器（新）
├── SimpleMesh.hpp           # 简单网格（新）
├── InstancedRenderer.hpp   # 新架构实例化渲染器（新）
├── Cube.hpp                 # 立方体（扩展接口）
├── Sphere.hpp               # 球体（扩展接口）
└── OBJModel.hpp             # OBJ 模型（扩展接口）

src/Renderer/
├── InstanceData.cpp         # 实例数据实现（新）
├── SimpleMesh.cpp           # 简单网格实现（新）
└── InstancedRenderer.cpp   # 实例化渲染器实现（新）

examples/
└── new_instanced_rendering.cpp  # 新架构示例（新）
```

---

## 🚀 未来扩展

### 1. 添加新的网格类型

```cpp
class CylinderMesh : public IMesh {
    // 实现 IMesh 接口
    // 可以直接用于 InstancedRenderer
};
```

### 2. 添加新的渲染器类型

```cpp
class FrustumCullingRenderer : public IRenderer {
    // 实现视锥体剔除的实例化渲染
    // 同样使用 IMesh 和 InstanceData
};
```

### 3. 添加 LOD (Level of Detail) 支持

```cpp
class LODInstancedRenderer {
    std::vector<SimpleMesh> m_lodMeshes;
    InstancedRenderer m_renderer;

    void Render(const glm::vec3& cameraPos) {
        int lod = CalculateLOD(cameraPos);
        m_renderer.SetMesh(m_lodMeshes[lod]);
        m_renderer.Render();
    }
};
```

---

## 📊 性能对比

| 方案 | 绘制调用 | CPU-GPU通信 | 代码复杂度 | 可维护性 |
|------|---------|-------------|-----------|---------|
| 传统方式 | N 次 | N 次 | 低 | 低 |
| 旧架构（InstancedMesh） | 1 次 | 1 次 | 中 | 中 |
| 新架构（方案C） | 1 次 | 1 次 | 低 | 高 |

---

## 🎓 总结

**方案C（职责分离）**通过以下改进实现了更好的架构：

1. **清晰的职责划分**：每个类只负责一件事
2. **高度复用性**：网格和实例数据可以灵活组合
3. **易于扩展**：接口清晰，便于添加新功能
4. **性能优化**：保持实例化渲染的性能优势

这是一个符合 SOLID 原则的现代化 C++ 设计，特别适合中大型项目的长期维护和扩展。
