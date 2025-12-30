# OpenGL学习项目构建配置
# 支持网格渲染、OBJ模型加载、GLM数学库封装

cmake_minimum_required(VERSION 3.15)
project(DarkRoomEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. 查找系统包
find_package(OpenGL REQUIRED)

# 3. 定义 Core 库
add_library(Core STATIC
    src/Core/Window.cpp
    src/Core/MouseController.cpp
    src/Core/KeyboardController.cpp
)
target_include_directories(Core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm
)

# 4. 关键：为 Core 库链接 GLFW 和系统库
if(WIN32)
    # 方式A: 链接项目vendor目录下的GLFW（优先确认库文件存在且与编译器兼容）
    target_link_libraries(Core PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/lib/libglfw3.a
        opengl32.lib
        gdi32.lib
    )
else()
    # 方式B: 适用于Linux/macOS，尝试查找系统安装的GLFW
    find_package(glfw3 QUIET)
    if(glfw3_FOUND)
        target_link_libraries(Core PUBLIC glfw)
    else()
        # 备用方案：链接可能的本地安装或特定路径的GLFW
        target_link_libraries(Core PUBLIC glfw3)
    endif()
    target_link_libraries(Core PUBLIC OpenGL::GL)
endif()

# 5. 定义 Renderer 库
add_library(Renderer STATIC
    src/Renderer/Shader.cpp
)
target_include_directories(Renderer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm
)
target_link_libraries(Renderer PUBLIC Core) # Renderer 依赖 Core

# 网格库 - 支持基础网格和OBJ模型加载
add_library(Geometry OBJECT
    src/Renderer/Mesh.cpp
    src/Renderer/Cube.cpp
    src/Renderer/Sphere.cpp
    src/Renderer/OBJLoader.cpp    # OBJ文件解析器
    src/Renderer/OBJModel.cpp     # OBJ模型渲染器
)

target_include_directories(Geometry PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/vendor/glm
)

target_link_libraries(Geometry PRIVATE Renderer Core)

# 6. 定义主程序 - 3D渲染演示应用
add_executable(HelloWindow
    src/main.cpp
    src/glad.c
    $<TARGET_OBJECTS:Geometry>
)
target_include_directories(HelloWindow PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(HelloWindow PRIVATE Core Renderer OpenGL::GL)

# 项目特性：
# - 3D网格渲染 (Cube, Sphere, OBJ模型)
# - 实时着色器切换 (Phong, Toon, Glass, etc.)
# - 3D相机控制 (WASD移动 + 鼠标旋转)
# - GLM数学库统一封装
# - 模块化架构设计
