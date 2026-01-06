# Lumenaris 3D渲染引擎 - 整体架构设计

## 2. 整体架构设计

### 2.1 层次结构与模块划分

项目采用**三模块静态库架构**，通过CMake构建系统实现编译时模块化：

**模块结构**：

| 模块名称 | 类型 | 职责 | 源文件位置 |
|---------|------|------|-----------|
| **Core** | 静态库 | 窗口、输入、摄像机、日志系统 | `src/Core/`, `include/Core/` |
| **Renderer** | 静态库 | 渲染管线、光照、资源管理 | `src/Renderer/`, `include/Renderer/` |
| **Geometry** | 对象库 | 纯静态几何体数据生成 | `src/Renderer/Geometry/`, `src/Renderer/Data/`, `src/Renderer/Factory/`, `src/Renderer/Renderer/` |
| **HelloWindow** | 可执行文件 | 主程序入口与场景编排 | `src/main.cpp` |

**模块依赖关系**：

```
HelloWindow → Core, Renderer, Geometry
     ↓
Renderer → Core（仅依赖Logger）
     ↓
Geometry → Renderer, Core（使用其资源管理）
```

**依赖特点**：
- HelloWindow依赖所有三个模块
- Renderer仅依赖Core的Logger，保持低耦合
- Geometry无依赖（纯数据生成），但编译时链接Renderer和Core
- 单向依赖，无循环依赖

---

### 2.2 模块职责详解

#### 2.2.1 Core模块：系统基础设施

**核心组件**：

| 类名 | 文件位置 | 职责范围 | 设计模式 |
|------|---------|---------|---------|
| **Window** | `Core/Window.hpp` | GLFW窗口封装与OpenGL上下文管理，窗口大小调整，事件循环处理 | RAII资源管理 |
| **Camera** | `Core/Camera.hpp` | 6DOF摄像机（位置、方向、移动），透视/正交投影切换，View/Projection矩阵计算，欧拉角控制 | 无状态计算 |
| **MouseController** | `Core/MouseController.hpp` | 鼠标事件捕获（偏移量），摄像机旋转控制，滚轮缩放（FOV调整），灵敏度配置 | 观察者模式 |
| **KeyboardController** | `Core/KeyboardController.hpp` | 多键位状态管理（支持同时按下），事件回调注册，按键重复和防抖 | 事件驱动 |
| **Logger** | `Core/Logger.hpp` | 异步分级日志（DEBUG/INFO/WARNING/ERROR），日志轮转（按大小/天/小时），统计功能（FPS、DrawCall） | 单例 + 双缓冲队列 |

**核心设计理念**：

**Window类**：
- 封装GLFW窗口创建和初始化
- 自动处理窗口大小调整回调
- 提供RAII资源管理（析构时自动清理GLFW资源）
- 管理OpenGL上下文生命周期

**Camera类**：
- 支持6自由度移动（WASD前后左右 + QE上下）
- 支持透视投影和正交投影切换
- 欧拉角控制（偏航角Yaw、俯仰角Pitch）
- 提供LookAt功能（观察指定目标点）
- 移动速度使用deltaTime归一化，保证不同帧率下速度一致

**MouseController类**：
- 捕获鼠标移动偏移量（用于摄像机旋转）
- 捕获滚轮滚动（用于FOV缩放）
- 支持鼠标捕获状态切换（光标隐藏/显示）
- 灵活的鼠标灵敏度和滚轮灵敏度配置

**KeyboardController类**：
- 支持多键同时按下检测（如W+D同时按）
- 事件驱动的按键回调注册机制
- 按键重复和防抖功能（避免按键抖动）
- 状态查询接口（IsKeyPressed、IsKeyJustPressed、IsKeyJustReleased）

**Logger类**：
- **异步写入**：后台线程写入日志，不阻塞主线程
- **分级日志**：DEBUG级别在Release模式零开销
- **日志轮转**：支持按文件大小、每日、每小时轮转
- **统计功能**：记录着色器激活次数、纹理绑定次数、DrawCall次数、FPS监控

---

#### 2.2.2 Renderer模块：渲染管线核心

**核心架构演进**：从传统继承式（IMesh接口）重构为**数据-资源-渲染**三职责分离模式

**架构对比**：

| 架构类型 | 设计方式 | 优点 | 缺点 |
|---------|---------|------|------|
| **旧架构**（已废弃） | 继承式：每个几何体继承IMesh接口，实现Create()和Draw() | 简单直观 | 职责混乱（数据+GPU+渲染），代码重复，难以扩展 |
| **新架构**（2026-01-02） | 职责分离：纯静态数据类 + GPU资源管理 + 渲染器 | 职责清晰，代码精简54-62%，易扩展 | 需要理解三层设计 |

**职责分离设计**：

| 类名 | 层级 | 职责 | 设计原则 |
|------|------|------|---------|
| **MeshData** | 数据层 | CPU端纯数据容器（顶点、索引、布局信息） | 无GPU资源，可序列化，可传递 |
| **MeshBuffer** | 资源层 | GPU资源包装器（管理VAO/VBO/EBO） | 管理OpenGL缓冲区，持有CPU数据副本 |
| **InstanceData** | 数据层 | 实例数据容器（模型矩阵、颜色） | 脏标记优化，支持动态更新 |
| **InstancedRenderer** | 渲染层 | 渲染逻辑（批量实例化渲染） | 继承IRenderer接口，管理instanceVBO |

---

**1. 数据容器层** (`Renderer/Data/`)

**MeshData类**：

