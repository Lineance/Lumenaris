# 主程序编译错误修复记录

## 错误信息

```
D:/Code/LearningOpenGL/src/main.cpp:307:25: error: 'class Core::MouseController' has no member named 'CaptureMouse'
  307 |         mouseController.CaptureMouse();
      |                         ^~~~~~~~~~~~
```

以及 Lambda 捕获全局变量的警告：
```
warning: capture of variable 'cameraPos' with non-automatic storage duration
```

## 修复方案

### 1. 修复 MouseController 方法调用

**错误代码**:
```cpp
mouseController.CaptureMouse();
```

**修复后**:
```cpp
mouseController.SetMouseCapture(true);
```

**原因**: `MouseController` 类没有 `CaptureMouse()` 方法，正确的方法是 `SetMouseCapture(bool)`。

### 2. 修复 Lambda 捕获警告

**错误代码**:
```cpp
keyboardController.RegisterKeyCallback(GLFW_KEY_1, [&currentScene, &cameraPos]()
{
    currentScene = 0;
    cameraPos = glm::vec3(0.0f, 20.0f, 50.0f);
    // ...
});
```

**修复后**:
```cpp
keyboardController.RegisterKeyCallback(GLFW_KEY_1, [&currentScene]()
{
    currentScene = 0;
    cameraPos = glm::vec3(0.0f, 20.0f, 50.0f);  // 直接访问全局变量
    // ...
});
```

**原因**: `cameraPos` 是全局变量，不需要捕获。Lambda 可以直接访问全局变量，捕获它会引发警告。

## 修改文件

- `src/main.cpp` - 第 307 行和第 365-384 行

## 验证

修复后，编译应该成功通过，没有错误和警告。

```bash
cd build
make -j$(nproc)
```

## 总结

这两个都是简单的 API 使用错误：

1. 方法名称错误：查阅 `MouseController.hpp` 确认正确的 API
2. 不必要的捕获：全局变量不需要捕获，直接访问即可

**修复时间**: 2026-01-01
**修复状态**: ✅ 完成
