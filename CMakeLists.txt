cmake_minimum_required(VERSION 3.15)
project(DarkRoomEngine)

set(CMAKE_CXX_STANDARD 17)

# 创建 Core 静态库
add_library(Core STATIC
    src/Core/Window.cpp
)

# 为 Core 库设置头文件路径
target_include_directories(Core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/vendor/glfw/include
)

# 创建可执行文件
add_executable(HelloWindow src/main.cpp src/glad.c)

# 为可执行文件设置头文件路径
target_include_directories(HelloWindow PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# 平台相关配置：重点修改 Windows 部分
if(WIN32)
    # 假设库文件已放置在 vendor/glfw/lib/ 下
    # 关键修改：链接具体的 .a 库文件，注意名称可能不同
    target_link_libraries(HelloWindow PRIVATE
        Core
        ${CMAKE_SOURCE_DIR}/vendor/glfw/lib/libglfw3.a  # 或 libglfw3dll.a
        opengl32.lib
        gdi32.lib # MinGW 链接 GLFW 有时需要额外链接 gdi32
    )
elseif(APPLE)
    find_package(OpenGL REQUIRED)
    find_package(glfw3 REQUIRED)
    target_link_libraries(HelloWindow PRIVATE Core glfw OpenGL::GL)
else()
    find_package(OpenGL REQUIRED)
    find_package(glfw3 REQUIRED)
    target_link_libraries(HelloWindow PRIVATE Core glfw OpenGL::GL)
endif()