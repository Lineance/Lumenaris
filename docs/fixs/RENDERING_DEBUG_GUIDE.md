# 渲染问题调试指南

## 问题描述

场景 2（波浪地板）渲染失败，场景 1 和 3 有类似问题但正方形不完整。

## 可能原因分析

### 1. 面剔除问题
当前代码启用了面剔除：
```cpp
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);
glFrontFace(GL_CCW);
```

如果立方体的顶点顺序（缠绕顺序）不正确，会导致某些面被剔除。

### 2. 深度测试问题
深度测试可能导致某些面被错误遮挡。

### 3. 摄像机位置
摄像机位置可能在立方体内部或太近。

### 4. 顶点数量计算
立方体有 36 个顶点（6 个面 × 每面 6 个顶点），没有索引。

## 调试步骤

### 步骤 1: 禁用面剔除

修改 `main.cpp` 中的 OpenGL 设置：

```cpp
// 查找这一行（约第 403 行）
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);
glFrontFace(GL_CCW);

// 临时禁用面剔除
// glEnable(GL_CULL_FACE);
// glCullFace(GL_BACK);
// glFrontFace(GL_CCW);
```

### 步骤 2: 调整摄像机位置

确保摄像机不会进入立方体内部：

```cpp
// 场景 2 的摄像机位置
cameraPos = glm::vec3(0.0f, 20.0f, 60.0f);  // 增加距离
```

### 步骤 3: 检查日志输出

运行程序后查看日志，确认：
- 立方体顶点数量应该是 36
- 实例数量是否正确（场景 2 应该是 1600）

### 步骤 4: 调整光照

光照太强或太弱可能导致看不清立方体：

```cpp
instancedShader.SetFloat("ambientStrength", 0.5f);  // 增加环境光
```

### 步骤 5: 使用简单的纯色渲染

临时修改着色器使用纯色，排除光照问题：

```cpp
// 在片段着色器中临时添加
void main()
{
    // 临时使用纯色测试
    FragColor = vec4(InstanceColor, 1.0f);
    return;

    // 原有的光照计算...
}
```

## 快速修复方案

### 方案 1: 完全禁用面剔除（推荐）

```cpp
// src/main.cpp 第 401-404 行
// 注释掉面剔除
// glEnable(GL_CULL_FACE);
// glCullFace(GL_BACK);
// glFrontFace(GL_CCW);
```

### 方案 2: 调整场景参数

```cpp
// 场景 2: 波浪地板
int gridSize = 40;           // 改为 20 测试
float spacing = 1.5f;        // 改为 2.0 增加间距
float waveAmplitude = 5.0f;  // 改为 2.0 降低波浪高度
```

### 方案 3: 增加环境光

```cpp
instancedShader.SetFloat("ambientStrength", 0.6f);  // 原来是 0.3f
```

## 立方体顶点数据验证

立方体的 `GetVertexData()` 返回：
- 36 个顶点
- 每个顶点 8 个 float（位置 3 + 法线 3 + UV 2）
- 总共 288 个 float
- 顶点数量 = 288 / 8 = 36 ✓

每个面有 6 个顶点（2 个三角形），共 6 个面：
```
前面: 6 个顶点 (索引 0-5)
后面: 6 个顶点 (索引 6-11)
左面: 6 个顶点 (索引 12-17)
右面: 6 个顶点 (索引 18-23)
下面: 6 个顶点 (索引 24-29)
上面: 6 个顶点 (索引 30-35)
```

## 预期结果

修复后，每个场景应该显示：
- **场景 1**: 完整的螺旋塔，每个立方体 6 个面都可见
- **场景 2**: 完整的波浪地板，所有立方体都正确渲染
- **场景 3**: 完整的浮动群岛，所有立方体 6 个面都可见

## 如果问题仍然存在

1. 检查 OpenGL 错误：
```cpp
GLenum err = glGetError();
if (err != GL_NO_ERROR) {
    Core::Logger::GetInstance().Error("OpenGL Error: " + std::to_string(err));
}
```

2. 使用线框模式查看拓扑结构：
```cpp
glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
```

3. 减少实例数量进行测试：
```cpp
int gridSize = 5;  // 只渲染 5x5=25 个立方体
```

---

**创建时间**: 2026-01-01
**问题状态**: 🔍 调试中
