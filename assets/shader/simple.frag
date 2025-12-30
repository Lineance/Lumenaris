#version 330 core
out vec4 FragColor;

uniform vec3 objectColor;

void main() {
    // 简单的纯色输出，用于测试渲染管线
    FragColor = vec4(objectColor, 1.0);
}
