# 编译指南 (Build Guide)

本文档提供 LearningOpenGL 项目的完整编译配置和优化指南。

## 目录

- [编译模式](#编译模式)
- [快速开始](#快速开始)
- [详细配置](#详细配置)
- [性能优化](#性能优化)
- [故障排除](#故障排除)

---

## 编译模式

### Debug 模式（默认）

**特点**：
- 包含完整调试符号
- 启用所有日志（DEBUG、INFO、WARNING、ERROR）
- 启用性能统计日志
- 无编译优化（`-O0`）
- 适合开发和调试

**配置**：
```cmake
cmake -B build -DCMAKE_BUILD_TYPE=Debug
```

### Release 模式

**特点**：
- 最小调试符号
- 仅启用重要日志（INFO、WARNING、ERROR）
- **禁用性能统计日志**（`ENABLE_RENDER_STATS=0`）
- 最高优化级别（`-O3`）
- 适合性能测试和生产环境

**配置**：
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
```

---

## 快速开始

### 1. 安装依赖

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libglfw3-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libglm-dev \
    libstb-dev
```

#### macOS
```bash
brew install cmake glfw glm stb
```

#### Windows (WSL)
参考 Ubuntu/Debian 安装方法。

### 2. 编译项目

#### Debug 模式（开发）
```bash
cd /mnt/d/Code/LearningOpenGL
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

#### Release 模式（性能优化）
```bash
cd /mnt/d/Code/LearningOpenGL
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### 3. 运行程序

```bash
# Debug 构建
./build/bin/LearningOpenGL

# Release 构建
./build/bin/LearningOpenGL
```

---

## 详细配置

### 编译选项说明

#### CMAKE_BUILD_TYPE

| 值 | 优化级别 | 调试符号 | 适用场景 |
|---|---|---|---|
| `Debug` | `-O0` | 完整 | 开发、调试 |
| `Release` | `-O3` | 最小 | 性能测试、发布 |
| `RelWithDebInfo` | `-O2` | 完整 | 性能分析 |
| `MinSizeRel` | `-Os` | 最小 | 嵌入式系统 |

#### 条件编译宏

项目支持以下编译时宏（在 `include/Core/Logger.hpp` 中定义）：

```cpp
// 1. DEBUG 日志控制
#if defined(NDEBUG) || defined(FORCE_RELEASE_MODE)
#define LOG_DEBUG_ENABLED 0    // Release 模式禁用 DEBUG 日志
#else
#define LOG_DEBUG_ENABLED 1    // Debug 模式启用 DEBUG 日志
#endif

// 2. 性能统计日志控制
#ifndef ENABLE_RENDER_STATS
#define ENABLE_RENDER_STATS 0  // 默认禁用（热路径日志）
#endif
```

### 自定义编译配置

#### 方案 1：标准 Release 构建
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**效果**：
- `LOG_DEBUG_ENABLED = 0`（自动）
- `ENABLE_RENDER_STATS = 0`（默认）
- 优化级别：`-O3`
- **预期性能提升**：~5-10%

#### 方案 2：Release + 性能统计
如果需要保留性能统计（用于性能分析）：

修改 `include/Core/Logger.hpp`：
```cpp
#define ENABLE_RENDER_STATS 1  // 启用性能统计
```

然后编译：
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**效果**：
- `LOG_DEBUG_ENABLED = 0`
- `ENABLE_RENDER_STATS = 1`
- 可以分析 DrawCall、三角形数量等统计信息

#### 方案 3：强制 Release 模式
创建 `cmake/ForceRelease.cmake`：
```cmake
# 强制禁用 DEBUG 日志
add_definitions(-DFORCE_RELEASE_MODE)

# 确保禁用性能统计
add_definitions(-DENABLE_RENDER_STATS=0)
```

修改 `CMakeLists.txt`：
```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    include(cmake/ForceRelease.cmake)
endif()
```

---

## 性能优化

### 已实施的优化

本项目已实现两个关键性能优化（详见 `docs/PERFORMANCE_ANALYSIS.md`）：

#### 优化 1：移动语义传递 InstanceData
**问题**：拷贝 `InstanceData` 导致大量内存复制

**解决方案**：
```cpp
// 添加移动语义版本
void SetInstances(InstanceData&& data) {
    m_instances = std::move(data);  // 零拷贝
    m_instanceCount = m_instances.GetCount();
}

// 使用时
cubeRenderer.SetInstances(std::move(cubeInstances));  // 移动而非拷贝
```

**预期提升**：首次创建时 ~5-10%

#### 优化 2：条件编译关闭日志
**问题**：热路径中频繁的日志调用影响性能

**解决方案**：
```cpp
void Draw() const {
    // ... 渲染代码
#if ENABLE_RENDER_STATS
    Core::Logger::GetInstance().LogDrawCall(triangleCount);
#endif
}
```

**预期提升**：~3-8%（取决于日志频率）

### 性能对比

| 配置 | 帧率 (FPS) | 说明 |
|---|---|---|
| 旧架构 | ~900 | 原始 InstancedMesh |
| 新架构（优化前） | ~870 | 职责分离设计 |
| 新架构（修复双VAO） | ~900 | 修复拷贝构造函数 |
| 新架构（优化1+2） | ~900-920 | 移动语义 + 条件编译 |

**注意**：不推荐使用原始指针缓存优化，因为：
- 增加代码复杂度
- 容易引入生命周期bug
- 违反现代C++最佳实践
- 性能提升微小（<5%），得不偿失

*测试场景：10x10 立方体 + 12 车辆（38 材质组）*

---

## 故障排除

### 问题 1：编译错误 - 找不到 GLFW

**错误信息**：
```
fatal error: GLFW/glfw3.h: No such file or directory
```

**解决方案**：
```bash
# Ubuntu/Debian
sudo apt-get install libglfw3-dev

# macOS
brew install glfw
```

### 问题 2：链接错误 - undefined reference

**错误信息**：
```
undefined reference to `glGenVertexArrays'
```

**解决方案**：
确保 CMakeLists.txt 正确链接 OpenGL：
```cmake
find_package(OpenGL REQUIRED)
target_link_libraries(LearningOpenGL OpenGL::GL GLFW GLM::GLM)
```

### 问题 3：运行时闪退

**可能原因**：
1. 着色器文件路径错误
2. 模型文件缺失
3. OpenGL 上下文未初始化

**解决方案**：
```bash
# 检查日志
cat logs/instanced_rendering.log

# 使用 Debug 模式重新编译
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
gdb ./build/bin/LearningOpenGL
(gdb) run
```

### 问题 4：性能低于预期

**检查清单**：
1. 确认使用 Release 模式
2. 检查 `ENABLE_RENDER_STATS` 是否为 0
3. 查看日志中的性能统计
4. 验证 GPU 驱动是否最新

**验证脚本**：
```bash
# 编译 Release 版本
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 运行并检查 FPS
./build/bin/LearningOpenGL
```

---

## 高级配置

### 启用链接时优化 (LTO)

修改 `CMakeLists.txt`：
```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result OUTPUT output)
    if(result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()
```

**预期提升**：~3-5%

### 启用本地符号

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(LearningOpenGL PRIVATE
        -fvisibility=hidden  # GCC/Clang
    )
endif()
```

**预期提升**：~1-2%

### 使用静态链接（减少依赖）

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_options(LearningOpenGL PRIVATE
        -static-libgcc
        -static-libstdc++
    )
endif()
```

---

## 开发工作流

### 日常开发（Debug 模式）
```bash
# 快速迭代
cmake --build build
./build/bin/LearningOpenGL
```

### 性能测试（Release 模式）
```bash
# 完整优化编译
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 运行性能测试
./build/bin/LearningOpenGL

# 查看日志
tail -f logs/instanced_rendering.log
```

### 性能分析
如果需要分析性能瓶颈：

1. 启用性能统计：
```cpp
// include/Core/Logger.hpp
#define ENABLE_RENDER_STATS 1
```

2. 使用 `RelWithDebInfo` 构建：
```bash
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build --config RelWithDebInfo
```

3. 使用性能分析工具：
```bash
# Linux perf
perf record ./build/bin/LearningOpenGL
perf report

# 或使用 gprof
gprof ./build/bin/LearningOpenGL gmon.out > analysis.txt
```

---

## 总结

### 推荐配置

**开发阶段**：
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

**性能测试**：
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/bin/LearningOpenGL
```

**生产发布**：
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
# 确保 ENABLE_RENDER_STATS=0
```

### 关键要点

1. **始终使用 Release 模式进行性能测试**
2. **开发环境使用 Debug 模式便于调试**
3. **条件编译宏可在 Logger.hpp 中调整**
4. **已启用优化：移动语义 + 条件编译日志**
5. **使用 shared_ptr 符合现代C++最佳实践，不推荐过度优化**

---

## 相关文档

- [架构文档](ARCHITECTURE.md) - 系统架构设计
- [性能分析](PERFORMANCE_ANALYSIS.md) - 详细性能优化说明
- [接口文档](interfaces/INTERFACES.md) - API 接口说明