| 方法类型 | 方法名 | 功能说明 | 移动语义支持 |
|---------|--------|---------|-------------|
| 数据设置 | `SetVertices()` | 设置顶点数据数组 | ✅ 支持右值引用 |
| 数据设置 | `SetIndices()` | 设置索引数据数组 | ✅ 支持右值引用 |
| 数据设置 | `SetVertexLayout()` | 设置顶点属性布局（偏移和大小） | ❌ 仅左值引用 |
| 数据设置 | `SetMaterialColor()` | 设置材质颜色 | ❌ 仅左值引用 |
| 数据访问 | `GetVertices()` | 获取顶点数据（只读） | - |
| 数据访问 | `GetIndices()` | 获取索引数据（只读） | - |
| 数据访问 | `GetVertexCount()` | 获取顶点数量 | - |
| 数据访问 | `HasIndices()` | 是否有索引数据 | - |
| 工具方法 | `Clear()` | 清空所有数据 | - |
| 工具方法 | `IsEmpty()` | 检查是否为空 | - |

**设计特点**：
- ✅ 纯数据容器，无GPU资源（无VAO/VBO/EBO）
- ✅ 无渲染能力（无Draw/Render方法）
- ✅ 可序列化、可传递、可复制
- ✅ 支持移动语义（避免数据拷贝）

---

**MeshBuffer类**：

| 方法类型 | 方法名 | 功能说明 | 移动语义支持 |
|---------|--------|---------|-------------|
| GPU操作 | `UploadToGPU()` | 上传数据到GPU（创建VAO/VBO/EBO） | ✅ 支持右值引用 |
| GPU操作 | `ReleaseGPU()` | 释放GPU资源 | - |
| 访问接口 | `GetVAO()` | 获取OpenGL VAO ID | - |
| 访问接口 | `GetVertexCount()` | 获取顶点数量 | - |
| 访问接口 | `HasIndices()` | 是否有索引数据 | - |
| 纹理管理 | `SetTexture()` | 设置纹理（shared_ptr管理） | - |
| 纹理管理 | `GetTexture()` | 获取纹理（shared_ptr） | - |
| 纹理管理 | `HasTexture()` | 是否有纹理 | - |

**拷贝/移动语义**：

| 操作 | 构造函数 | 赋值运算符 | 说明 |
|------|---------|-----------|------|
| **拷贝** | ❌ 已删除 | ❌ 已删除 | 防止意外深拷贝（昂贵操作） |
| **移动** | ✅ 默认 | ✅ 默认 | 高效转移GPU资源所有权 |

**设计特点**：
- ✅ 持有CPU数据副本（支持深拷贝）
- ✅ 管理GPU资源（VAO/VBO/EBO）
- ✅ 不继承任何接口（不是可渲染对象）
- ✅ 不提供Draw/Render方法（由Renderer负责）
- ✅ 使用shared_ptr管理纹理生命周期

---

**InstanceData类**：

| 方法类型 | 方法名 | 功能说明 | 自动标记脏 |
|---------|--------|---------|-----------|
| 实例管理 | `Add()` | 添加单个实例（位置、旋转、缩放、颜色） | ✅ 是 |
| 实例管理 | `AddBatch()` | 批量添加实例（矩阵数组、颜色数组） | ✅ 是 |
| 实例管理 | `Clear()` | 清除所有实例 | ✅ 是 |
| 数据访问 | `GetModelMatrices()` | 获取模型矩阵数组（const版本） | - |
| 数据访问 | `GetModelMatrices()` | 获取模型矩阵数组（可修改版本） | ⚠️ 需手动标记脏 |
| 数据访问 | `GetColors()` | 获取颜色数组（const版本） | - |
| 数据访问 | `GetColors()` | 获取颜色数组（可修改版本） | ⚠️ 需手动标记脏 |
| 脏标记 | `IsDirty()` | 检查数据是否被修改（需要更新GPU） | - |
| 脏标记 | `ClearDirty()` | 清除脏标记（数据已同步到GPU） | - |
| 脏标记 | `MarkDirty()` | 手动标记为脏（数据已修改） | - |
| 便捷方法 | `SetModelMatrix()` | 直接设置单个实例的矩阵 | ✅ 自动标记脏 |
| 便捷方法 | `SetColor()` | 直接设置单个实例的颜色 | ✅ 自动标记脏 |

**设计特点**：
- ✅ 纯数据容器，无GPU资源
- ✅ 脏标记机制：避免冗余GPU更新
- ✅ 自动标记：Add/Clear/SetModelMatrix/SetColor自动设置脏标记
- ✅ 手动控制：支持MarkDirty和ClearDirty
- ⚠️ **调用者负责清除**：多材质模型需在所有渲染器更新完毕后清除

**脏标记性能优化**：

| 场景 | 性能表现 | 说明 |
|------|---------|------|
| **动画运行时** | 性能相同 | 数据确实在变化，每帧都需要更新 |
| **动画暂停时** | 帧率 +50%，GPU带宽节省 100% | 跳过GPU数据传输 |
| **静态几何体** | 完全跳过传输 | 零开销 |

---

**2. 光照系统** (`Renderer/Lighting/`)

**Light类（光源基类）**：

| 方法名 | 功能说明 |
|--------|---------|
| `GetType()` | 获取光源类型（纯虚函数） |
| `ApplyToShader()` | 将光源数据传递给着色器（纯虚函数） |
| `GetDescription()` | 获取光源描述（纯虚函数） |
| `GetColor()` / `SetColor()` | 获取/设置光源颜色 |
| `GetIntensity()` / `SetIntensity()` | 获取/设置光照强度 |
| `IsEnabled()` / `SetEnabled()` | 检查/设置光源开关状态 |
| `Toggle()` | 切换光源开关状态 |
| `GetAmbient()` / `SetAmbient()` | 获取/设置环境光分量 |
| `GetDiffuse()` / `SetDiffuse()` | 获取/设置漫反射分量 |
| `GetSpecular()` / `SetSpecular()` | 获取/设置镜面反射分量 |

**光源类型**：

| 类型枚举 | 说明 | 应用场景 |
|---------|------|---------|
| `DIRECTIONAL` | 平行光（方向光） | 太阳光、月光 |
| `POINT` | 点光源（从一个点向所有方向发光） | 灯泡、蜡烛 |
| `SPOT` | 聚光灯（从一点向特定方向锥形发光） | 手电筒、舞台灯光 |

---

**LightWithAttenuation类（带衰减的光源基类）**：

