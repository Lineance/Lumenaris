# OBJModel 重构方案

**日期**：2026-01-02
**状态**：方案设计

## 📋 当前状态分析

### OBJModel 的特殊之处

与基础几何体（Cube/Sphere/Plane/Torus）不同，OBJModel 有以下特殊性：

1. **需要文件加载**：`LoadFromFile(filepath)` 从磁盘读取 OBJ 文件
2. **多材质管理**：一个模型可能有多个材质，需要按材质分组渲染
3. **纹理资源管理**：需要加载和管理多个纹理文件
4. **已有静态方法**：`GetMaterialVertexData(objPath)` 已被 MeshDataFactory 使用

### 当前使用情况

✅ **已经在使用静态方式**：
```cpp
// MeshDataFactory 中已经使用
auto materialDataList = OBJModel::GetMaterialVertexData(objPath);
```

⚠️ **实例方法未被使用**：
```cpp
// main.cpp 中没有 OBJModel 的实例化
// InstancedRenderer 中只使用静态方法
```

## 🎯 重构方案

### 方案 1：完全静态化（推荐 ⭐）

将 OBJModel 完全重构为纯静态工具类，移除所有实例状态和方法。

#### 优点
- ✅ 与其他几何体类保持一致
- ✅ 职责更清晰：只负责数据解析
- ✅ 无状态，线程安全
- ✅ 代码更简洁

#### 缺点
- ❌ 失去传统的使用方式（`OBJModel model; model.Create(); model.Draw();`）
- ❌ 需要调用者自己管理纹理资源

#### 实现步骤

1. **删除所有实例方法**：
   - `OBJModel()` 构造函数
   - `Create()` 方法
   - `Draw()` / `DrawWithMaterial()` 方法
   - `SetPosition()`, `SetRotation()`, `SetScale()`, `SetColor()` 方法
   - `GetModelMatrix()` 方法
   - `LoadFromFile()` 方法（改为静态）

2. **保留静态方法**：
   - `GetMaterialVertexData(objPath)` - 已存在
   - `GetVertexData(objPath)` - 新增（简化版）
   - `GetLayout()` - 新增（返回顶点布局）

3. **纹理管理**：
   - 移除 `m_textures` 成员
   - 在 `MaterialVertexData` 中返回纹理路径
   - 让调用者（MeshBufferFactory 或用户）管理纹理

#### 重构后的接口

```cpp
class OBJModel {
    OBJModel() = delete;  // 禁止实例化

    // 主要接口：获取按材质分离的顶点数据
    static std::vector<MaterialVertexData> GetMaterialVertexData(const std::string& objPath);

    // 简化接口：获取合并的顶点数据（忽略材质）
    static MeshData GetVertexData(const std::string& objPath);

    // 工具方法：获取材质信息
    static std::vector<OBJMaterial> GetMaterials(const std::string& objPath);
    static bool HasMaterials(const std::string& objPath);
};
```

#### 使用方式

```cpp
// ✅ 推荐：通过 MeshDataFactory
auto objBuffers = MeshDataFactory::CreateOBJBuffers("models/bunny.obj");

// ✅ 或直接使用静态方法
auto materialDataList = OBJModel::GetMaterialVertexData("models/bunny.obj");
for (const auto& materialData : materialDataList) {
    // 创建 MeshBuffer
    // 加载纹理：materialData.texturePath
    // 渲染
}
```

---

### 方案 2：保留实例类（保守方案）

保留 OBJModel 为实例类，但移除不必要的实例状态。

#### 优点
- ✅ 保留传统使用方式
- ✅ 向后兼容
- ✅ 适合需要直接操作 OBJModel 的场景

#### 缺点
- ❌ 与其他几何体类不一致
- ❌ 混合职责（数据解析 + 渲染）
- ❌ 维护成本更高

#### 实现步骤

