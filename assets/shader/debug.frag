#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    // 显示法线作为颜色来调试
    vec3 norm = normalize(Normal);
    vec3 normalColor = norm * 0.5 + 0.5; // 将[-1,1]映射到[0,1]

    // 也可以显示简单的光照
    // vec3 lightDir = normalize(lightPos - FragPos);
    // float diff = max(dot(norm, lightDir), 0.0);
    // vec3 result = diff * objectColor;

    FragColor = vec4(normalColor, 1.0);
}
