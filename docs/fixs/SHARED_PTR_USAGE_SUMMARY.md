# shared_ptr 使用情况分析

## 📋 检查结果

### ✅ SimpleMesh.cpp - **已使用 shared_ptr**

**位置**：`include/Renderer/SimpleMesh.hpp` 和 `src/Renderer/SimpleMesh.cpp`

**纹理成员变量**：
```cpp
// include/Renderer/SimpleMesh.hpp:92
class SimpleMesh {
private:
    std::shared_ptr<Texture> m_texture;  // ✅ 使用 shared_ptr

public:
    void SetTexture(std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> GetTexture() const { return m_texture; }
};
```

**构造/析构函数**：
```cpp
// src/Renderer/SimpleMesh.cpp:10-30
SimpleMesh::~SimpleMesh()
{
    // 清理 OpenGL 资源
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);

    // ✅ 注意：纹理由 shared_ptr 自动管理，无需手动 delete
}
```

**拷贝构造函数**：
```cpp
// src/Renderer/SimpleMesh.cpp:33-52
SimpleMesh::SimpleMesh(const SimpleMesh& other)
    : m_texture(other.m_texture),  // ✅ shared_ptr 拷贝（引用计数+1）
      // ...
{
    if (other.m_vao != 0) {
        Create();  // 创建新的 OpenGL 对象
    }
}
```

**移动构造函数**：
```cpp
// src/Renderer/SimpleMesh.cpp:97-118
SimpleMesh::SimpleMesh(SimpleMesh&& other) noexcept
    : m_texture(std::move(other.m_texture)),  // ✅ shared_ptr 移动（零拷贝）
      // ...
{
    // 置零源对象的 OpenGL 句柄
    other.m_vao = 0;
    other.m_vbo = 0;
    other.m_ebo = 0;
}
```

**静态工厂方法**：
```cpp
// src/Renderer/SimpleMesh.cpp:254-268
SimpleMesh SimpleMesh::CreateFromMaterialData(
    const OBJModel::MaterialVertexData& materialData)
{
    SimpleMesh mesh;

    // ✅ 使用 shared_ptr 自动管理纹理生命周期
    if (!materialData.texturePath.empty())
    {
        auto texture = std::make_shared<Texture>();
        if (texture->LoadFromFile(materialData.texturePath))
        {
            mesh.SetTexture(texture);  // ✅ shared_ptr 自动管理
        }
        // 失败时 texture 自动销毁（引用计数=0）
    }

    return mesh;  // ✅ 移动返回
}
```

---

### ❌ InstanceData.cpp - **不需要使用 shared_ptr**

**原因分析**：

`InstanceData` 是一个**纯数据容器**，只负责存储：
- `std::vector<glm::mat4> m_modelMatrices` - 模型矩阵
- `std::vector<glm::vec3> m_colors` - 实例颜色

**为什么不使用 shared_ptr**？

1. **值类型语义**
   ```cpp
   // InstanceData 存储的是值类型，不是指针
   std::vector<glm::mat4> m_modelMatrices;  // 值
   std::vector<glm::vec3> m_colors;         // 值
   ```

2. **不需要共享所有权**
   - `InstanceData` 本身就拥有这些数据
   - 不需要多个对象共享同一个 `InstanceData`

3. **使用 shared_ptr 管理 InstanceData**
   ```cpp
   // ✅ 正确用法：在 InstancedRenderer 中使用 shared_ptr 管理 InstanceData
   class InstancedRenderer {
   private:
       std::shared_ptr<InstanceData> m_instances;  // 多个渲染器可共享实例数据
   };
   ```

**InstanceData.cpp 的实现**：

```cpp
// src/Renderer/InstanceData.cpp:8-20
void InstanceData::Add(const glm::vec3& position,
                       const glm::vec3& rotation,
                       const glm::vec3& scale,
                       const glm::vec3& color)
{
    // ✅ 直接存储值（无需 shared_ptr）
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);

    m_modelMatrices.push_back(model);  // ✅ 值拷贝
    m_colors.push_back(color);         // ✅ 值拷贝
}
```

---

## 📊 shared_ptr 使用总结

### 使用 shared_ptr 的场景

| 类 | 成员变量 | 使用 shared_ptr | 原因 |
|---|---------|----------------|------|
| **SimpleMesh** | `m_texture` | ✅ **是** | 多个 mesh 可共享同一纹理，避免重复加载 |
| **InstancedRenderer** | `m_mesh` | ✅ **是** | 多个渲染器可共享同一网格模板 |
| **InstancedRenderer** | `m_instances` | ✅ **是** | 多个渲染器可共享同一实例数据 |
| **InstancedRenderer** | `m_texture` | ✅ **是** | 渲染器共享 mesh 的纹理 |

### 不使用 shared_ptr 的场景

