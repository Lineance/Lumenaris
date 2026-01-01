#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 InstanceColor;

out vec4 FragColor;

uniform bool useInstanceColor;

void main()
{
    // 根据设置输出颜色
    vec3 color;
    if (useInstanceColor)
    {
        color = InstanceColor;  // 使用实例颜色
    }
    else
    {
        color = vec3(1.0, 0.0, 0.0);  // 默认红色
    }

    // 加上简单的光照效果
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(normalize(Normal), lightDir), 0.3);
    color = color * (0.3 + 0.7 * diff);

    FragColor = vec4(color, 1.0);
}