| 功能 | 说明 |
|------|------|
| **设计目标** | 消除PointLight和SpotLight之间的代码重复（DRY原则） |
| **继承关系** | PointLight和SpotLight都继承此类 |
| **公共属性** | 位置（position）、衰减参数（attenuation） |
| **虚函数** | `GetEffectiveRange()` - 支持多态调用，计算有效距离 |

**Attenuation预设范围**：

| 预设方法 | 有效距离 | 适用场景 |
|---------|---------|---------|
| `Range7()` | 7米 | 小范围光源（蜡烛） |
| `Range13()` | 13米 | 中小范围光源 |
| `Range20()` | 20米 | 中范围光源（室内灯） |
| `Range32()` | 32米 | 中大范围光源 |
| `Range50()` | 50米 | 大范围光源（路灯） |
| `Range65()` | 65米 | 超大范围光源 |
| `Range100()` | 100米 | 超大范围光源 |

---

**LightHandle类（光源句柄）**：

| 方法名 | 功能说明 |
|--------|---------|
| `GetId()` | 获取稳定ID |
| `GetGeneration()` | 获取代数标记（用于检测句柄是否失效） |
| `GetType()` | 获取光源类型 |
| `IsValid()` | 检查句柄是否有效 |

**设计特点**：
- ✅ 使用稳定的 `id + generation` 机制（避免索引失效问题）
- ✅ 禁用拷贝，仅可移动（避免意外复制）
- ✅ 类型安全，包含光源类型标签
- ✅ 线程安全（所有操作都是只读的）

**拷贝/移动语义**：

| 操作 | 构造函数 | 赋值运算符 |
|------|---------|-----------|
| **拷贝** | ❌ 已删除 | ❌ 已删除 |
| **移动** | ✅ 默认 | ✅ 默认 |

---

**LightManager类（光照管理器）**：

| 功能分类 | 方法名 | 功能说明 | 线程安全 |
|---------|--------|---------|---------|
| **数量限制** | `MAX_DIRECTIONAL_LIGHTS` | 最多4个平行光 | - |
| | `MAX_POINT_LIGHTS` | 最多48个点光源 | - |
| | `MAX_SPOT_LIGHTS` | 最多8个聚光灯 | - |
| **添加光源** | `AddDirectionalLight()` | 添加平行光，返回LightHandle | ✅ 写操作（独占锁） |
| | `AddPointLight()` | 添加点光源，返回LightHandle | ✅ 写操作（独占锁） |
| | `AddSpotLight()` | 添加聚光灯，返回LightHandle | ✅ 写操作（独占锁） |
| **移除光源** | `RemoveDirectionalLight()` | 移除平行光（使用LightHandle） | ✅ 写操作（独占锁） |
| | `RemovePointLight()` | 移除点光源（使用LightHandle） | ✅ 写操作（独占锁） |
| | `RemoveSpotLight()` | 移除聚光灯（使用LightHandle） | ✅ 写操作（独占锁） |
| **获取光源** | `GetDirectionalLight()` | 获取平行光（使用LightHandle） | ✅ 读操作（共享锁） |
| | `GetPointLight()` | 获取点光源（使用LightHandle） | ✅ 读操作（共享锁） |
| | `GetSpotLight()` | 获取聚光灯（使用LightHandle） | ✅ 读操作（共享锁） |
| **查询统计** | `GetDirectionalLightCount()` | 获取平行光数量 | ✅ 读操作（共享锁） |
| | `GetPointLightCount()` | 获取点光源数量 | ✅ 读操作（共享锁） |
| | `GetSpotLightCount()` | 获取聚光灯数量 | ✅ 读操作（共享锁） |
| | `GetTotalLightCount()` | 获取总光源数量 | ✅ 读操作（共享锁） |
| **应用光源** | `ApplyToShader()` | 将所有光源应用到着色器 | ✅ 读操作（共享锁） |
| **调试信息** | `GetStatistics()` | 获取统计信息 | - |
| | `PrintAllLights()` | 打印所有光源信息 | - |

**线程安全设计**：
- ✅ 使用 `std::shared_mutex` 实现读写锁
- ✅ 读操作使用共享锁（允许并发读）
- ✅ 写操作使用独占锁
- ✅ `ApplyToShader` 可在渲染线程并发调用

**架构更新**（2026-01-02）：
- ✅ 线程安全：使用 `std::shared_mutex` 支持读写并发
- ✅ 稳定引用：使用 `LightHandle` 替代容易失效的索引
- ✅ 修复ODR违规：使用 `inline static constexpr`
- ✅ 修复Uniform未初始化：禁用光源时设置零值

---

**3. 环境渲染系统** (`Renderer/Environment/`)

**Skybox类（天空盒）**：

| 方法名 | 功能说明 |
|--------|---------|
| `Initialize()` | 初始化天空盒，创建立方体网格 |
| `Load()` | 从6个纹理文件加载天空盒 |
| `LoadFromConfig()` | 从配置加载天空盒 |
| `LoadShaders()` | 加载天空盒着色器 |
| `Render()` | 渲染天空盒（projection、view矩阵） |
| `BindTexture()` | 绑定天空盒纹理到指定纹理单元 |
| `GetTextureID()` | 获取天空盒纹理ID |
| `IsLoaded()` | 检查天空盒是否已加载 |
| `SetRotation()` / `GetRotation()` | 设置/获取旋转角度 |

**渲染特性**：
- 视差移除（深度测试改为GL_LEQUAL）
- 无深度写入（glDepthMask(GL_FALSE)）
- 立方体贴图渲染

---

**SkyboxLoader类（天空盒加载工具）**：

| Cubemap约定 | 面顺序 | 适用场景 |
|------------|--------|---------|
| `OPENGL` | right, left, top, bottom, front(+Z), back(-Z) | OpenGL标准 |
| `DIRECTX` | left, right, top, bottom, front, back | DirectX应用 |
| `MAYA` | right, left, top, bottom, back(+Z), front(-Z) | Maya/Corona |
| `BLENDER` | right, left, top, bottom, front, back | Blender |
| `CUSTOM` | 自定义映射 | 特殊需求 |