| 类 | 成员变量 | 使用 shared_ptr | 原因 |
|---|---------|----------------|------|
| **InstanceData** | `m_modelMatrices` | ❌ **否** | 值类型，直接存储向量 |
| **InstanceData** | `m_colors` | ❌ **否** | 值类型，直接存储向量 |
| **SimpleMesh** | `m_vertices` | ❌ **否** | 值类型，直接存储向量 |
| **SimpleMesh** | `m_indices` | ❌ **否** | 值类型，直接存储向量 |

---

## 🎯 设计原则

### 何时使用 shared_ptr

✅ **需要共享所有权**
```cpp
// 场景：多个 SimpleMesh 共享同一纹理
auto texture = std::make_shared<Texture>();
texture->LoadFromFile("brick.png");

mesh1->SetTexture(texture);  // 引用计数 = 1
mesh2->SetTexture(texture);  // 引用计数 = 2
mesh3->SetTexture(texture);  // 引用计数 = 3
// texture 在所有 mesh 销毁后自动释放
```

✅ **生命周期不确定**
```cpp
// 场景：通过接口返回，调用者负责管理
std::shared_ptr<Texture> LoadTexture(const std::string& path) {
    auto texture = std::make_shared<Texture>();
    texture->LoadFromFile(path);
    return texture;  // 调用者共享所有权
}
```

### 何时不需要 shared_ptr

❌ **值类型**
```cpp
// 场景：存储简单的数据值
struct VertexData {
    std::vector<float> vertices;  // ❌ 不需要 shared_ptr
    std::vector<unsigned int> indices;  // ❌ 不需要 shared_ptr
};
```

❌ **独占所有权**
```cpp
// 场景：只有一个所有者
class Mesh {
private:
    std::unique_ptr<OpenGLBuffer> m_vbo;  // ✅ 使用 unique_ptr（独占所有权）
};
```

---

## 📈 性能影响

### shared_ptr 的开销

**内存开销**：
```cpp
裸指针：8 bytes（64位系统）
shared_ptr：16 bytes（指针 + 控制块指针）
```

**操作开销**：
```cpp
拷贝：原子递增引用计数（~10 ns）
移动：零拷贝（~0.5 ns）
析构：原子递减引用计数（~10 ns）
```

### 在本项目的应用

**SimpleMesh 使用 shared_ptr<Texture>**：
```
优势：
✅ 自动内存管理，消除泄漏
✅ 支持纹理共享（减少重复加载）
✅ 异常安全

劣势：
❌ 每个纹理多占用 8 bytes
❌ 拷贝时原子操作（~10 ns）

结论：优势 >> 劣势，应该使用 ✅
```

**InstanceData 不使用 shared_ptr**：
```
优势：
✅ 值类型，语义清晰
✅ 无额外开销
✅ 缓存友好（连续内存）

劣势：
❌ 拷贝时需要复制数据（但通常使用引用传递）

结论：不需要 shared_ptr ✅
```

---

## 🔍 代码审查清单

### ✅ 已正确使用 shared_ptr 的地方

1. **SimpleMesh::m_texture** ✅
   - 多个 mesh 可共享纹理
   - 避免重复加载
   - 自动生命周期管理

2. **InstancedRenderer::m_mesh** ✅
   - 多个渲染器可共享网格模板
   - 使用 `shared_ptr<SimpleMesh>`

3. **InstancedRenderer::m_instances** ✅
   - 多个渲染器可共享实例数据
   - 使用 `shared_ptr<InstanceData>`

4. **InstancedRenderer::m_texture** ✅
   - 从 mesh 继承纹理所有权
   - 使用 `shared_ptr<Texture>`

### ✅ 正确不使用 shared_ptr 的地方

1. **InstanceData 的成员变量** ✅
   - `m_modelMatrices` 和 `m_colors` 是值类型
   - 不需要共享所有权

2. **SimpleMesh 的顶点数据** ✅
   - `m_vertices` 和 `m_indices` 是值类型
   - OpenGL 已上传数据，CPU 端可销毁

---

## 📚 总结

| 文件 | 是否使用 shared_ptr | 是否正确 |
|------|---------------------|----------|
| **SimpleMesh.cpp** | ✅ 是（纹理） | ✅ 正确 |
| **InstanceData.cpp** | ❌ 否（数据容器） | ✅ 正确 |

### 设计理念

```
SimpleMesh:
├─ m_texture: shared_ptr<Texture>  ✅ 共享纹理
├─ m_vertices: vector<float>       ✅ 值类型
└─ m_indices: vector<uint>         ✅ 值类型

InstanceData:
├─ m_modelMatrices: vector<mat4>   ✅ 值类型
└─ m_colors: vector<vec3>          ✅ 值类型

InstancedRenderer:
├─ m_mesh: shared_ptr<SimpleMesh>  ✅ 共享网格
├─ m_instances: shared_ptr<InstanceData>  ✅ 共享数据
└─ m_texture: shared_ptr<Texture>  ✅ 共享纹理
```

**结论**：当前的 shared_ptr 使用是**完全正确**的！🎉

---

*检查完成日期：2026-01-01*
