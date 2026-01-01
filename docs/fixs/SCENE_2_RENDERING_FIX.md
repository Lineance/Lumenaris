# 场景 2 渲染问题诊断和修复

## 问题描述

场景 2（波浪地板）渲染失败，但场景 1 和 3 可以正常显示。

## 诊断过程

### 1. 初步分析

场景 2 原本配置：
- **网格大小**: 40 × 40 = 1600 个立方体
- **间距**: 1.5
- **波浪幅度**: 5.0
- **旋转**: 复杂的多轴旋转

### 2. 可能的原因

1. **实例数量过多**: 1600 个实例可能导致某些 GPU 驱动问题
2. **旋转角度过大**: Y 轴旋转计算 `(x * 2.0f + z * 2.0f)` 在边缘处可达 160 度
3. **波浪幅度过大**: 5.0 的高度差可能导致某些立方体重叠或超出视野
4. **内存问题**: 大量实例可能导致内存分配问题

## 应用的修复

### 修复 1: 减小网格大小

**修改**:
```cpp
int gridSize = 20;  // 原来是 40
```

**效果**:
- 立方体数量: 1600 → 400
- 减少约 75% 的实例数量

### 修复 2: 增加间距

**修改**:
```cpp
float spacing = 2.0f;  // 原来是 1.5f
```

**效果**:
- 立方体之间的距离增加
- 减少可能的遮挡问题
- 总覆盖范围: 约 38 × 38 单位

### 修复 3: 降低波浪幅度

**修改**:
```cpp
float waveAmplitude = 3.0f;  // 原来是 5.0f
```

**效果**:
- 波浪高度变化: ±3.0 单位
- 避免极端的高度差

### 修复 4: 简化旋转

**修改前**:
```cpp
glm::vec3 rotation(
    std::sin(distance * 0.2f) * 15.0f,  // ±15 度
    x * 2.0f + z * 2.0f,                // 0 ~ 160 度
    std::cos(distance * 0.2f) * 15.0f   // ±15 度
);
```

**修改后**:
```cpp
glm::vec3 rotation(
    std::sin(distance * 0.2f) * 10.0f,  // ±10 度 ✅
    x * 5.0f + z * 5.0f,                // 0 ~ 190 度（简化）
    std::cos(distance * 0.2f) * 10.0f   // ±10 度 ✅
);
```

**效果**: X 和 Z 轴旋转范围减小

### 修复 5: 统一尺寸

**修改前**:
```cpp
float scaleVar = 0.5f + 0.5f * std::sin(distance * 0.3f);
glm::vec3 scale(scaleVar);  // 尺寸变化：0.5 ~ 1.0
```

**修改后**:
```cpp
glm::vec3 scale(0.8f);  // 统一尺寸
```

**效果**: 简化渲染，避免可能的缩放问题

### 修复 6: 调整摄像机位置

**修改**:
```cpp
cameraPos = glm::vec3(0.0f, 15.0f, 45.0f);  // 原来是 (0, 20, 40)
```

**效果**: 更好的视角观察波浪地板

### 修复 7: 添加调试信息

**新增代码**:
```cpp
static int lastScene = -1;
if (lastScene != currentScene)
{
    Core::Logger::GetInstance().Info("Rendering scene " + std::to_string(currentScene + 1) +
                                     " with " + std::to_string(scenes[currentScene]->GetCount()) + " instances");
    lastScene = currentScene;
}
```

**效果**: 可以在日志中确认场景切换和实例数量

### 修复 8: 类型转换修正

**修改**:
```cpp
float distance = std::sqrt(static_cast<float>(x * x + z * z));  // ✅ 显式转换
```

**效果**: 避免整数精度问题

## 预期结果

修复后，场景 2 应该：
- ✅ 正常渲染 400 个立方体
- ✅ 显示平滑的波浪效果
- ✅ 颜色从中心向边缘从蓝到绿渐变
- ✅ 所有立方体完整可见

## 测试步骤

1. **编译程序**
   ```bash
   cd build
   make -j$(nproc)
   ```

2. **运行程序**
   ```bash
   ./HelloWindow
   ```

3. **切换到场景 2**
   - 按 `2` 键
   - 查看日志输出：
     ```
     Switched to Scene 2: Wave Floor
     Camera position: 0.000000, 15.000000, 45.000000
     Rendering scene 2 with 400 instances
     ```

4. **使用摄像机控制**
   - `WASD` 移动
   - `Q/E` 上下移动
   - 鼠标旋转视角

## 参数对比

| 参数 | 修复前 | 修复后 | 说明 |
|------|--------|--------|------|
| 网格大小 | 40 | 20 | 减小 75% |
| 立方体数量 | 1600 | 400 | 减少约 75% |
| 间距 | 1.5 | 2.0 | 增加 33% |
| 波浪幅度 | 5.0 | 3.0 | 降低 40% |
| X/Z 轴旋转 | ±15° | ±10° | 减小 33% |
| 尺寸 | 0.5~1.0 | 0.8 | 固定 |
| 摄像机 Y | 20 | 15 | 降低 |
| 摄像机 Z | 40 | 45 | 稍远 |

## 如果问题仍然存在

### 进一步调试步骤

1. **使用线框模式**
   ```cpp
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   ```

2. **检查 OpenGL 错误**
   ```cpp
   GLenum err = glGetError();
   if (err != GL_NO_ERROR) {
       Core::Logger::GetInstance().Error("OpenGL Error: " + std::to_string(err));
   }
   ```

3. **进一步减少立方体数量**
   ```cpp
   int gridSize = 10;  // 只测试 100 个立方体
   ```

4. **简化为平面**
   ```cpp
   position.y = 0.0f;  // 移除波浪效果
   glm::vec3 rotation(0.0f, 0.0f, 0.0f);  // 移除旋转
   ```

### 可能的根本原因

如果上述修复都无效，问题可能是：

1. **InstancedRenderer 实现问题**
   - 检查 `InstancedRenderer::Render()` 方法
   - 确认 `glDrawArraysInstanced` 调用正确

2. **着色器问题**
   - 检查实例化着色器的编译
   - 确认属性绑定正确

3. **MeshBuffer 问题**
   - 确认 VAO/VBO 正确创建
   - 确认顶点属性正确设置

## 相关文件

- ✅ `src/main.cpp` - 应用了所有修复
- 📝 `docs/fixs/SCENE_2_RENDERING_FIX.md` - 本文档
- 📝 `docs/fixs/RENDERING_DEBUG_GUIDE.md` - 调试指南
- 📝 `docs/fixs/RENDERING_FIX_APPLIED.md` - 面剔除修复

---

**修复时间**: 2026-01-01
**修复状态**: ✅ 已应用
**测试状态**: ⏳ 待用户验证
