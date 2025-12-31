# OpenGL学习项目文档

本文档目录包含了项目的完整技术文档，按照不同用途进行组织。

## 📁 文档结构

### 根目录文档

- **[ARCHITECTURE.md](ARCHITECTURE.md)** - 项目架构设计文档
  - 项目概述和目标
  - 模块化架构设计
  - 设计模式应用
  - 架构分析与优化
  - 已实现功能特性
  - 代码结构和技术特点

- **[OPTIMIZATION_GUIDE.md](OPTIMIZATION_GUIDE.md)** - 性能优化指南
  - 核心优化方向
  - 扩展方向建议
  - 实施优先级排序

### interfaces/ 接口文档目录

- **[interfaces/INTERFACES.md](interfaces/INTERFACES.md)** - 项目核心接口文档
  - Core 模块接口 (Window、MouseController、KeyboardController)
  - Renderer 模块接口 (IMesh、MeshFactory、Shader、Texture)
  - 几何体接口 (Cube、Sphere、OBJModel)
  - 使用示例和最佳实践

- **[interfaces/TINYOBJ_API_REFERENCE.md](interfaces/TINYOBJ_API_REFERENCE.md)** - TinyOBJLoader API参考
  - 第三方OBJ加载库的完整API文档
  - 数据类型和类接口说明
  - 使用指南和性能优化建议

## 📚 阅读指南

### 新开发者入门

1. **ARCHITECTURE.md** - 了解项目整体架构和设计理念
2. **interfaces/INTERFACES.md** - 学习核心类的使用方法
3. **OPTIMIZATION_GUIDE.md** - 了解性能优化方向

### 接口开发参考

- **interfaces/INTERFACES.md** - 项目内部接口规范
- **interfaces/TINYOBJ_API_REFERENCE.md** - 第三方库接口参考

### 架构优化与扩展

- **ARCHITECTURE.md** - 架构分析与优化建议
- **OPTIMIZATION_GUIDE.md** - 具体优化实施指南

## 🔗 相关链接

- [项目源码](../src/) - 源代码目录
- [着色器资源](../assets/shader/) - 着色器文件
- [构建配置](../CMakeLists.txt) - 项目构建配置

---

