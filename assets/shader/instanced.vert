#version 330 core

// 顶点属性
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// 实例化属性
layout (location = 3) in mat4 aInstanceMatrix;
layout (location = 7) in vec3 aInstanceColor;

// Uniforms
uniform mat4 projection;
uniform mat4 view;

// 输出到片段着色器
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 InstanceColor;

void main()
{
    // 计算最终的位置：投影 * 视图 * 实例矩阵 * 顶点位置
    mat4 model = aInstanceMatrix;
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // 传递数据到片段着色器
    FragPos = vec3(model * vec4(aPos, 1.0));

    // 计算法线矩阵（用于正确变换法线）
    Normal = mat3(transpose(inverse(model))) * aNormal;

    TexCoord = aTexCoord;
    InstanceColor = aInstanceColor;
}
