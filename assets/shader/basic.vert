#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord; // 纹理坐标

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;     // 片段的世界坐标
out vec3 Normal;      // 片段的世界空间法线
out vec2 TexCoord;    // 传递纹理坐标

void main() {
    // 计算顶点在世界空间中的位置
    FragPos = vec3(model * vec4(aPos, 1.0));
    // 计算世界空间下的法线（考虑模型旋转和缩放，但不考虑位移）
    Normal = mat3(transpose(inverse(model))) * aNormal;
    // 传递纹理坐标
    TexCoord = aTexCoord;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}