**静态工厂方法**：

| 方法名 | 功能说明 |
|--------|---------|
| `CreateConfig()` | 从标准命名约定创建配置 |
| `CreateCustomConfig()` | 从自定义文件名创建配置 |
| `CreateFromPattern()` | 从文件模式创建配置（支持通配符） |
| `CreateFromCustomScheme()` | 从自定义面命名方案创建配置 |
| `ConvertToOpenGL()` | 转换到OpenGL标准顺序 |
| `GetOpenGLScheme()` | 获取OpenGL标准命名方案 |
| `GetMayaScheme()` | 获取Maya命名方案 |
| `GetDirectXScheme()` | 获取DirectX命名方案 |
| `GetHDRLabScheme()` | 获取HDR Lab命名方案 |

**设计优势**：
- ✅ 支持多种cubemap约定（OpenGL/DirectX/Maya/Blender）
- ✅ 自动转换面顺序
- ✅ 灵活的命名格式支持

---

**AmbientLighting类（环境光照）**：

| 环境光模式 | 说明 | 应用场景 |
|-----------|------|---------|
| `SOLID_COLOR` | 固定颜色环境光（传统Phong） | 室内场景 |
| `SKYBOX_SAMPLE` | 从天空盒采样环境光（IBL） | 室外场景 |
| `HEMISPHERE` | 半球渐变环境光（天空/地面颜色插值） | 自然场景 |

| 方法名 | 功能说明 |
|--------|---------|
| `Initialize()` | 初始化环境光照系统 |
| `LoadFromSkybox()` | 从天空盒加载环境光照 |
| `SetMode()` / `GetMode()` | 设置/获取环境光照模式 |
| `SetSkyColor()` | 设置天空颜色（用于HEMISPHERE模式） |
| `SetGroundColor()` | 设置地面颜色（用于HEMISPHERE模式） |
| `SetIntensity()` / `GetIntensity()` | 设置/获取环境光强度 |
| `ApplyToShader()` | 将环境光照设置应用到着色器 |

**设计特点**：
- ✅ 轻量级环境光照系统（非PBR）
- ✅ 支持三种环境光模式
- ✅ 运行时模式切换

---

**4. RenderContext类（渲染上下文）**：

**多Context架构核心**：

| 组件 | 访问方法 | 说明 |
|------|---------|------|
| **光照管理器** | `GetLightManager()` | 每个Context拥有独立的光照管理器实例 |
| **天空盒** | `GetSkybox()` | 每个Context可以拥有独立的天空盒 |
| **环境光照** | `GetAmbientLighting()` | 每个Context可以拥有独立的环境光照设置 |
| **清空状态** | `Clear()` | 清空Context所有状态（光源、环境光） |
| **统计信息** | `GetStatistics()` | 获取Context统计信息（用于调试） |

**设计优势**：
- ✅ 每个渲染场景（主场景、ImGui层、预览窗口）拥有独立Context
- ✅ 避免单例模式导致的全局状态污染
- ✅ 线程安全：每个Context独立加锁，无跨Context锁竞争
- ✅ 可测试性：每个Context独立创建销毁

**应用场景**：

| 场景 | Context配置 | 说明 |
|------|------------|------|
| **主游戏场景** | 丰富光照（48点光源 + 4平行光 + 8聚光灯） | 主要渲染场景 |
| **ImGui 2D层** | 零光照环境 | 不影响主场景 |
| **预览窗口** | 完全独立的场景 | 无状态覆盖问题 |

---

**5. 渲染器层** (`Renderer/Renderer/`)

**IRenderer接口（渲染器抽象接口）**：

| 方法 | 说明 |
|------|------|
| `Initialize()` | 初始化渲染器（创建OpenGL缓冲区等） |
| `Render()` | 执行渲染操作 |
| `GetName()` | 获取渲染器名称（用于调试） |

---

**InstancedRenderer类（实例化渲染器）**：

| 方法类型 | 方法名 | 功能说明 |
|---------|--------|---------|
| **IRenderer接口** | `Initialize()` | 初始化实例化渲染器，上传实例数据并设置实例化属性 |
| | `Render()` | 执行实例化渲染（glDrawElementsInstanced） |
| | `GetName()` | 返回"InstancedRenderer" |
| **资源设置** | `SetMesh()` | 设置网格模板（使用shared_ptr管理所有权） |
| | `SetInstances()` | 设置实例数据（使用shared_ptr避免拷贝） |
| | `SetMaterialColor()` | 设置材质颜色 |
| | `SetTexture()` | 设置纹理（使用shared_ptr管理所有权） |
| **信息查询** | `GetInstanceCount()` | 获取实例数量 |
| | `GetMesh()` | 获取网格（shared_ptr） |
| | `GetInstances()` | 获取实例数据（shared_ptr） |
| | `HasTexture()` | 是否有纹理 |
| **动画支持** | `UpdateInstanceData()` | 更新实例数据到GPU（用于动画） |
| **静态工厂** | `CreateForCube()` | 为Cube创建实例化渲染器 |
| | `CreateForOBJ()` | 为OBJ模型创建多个材质渲染器（返回渲染器、meshBuffer、instanceData的shared_ptr） |
| **批量渲染** | `RenderBatch()` | 静态方法：按纹理分组渲染多个渲染器，减少OpenGL状态切换 |

**职责分离设计**：

| 类名 | 职责 | 所有权管理 |
|------|------|-----------|
| **MeshBuffer** | GPU资源包装器（VAO/VBO/EBO） | shared_ptr管理 |
| **InstanceData** | 实例数据容器（矩阵、颜色） | shared_ptr管理 |
| **InstancedRenderer** | 渲染逻辑（批量实例化） | 继承IRenderer接口 |

**批量渲染优化**（2026-01-02）：