1. **移除实例状态**：
   - 删除 `m_position`, `m_rotation`, `m_scale`, `m_color`
   - 删除 `SetPosition()`, `SetRotation()`, `SetScale()`, `SetColor()`

2. **保留核心功能**：
   - 保留 `LoadFromFile()`, `Create()`, `Draw()`, `DrawWithMaterial()`
   - 保留纹理管理功能

3. **强调静态方法**：
   - 文档中推荐使用 `GetMaterialVertexData()`
   - 实例方法标记为"用于兼容"

#### 重构后的接口

```cpp
class OBJModel {
public:
    // 构造函数
    OBJModel();
    explicit OBJModel(const std::string& filepath);

    // 核心功能（保留）
    bool LoadFromFile(const std::string& filepath);
    void Create();
    void Draw() const;
    void DrawWithMaterial(int materialIndex) const;

    // ❌ 删除：实例状态管理
    // void SetPosition(const glm::vec3& pos);
    // void SetRotation(const glm::vec3& rotation);
    // void SetScale(float scale);
    // void SetColor(const glm::vec3& color);
    // glm::mat4 GetModelMatrix() const;

    // ✅ 静态方法（推荐使用）
    static std::vector<MaterialVertexData> GetMaterialVertexData(const std::string& objPath);
};
```

#### 使用方式

```cpp
// ⚠️ 传统方式（保留用于兼容）
OBJModel bunny;
bunny.LoadFromFile("models/bunny.obj");
bunny.Create();
bunny.Draw();  // 或 bunny.DrawWithMaterial(0);

// ✅ 推荐方式：使用静态方法
auto materialDataList = OBJModel::GetMaterialVertexData("models/bunny.obj");
// 通过 MeshDataFactory 处理
```

---

### 方案 3：分离关注点（最佳实践 ⭐⭐）

将 OBJModel 拆分为两个独立的类：

1. **OBJLoader** - 纯静态工具类，负责文件解析
2. **OBJMesh** - 可选的渲染器类，提供传统渲染方式

#### 优点
- ✅ 职责最清晰
- ✅ 最大灵活性
- ✅ 向后兼容（通过 OBJMesh）
- ✅ 易于测试和扩展

#### 缺点
- ❌ 需要创建新类
- ❌ 更多的文件

#### 实现步骤

1. **重构 OBJLoader**（已存在，增强功能）：
   ```cpp
   class OBJLoader {
   public:
       static std::vector<MaterialVertexData> LoadFromFile(const std::string& objPath);
       static std::vector<OBJMaterial> GetMaterials(const std::string& objPath);
       static bool ValidateFile(const std::string& objPath);
   };
   ```

2. **创建 OBJMesh**（新的渲染器类）：
   ```cpp
   class OBJMesh {
   public:
       OBJMesh(const std::string& objPath);
       void Create();
       void Draw() const;
       void DrawWithMaterial(int materialIndex) const;
   };
   ```

3. **废弃 OBJModel**（标记为 deprecated）：
   ```cpp
   class OBJModel {
       // [[deprecated("Use OBJLoader or OBJMesh instead")]]
   };
   ```

---

## 📊 方案对比

| 特性 | 方案 1：完全静态化 | 方案 2：保留实例类 | 方案 3：分离关注点 |
|------|-------------------|-------------------|------------------|
| **一致性** | ⭐⭐⭐ 与其他几何体一致 | ⭐ 不一致 | ⭐⭐ 通过 OBJMesh 一致 |
| **职责清晰** | ⭐⭐⭐ 纯数据解析 | ⭐⭐ 混合职责 | ⭐⭐⭐ 解析与渲染分离 |
| **向后兼容** | ❌ 不兼容 | ⭐⭐⭐ 完全兼容 | ⭐⭐ 通过 OBJMesh 兼容 |
| **代码复杂度** | ⭐ 简单 | ⭐⭐ 中等 | ⭐⭐⭐ 较复杂 |
| **维护成本** | ⭐⭐⭐ 低 | ⭐⭐ 中 | ⭐ 中 |
| **灵活性** | ⭐⭐ 高 | ⭐ 中 | ⭐⭐⭐ 最高 |

