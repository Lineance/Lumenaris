#version 330 core

// 顶点属性
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// 实例化属性
layout (location = 3) in mat4 aInstanceMatrix;
layout (location = 7) in vec3 aInstanceColor;

// 输出到片段着色器
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 InstanceColor;

// 变换矩阵
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // 计算世界坐标位置
    vec4 worldPos = aInstanceMatrix * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;

    // 计算法线（使用法线矩阵，正确处理非均匀缩放）
    mat3 normalMatrix = transpose(inverse(mat3(aInstanceMatrix)));
    Normal = normalize(normalMatrix * aNormal);

    // 传递纹理坐标和实例颜色
    TexCoord = aTexCoord;
    InstanceColor = aInstanceColor;

    // 应用变换矩阵
    gl_Position = projection * view * worldPos;
}