| 场景 | 传统渲染 | 批量渲染 | 性能提升 |
|------|---------|---------|---------|
| **12辆车 × 38材质** | 456次DrawCall | 38次DrawCall | **12倍** |
| **Disco舞台（46渲染器）** | 184次状态切换/帧 | ~60次状态切换/帧 | **减少67%** |

**批量渲染重载**：

| 方法签名 | 参数类型 | 说明 |
|---------|---------|------|
| `RenderBatch()` | `vector<InstancedRenderer*>` | 指针版本 |
| `RenderBatch()` | `vector<unique_ptr<InstancedRenderer>>` | unique_ptr版本 |
| `RenderBatch()` | `vector<InstancedRenderer>&` | 值类型版本（推荐） |

**拷贝/移动语义**：

| 操作 | 构造函数 | 赋值运算符 | 说明 |
|------|---------|-----------|------|
| **拷贝** | ❌ 已删除 | ❌ 已删除 | 防止OpenGL资源双重释放 |
| **移动** | ✅ 默认 | ✅ 默认 | 转移OpenGL资源所有权 |

**性能优化**：
- ✅ 单次绘制调用渲染数百个相同几何体
- ✅ 每个实例独立的变换矩阵和颜色属性
- ✅ 使用glVertexAttribDivisor实现属性实例化
- ✅ 支持多材质OBJ模型（每个材质一个渲染器）

---

**6. 资源管理** (`Renderer/Resources/`)

**Shader类（着色器管理）**：

| 方法名 | 功能说明 |
|--------|---------|
| `Load()` | 编译并链接顶点和片段着色器 |
| `Use()` | 激活当前着色器程序 |
| `SetMat4()` | 设置4x4矩阵uniform变量 |
| `SetVec3()` | 设置3D向量uniform变量 |
| `SetFloat()` | 设置浮点数uniform变量 |
| `SetInt()` | 设置整数uniform变量 |
| `SetBool()` | 设置布尔值uniform变量 |
| `GetID()` | 返回OpenGL着色器程序ID |

---

**Texture类（纹理管理）**：

| 方法名 | 功能说明 |
|--------|---------|
| `LoadFromFile()` | 从文件加载纹理，支持PNG、JPG、BMP格式 |
| `Bind()` | 将纹理绑定到指定的纹理单元 |
| `Unbind()` | 解绑当前纹理 |
| `GetID()` | 返回OpenGL纹理对象ID |
| `IsLoaded()` | 检查纹理是否成功加载 |
| `GetFilePath()` | 获取纹理文件路径 |

---

#### 2.2.3 Geometry模块：数据生成层

**纯静态工具类设计**（2026-01-02重构）：

所有几何体类均为**纯静态工具类**，禁止实例化，只负责数据生成：

| 类名 | 文件位置 | 主要静态方法 | 参数化支持 | 说明 |
|------|---------|-------------|-----------|------|
| **Cube** | `Renderer/Geometry/Cube.hpp` | `GetVertexData()`<br>`GetVertexLayout()` | ❌ 固定大小 | 24个顶点（位置3+法线3+UV2） |
| **Sphere** | `Renderer/Geometry/Sphere.hpp` | `GetVertexData(radius, stacks, slices)`<br>`GetIndexData(stacks, slices)` | ✅ 半径、分段数 | 支持运行时参数调整 |
| **Torus** | `Renderer/Geometry/Torus.hpp` | `GetVertexData(majorRadius, minorRadius, majorSegments, minorSegments)`<br>`GetIndexData(majorSegments, minorSegments)` | ✅ 主半径、管半径、分段 | 支持运行时参数调整 |
| **Plane** | `Renderer/Geometry/Plane.hpp` | `GetVertexData(width, height, widthSegments, heightSegments)`<br>`GetIndexData(widthSegments, heightSegments)` | ✅ 宽高、分段数 | 支持运行时参数调整 |
| **OBJModel** | `Renderer/Geometry/OBJModel.hpp` | `GetMaterialVertexData(objPath)`<br>`GetMeshData(objPath)`<br>`GetMaterials(objPath)`<br>`HasMaterials(objPath)` | ✅ OBJ文件路径 | 支持多材质模型 |

**架构优势**：

| 特性 | 说明 |
|------|------|
| **职责分离** | 几何体类只负责数据生成，不涉及GPU操作 |
| **无实例状态** | 所有方法都是静态的，线程安全 |
| **参数化支持** | Sphere、Torus、Plane支持运行时参数调整 |
| **代码精简** | 相比重构前，代码量减少54-62% |

**重构前后代码量对比**：

| 类名 | 重构前行数 | 重构后行数 | 减少比例 |
|------|-----------|-----------|---------|
| **Cube** | 130行 | 59行 | **54%** |
| **Sphere** | 231行 | 88行 | **62%** |
| **Plane** | 216行 | 88行 | **59%** |
| **Torus** | 180行 | 99行 | **45%** |
| **OBJModel** | 390行 | 177行 | **55%** |

**MeshDataFactory类（网格数据工厂）**：

| 方法名 | 功能说明 | 返回值类型 |
|--------|---------|-----------|
| `CreateCubeData()` | 创建立方体数据 | MeshData（CPU纯数据） |
| `CreateSphereData()` | 创建球体数据（参数化） | MeshData（CPU纯数据） |
| `CreateTorusData()` | 创建圆环体数据（参数化） | MeshData（CPU纯数据） |
| `CreatePlaneData()` | 创建平面数据（参数化） | MeshData（CPU纯数据） |
| `CreateOBJData()` | 从OBJ文件创建数据（每个材质一个） | vector<MeshData> |

**MeshBufferFactory类（网格缓冲区工厂）**：

| 方法名 | 功能说明 | 返回值类型 |
|--------|---------|-----------|
| `CreateCubeBuffer()` | 创建立方体GPU缓冲区（已上传） | MeshBuffer |
| `CreateSphereBuffer()` | 创建球体GPU缓冲区（已上传，参数化） | MeshBuffer |
| `CreateTorusBuffer()` | 创建圆环体GPU缓冲区（已上传，参数化） | MeshBuffer |
| `CreatePlaneBuffer()` | 创建平面GPU缓冲区（已上传，参数化） | MeshBuffer |
| `CreateOBJBuffers()` | 从OBJ文件创建GPU缓冲区（已上传，每个材质一个） | vector<MeshBuffer> |