## 🎯 推荐方案

### 短期（当前迭代）：**方案 1 - 完全静态化**

**理由**：
1. ✅ 与已重构的 Cube/Sphere/Plane/Torus 保持一致
2. ✅ `main.cpp` 和 `InstancedRenderer` 已经只使用静态方法
3. ✅ 代码更简洁，维护成本更低
4. ✅ 项目的实例化渲染架构不需要实例类的 OBJModel

### 长期（未来扩展）：**方案 3 - 分离关注点**

**理由**：
1. ✅ 最大灵活性
2. ✅ 可以同时满足静态使用和实例使用的需求
3. ✅ 符合单一职责原则

## 📝 实施计划（方案 1）

### 阶段 1：清理实例状态

删除以下成员和方法：
- ❌ `m_position`, `m_rotation`, `m_scale`, `m_color`
- ❌ `SetPosition()`, `SetRotation()`, `SetScale()`, `SetColor()`
- ❌ `GetModelMatrix()`
- ❌ `GetVAO()`, `GetVertexCount()`, `GetIndexCount()`, `HasIndices()`, `HasTexture()`

### 阶段 2：增强静态方法

保留和改进：
- ✅ `GetMaterialVertexData(objPath)` - 已存在
- ✅ 添加 `GetVertexData(objPath)` - 简化版，返回合并的数据
- ✅ 添加 `GetMaterials(objPath)` - 获取材质信息

### 阶段 3：文档更新

- ✅ 更新 GEOMETRY_GUIDE.md
- ✅ 更新 ARCHITECTURE.md
- ✅ 添加迁移指南

## 🔄 迁移示例

### 旧代码（不再支持）

```cpp
// ❌ 旧方式
OBJModel bunny;
bunny.LoadFromFile("models/bunny.obj");
bunny.Create();
bunny.SetPosition(glm::vec3(1.0f, 0.0f, 0.0f));
bunny.Draw();
```

### 新代码（推荐）

```cpp
// ✅ 新方式：通过 MeshDataFactory
auto objBuffers = MeshDataFactory::CreateOBJBuffers("models/bunny.obj");

// 创建实例数据
auto instances = std::make_shared<InstanceData>();
instances->AddInstance({glm::vec3(1.0f, 0.0f, 0.0f), ...});

// 为每个材质创建渲染器
for (auto& buffer : objBuffers) {
    InstancedRenderer renderer(std::move(buffer), instances);
    renderer.Initialize();
    renderer.Render();
}
```

## ❓ 常见问题

### Q: 为什么不保留 OBJModel 的实例方式？

A:
1. **项目已转向实例化渲染**：所有新代码都使用 InstancedRenderer
2. **静态方法已足够**：`GetMaterialVertexData()` 已满足所有需求
3. **维护成本**：保留两种使用方式会增加维护负担

### Q: 如果我需要直接渲染单个 OBJ 怎么办？

A: 使用 MeshDataFactory：

```cpp
auto objBuffers = MeshDataFactory::CreateOBJBuffers("model.obj");
// 使用第一个材质的缓冲区
InstancedRenderer renderer(std::move(objBuffers[0]), singleInstance);
renderer.Render();
```

### Q: 纹理如何管理？

A:
```cpp
auto materialDataList = OBJModel::GetMaterialVertexData("model.obj");
for (const auto& materialData : materialDataList) {
    if (!materialData.texturePath.empty()) {
        Texture texture;
        texture.LoadFromFile(materialData.texturePath);
        // 使用纹理
    }
}
```

## 🎉 总结

推荐采用**方案 1（完全静态化）**，理由：
1. 与项目架构一致（实例化渲染）
2. 代码简洁，维护成本低
3. 功能完整（静态方法已满足需求）
4. 迁移成本低（main.cpp 未使用实例方式）