**推荐使用方式**：

| 场景 | 推荐方法 | 说明 |
|------|---------|------|
| **直接创建GPU资源** | `MeshBufferFactory::CreateXXXBuffer()` | 最常用，一步到位 |
| **需要CPU数据** | 几何体类静态方法 + `MeshBuffer::UploadToGPU()` | 灵活控制 |
| **OBJ模型** | `MeshBufferFactory::CreateOBJBuffers()` | 自动处理多材质 |

---

### 2.3 设计模式应用

#### 2.3.1 工厂模式（MeshDataFactory/MeshBufferFactory）

| 类型 | 实现方式 | 优势 | 应用场景 |
|------|---------|------|---------|
| **编译时工厂** | 静态工厂方法，每个几何体有独立的Create方法 | 编译时类型检查，性能优于运行时工厂 | 创建几何体数据和GPU缓冲区 |

**工厂方法列表**：

| 工厂类 | 方法 | 参数 | 返回值 |
|--------|------|------|--------|
| **MeshDataFactory** | `CreateCubeData()` | 无 | MeshData |
| | `CreateSphereData()` | stacks, slices, radius | MeshData |
| | `CreateTorusData()` | majorRadius, minorRadius, majorSegments, minorSegments | MeshData |
| | `CreatePlaneData()` | width, height, widthSegments, heightSegments | MeshData |
| | `CreateOBJData()` | objPath | vector<MeshData> |
| **MeshBufferFactory** | `CreateCubeBuffer()` | 无 | MeshBuffer |
| | `CreateSphereBuffer()` | stacks, slices, radius | MeshBuffer |
| | `CreateTorusBuffer()` | majorRadius, minorRadius, majorSegments, minorSegments | MeshBuffer |
| | `CreatePlaneBuffer()` | width, height, widthSegments, heightSegments | MeshBuffer |
| | `CreateOBJBuffers()` | objPath | vector<MeshBuffer> |

---

#### 2.3.2 观察者模式（输入系统）

| 组件 | 角色 | 注册方式 | 触发方式 |
|------|------|---------|---------|
| **KeyboardController** | 主题（Subject） | `RegisterKeyCallback(key, callback)` | 按键事件发生时 |
| **主程序** | 观察者（Observer） | 回调函数 | 接收按键通知 |

**优势**：
- ✅ 解耦事件产生者和消费者
- ✅ 支持多个监听器
- ✅ 灵活的回调注册机制

---

#### 2.3.3 策略模式（渲染风格）

| 策略 | 实现方式 | 切换方式 |
|------|---------|---------|
| **着色器策略** | 运行时加载不同的片段着色器 | 动态切换Shader对象 |
| **渲染策略** | 使用不同的着色器文件 | 重新Load着色器 |

**优势**：
- ✅ 支持多种渲染策略
- ✅ 提高代码复用性
- ✅ 灵活的着色器组合

---

#### 2.3.4 脏标记模式（动画优化）

| 角色 | 职责 | 方法 |
|------|------|------|
| **InstanceData** | 管理脏标记 | `IsDirty()`, `MarkDirty()`, `ClearDirty()` |
| **调用者** | 控制脏标记生命周期 | 在所有渲染器更新后统一清除 |

**设计原则**：

| 原则 | 说明 | 示例 |
|------|------|------|
| **调用者管理** | 脏标记由调用者管理，不在被调用者中自动清除 | 多材质模型的主程序负责清除 |
| **避免副作用** | 避免第一个消费者清除标记后，其余消费者跳过更新 | 确保所有渲染器都能检测到脏标记 |
| **共享状态** | 适用于多个消费者共享同一个状态标志的场景 | 38个材质渲染器共享1个InstanceData |

**应用场景**：

| 场景 | 说明 |
|------|------|
| **多材质OBJ模型动画** | 38个材质共享1个InstanceData，需统一清除脏标记 |
| **批量渲染动态更新** | 多个渲染器批量更新实例数据 |
| **缓存失效同步** | 共享缓存的状态管理 |

**正确更新流程**：

```
1. 修改 InstanceData 中的模型矩阵
   ↓
2. 调用 instanceData->MarkDirty() 标记数据为脏
   ↓
3. 更新所有渲染器的 instanceVBO（多材质）
   for (auto& renderer : renderers) {
       renderer.UpdateInstanceData();  // 所有渲染器都更新
   }
   ↓
4. 所有渲染器更新完毕后，统一清除脏标记
   if (instanceData->IsDirty()) {
       instanceData->ClearDirty();  // 调用者统一清除
   }
```

---

### 2.4 架构演进历程

#### 2.4.1 重构前（2026-01-02前）

**传统继承式架构**：

| 特性 | 说明 |
|------|------|
| **接口** | IMesh抽象接口（Create、Draw） |
| **继承** | 所有几何体继承IMesh接口 |
| **职责** | 每个几何体混合了数据生成、GPU操作、渲染逻辑 |

**问题**：

| 问题 | 说明 | 影响 |
|------|------|------|
| **职责混乱** | 数据生成 + GPU操作 + 渲染逻辑混在一起 | 难以维护 |
| **代码重复** | 每个几何体都有Create和Draw方法 | 代码冗余 |
| **难以扩展** | 新增功能需修改所有几何体类 | 扩展性差 |

---

#### 2.4.2 重构后（2026-01-02）

**职责分离架构**：

| 层级 | 类名 | 职责 | 设计特点 |
|------|------|------|---------|
| **数据层** | Cube/Sphere/Torus/Plane/OBJModel | 纯静态数据生成 | 禁止实例化，线程安全 |
| **资源层** | MeshBuffer | GPU资源管理 | shared_ptr管理纹理 |
| **数据层** | InstanceData | 实例数据存储 | 脏标记优化 |
| **渲染层** | InstancedRenderer | 渲染逻辑 | 继承IRenderer接口 |

**优势**：

| 优势 | 说明 | 数据支持 |
|------|------|---------|
| **职责清晰** | 数据-资源-渲染完全分离 | 架构清晰 |
| **代码精简** | 减少冗余代码 | 代码量减少54-62% |
| **易于扩展** | 新增几何体只需提供数据 | 扩展性强 |
| **性能优化** | 脏标记、批量渲染 | 性能提升12倍 |

---

### 2.5 性能优化策略

#### 2.5.1 实例化渲染

**性能对比**：

| 渲染方式 | DrawCall次数 | 性能 |
|---------|-------------|------|
| **传统渲染** | 12辆车 × 38个材质 = 456次 | 基准 |
| **实例化渲染** | 38个材质 = 38次 | **约12倍提升** |

**实现方式**：
- 使用OpenGL实例化渲染API（`glDrawElementsInstanced`）
- 单次绘制调用渲染多个相同几何体
- 每个实例独立的变换矩阵和颜色属性

---

#### 2.5.2 批量渲染（RenderBatch）

**优化效果**：

| 场景 | 优化前 | 优化后 | 提升 |
|------|--------|--------|------|
| **12辆车 × 38材质** | 456次DrawCall | 38次DrawCall | 12倍 |
| **Disco舞台（46渲染器）** | 184次状态切换/帧 | ~60次状态切换/帧 | 减少67% |

**优化策略**：
- 按纹理分组渲染（使用`std::map`保持顺序稳定）
- 每个纹理组只绑定一次纹理
- 批量渲染同组的所有渲染器

**重载版本**：

| 参数类型 | 说明 |
|---------|------|
| `vector<InstancedRenderer*>` | 指针版本 |
| `vector<unique_ptr<InstancedRenderer>>` | unique_ptr版本 |
| `vector<InstancedRenderer>&` | 值类型版本（推荐） |

---

#### 2.5.3 脏标记机制

**性能优化效果**：

| 场景 | 优化前 | 优化后 | 提升 |
|------|--------|--------|------|
| **动画运行时** | 每帧更新GPU | 每帧更新GPU | 性能相同（数据确实在变化） |
| **动画暂停时** | 仍然每帧更新GPU | 跳过GPU更新 | 帧率+50%，GPU带宽节省100% |
| **静态几何体** | 每帧更新GPU | 完全跳过传输 | 零开销 |

**实现机制**：
- InstanceData维护脏标记（m_dirty）
- 数据修改时自动标记脏（Add、Clear、SetModelMatrix、SetColor）
- UpdateInstanceData检查脏标记，为true时才更新GPU
- 调用者负责统一清除脏标记

**自动标记脏的方法**：

| 方法 | 自动标记脏 | 说明 |
|------|-----------|------|
| `Add()` | ✅ 是 | 添加实例时 |
| `AddBatch()` | ✅ 是 | 批量添加时 |
| `Clear()` | ✅ 是 | 清除所有实例时 |
| `SetModelMatrix()` | ✅ 是 | 设置单个实例矩阵时 |
| `SetColor()` | ✅ 是 | 设置单个实例颜色时 |
| `GetModelMatrices()` | ⚠️ 否 | 返回可修改引用，需手动标记脏 |
| `GetColors()` | ⚠️ 否 | 返回可修改引用，需手动标记脏 |

---

#### 2.5.4 移动语义

**优化效果**：

| 操作 | 拷贝语义 | 移动语义 | 提升 |
|------|---------|---------|------|
| **初始化速度** | 需要拷贝数据 | 直接转移所有权 | 提升30-40% |

**支持移动语义的接口**：

| 类名 | 方法 | 移动语义支持 |
|------|------|-------------|
| **MeshData** | `SetVertices()` | ✅ 右值引用版本 |
| **MeshData** | `SetIndices()` | ✅ 右值引用版本 |
| **MeshBuffer** | `UploadToGPU()` | ✅ 右值引用版本 |
| **MeshBuffer** | 拷贝构造 | ❌ 已删除 |
| **MeshBuffer** | 移动构造 | ✅ 默认 |
| **InstancedRenderer** | 拷贝构造 | ❌ 已删除 |
| **InstancedRenderer** | 移动构造 | ✅ 默认 |

**设计原则**：
- 禁用拷贝构造和拷贝赋值（防止意外深拷贝）
- 启用移动构造和移动赋值（高效转移资源）
- 提供右值引用版本的重载方法

---

### 2.6 线程安全设计

#### 2.6.1 LightManager线程安全

**实现方式**：

| 锁类型 | 使用场景 | 并发性能 |
|--------|---------|---------|
| **std::shared_mutex** | 读写锁 | 支持多读单写 |
| **共享锁** | 读操作（ApplyToShader、GetXXX） | 允许多线程并发读 |
| **独占锁** | 写操作（Add、Remove） | 写操作互斥 |

**线程安全方法**：

| 操作类型 | 方法 | 锁类型 | 说明 |
|---------|------|--------|------|
| **读操作** | `GetPointLight()` | 共享锁 | 多线程可并发调用 |
| **读操作** | `GetPointLightCount()` | 共享锁 | 多线程可并发调用 |
| **读操作** | `ApplyToShader()` | 共享锁 | 渲染线程可并发调用 |
| **写操作** | `AddPointLight()` | 独占锁 | 写操作互斥 |
| **写操作** | `RemovePointLight()` | 独占锁 | 写操作互斥 |

**优势**：
- ✅ 支持多线程并发读（提升性能）
- ✅ 写操作互斥（保证数据一致性）
- ✅ `ApplyToShader`可在渲染线程并发调用

---

#### 2.6.2 Logger线程安全

**实现方式**：

| 组件 | 类型 | 作用 |
|------|------|------|
| **后台写入线程** | `std::thread` | 独立线程负责写入日志 |
| **消息队列** | `std::queue<LogEntry>` | 缓存日志消息 |
| **队列锁** | `std::mutex` | 保护队列访问 |
| **条件变量** | `std::condition_variable` | 唤醒写入线程 |

**工作流程**：

```
主线程调用Log()
    ↓
加锁 → 推入消息队列
    ↓
通知条件变量（notify_one）
    ↓
后台线程被唤醒
    ↓
取出消息 → 写入文件
```

**优势**：
- ✅ 异步写入，不阻塞主线程
- ✅ 支持多线程并发日志
- ✅ 条件变量唤醒，降低CPU占用

---

### 2.7 内存管理策略

#### 2.7.1 shared_ptr使用

**应用场景**：

| 资源类型 | shared_ptr使用者 | 共享场景 |
|---------|----------------|---------|
| **MeshBuffer** | InstancedRenderer | 多个渲染器共享同一网格 |
| **InstanceData** | InstancedRenderer | 多材质模型共享实例数据 |
| **Texture** | MeshBuffer | 多个网格共享同一纹理 |

**优势**：
- ✅ 自动内存管理（无需手动delete）
- ✅ 避免悬空指针（生命周期自动管理）
- ✅ 支持资源共享（避免重复加载）

**使用示例场景**：

| 场景 | 说明 |
|------|------|
| **多材质OBJ模型** | 38个材质渲染器共享同一个InstanceData |
| **多车渲染** | 12辆车共享同一个车模型MeshBuffer |
| **纹理复用** | 多个网格共享同一个纹理Texture |

---

#### 2.7.2 移动语义

**设计原则**：

| 操作 | MeshBuffer | InstancedRenderer |
|------|-----------|------------------|
| **拷贝构造** | ❌ 已删除 | ❌ 已删除 |
| **拷贝赋值** | ❌ 已删除 | ❌ 已删除 |
| **移动构造** | ✅ 默认 | ✅ 默认 |
| **移动赋值** | ✅ 默认 | ✅ 默认 |

**原因**：
- ❌ **禁止拷贝**：防止意外深拷贝（昂贵操作）和OpenGL资源双重释放
- ✅ **启用移动**：高效转移资源所有权

---

### 2.8 可扩展性设计

#### 2.8.1 新增几何体

**步骤**：

| 步骤 | 操作 | 说明 |
|------|------|------|
| **1** | 创建纯静态类 | 继承无接口，禁止实例化 |
| **2** | 实现静态方法 | `GetVertexData()`、`GetIndexData()`、`GetVertexLayout()` |
| **3** | 添加工厂方法 | 在MeshDataFactory中添加`CreateXXXData()` |
| **4** | 添加GPU工厂 | 在MeshBufferFactory中添加`CreateXXXBuffer()` |

**示例（新增Cylinder圆柱体）**：

| 类名 | 文件 | 静态方法 |
|------|------|---------|
| **Cylinder** | `Renderer/Geometry/Cylinder.hpp` | `GetVertexData(radius, height, segments)`<br>`GetIndexData(segments)`<br>`GetVertexLayout()` |

| 工厂类 | 新增方法 |
|--------|---------|
| **MeshDataFactory** | `CreateCylinderData(radius, height, segments)` |
| **MeshBufferFactory** | `CreateCylinderBuffer(radius, height, segments)` |

---

#### 2.8.2 新增渲染器

**步骤**：

| 步骤 | 操作 | 说明 |
|------|------|------|
| **1** | 继承IRenderer接口 | 实现Initialize、Render、GetName |
| **2** | 管理GPU资源 | 创建VAO/VBO/EBO等OpenGL对象 |
| **3** | 实现渲染逻辑 | 在Render方法中实现具体渲染 |

**示例（新增DeferredRenderer延迟渲染器）**：

| 方法 | 实现内容 |
|------|---------|
| `Initialize()` | 创建G-Buffer（几何缓冲区） |
| `Render()` | 几何通道 + 光照通道 |
| `GetName()` | 返回"DeferredRenderer" |

---

#### 2.8.3 新增光源类型

**步骤**：

| 步骤 | 操作 | 说明 |
|------|------|------|
| **1** | 继承Light或LightWithAttenuation | 选择合适的基类 |
| **2** | 实现纯虚函数 | GetType、ApplyToShader、GetDescription |
| **3** | 在LightManager中添加管理方法 | Add、Remove、Get |

**示例（新增AreaLight区域光）**：

| 继承关系 | 说明 |
|---------|------|
| `AreaLight : public LightWithAttenuation` | 继承带衰减的光源基类 |

| 需实现的方法 | 说明 |
|------------|------|
| `GetType()` | 返回`LightType::AREA` |
| `ApplyToShader()` | 将区域光参数传递给着色器 |
| `GetEffectiveRange()` | 重写虚函数，计算有效距离 |
| `GetDescription()` | 返回区域光描述字符串 |

| LightManager新增方法 | 说明 |
|---------------------|------|
| `AddAreaLight()` | 添加区域光 |
| `RemoveAreaLight()` | 移除区域光 |
| `GetAreaLight()` | 获取区域光 |

---

### 2.9 架构优势总结

| 特性 | 说明 | 优势 |
|------|------|------|
| **模块化** | Core/Renderer/Geometry三模块独立编译 | 降低耦合，提高内聚 |
| **职责分离** | 数据-资源-渲染完全解耦 | 易于维护和扩展 |
| **类型安全** | 编译时工厂，shared_ptr管理 | 减少运行时错误 |
| **性能优化** | 实例化渲染、批量渲染、脏标记 | 性能提升12倍 |
| **线程安全** | shared_mutex、异步日志 | 支持多线程并发 |
| **易扩展** | 纯静态几何体、IRenderer接口 | 新增功能简单 |
| **内存安全** | RAII、移动语义、智能指针 | 自动内存管理，无泄漏 |

---

*本文档基于源代码完整分析，详细描述了Lumenaris 3D渲染引擎的整体架构设计，不包含代码实现细节*
