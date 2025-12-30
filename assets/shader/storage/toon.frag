#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    // 漫反射强度
    float diff = max(dot(norm, lightDir), 0.0);

    // 卡通色阶化 (4个色阶)
    float toonLevels = 4.0;
    float toonDiff = floor(diff * toonLevels) / toonLevels;

    // 镜面反射 (卡通风格)
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
    float toonSpec = step(0.8, spec); // 镜面高光只有亮或灭

    // 环境光
    vec3 ambient = 0.3 * objectColor;

    // 卡通漫反射
    vec3 diffuse = toonDiff * objectColor * lightColor;

    // 镜面高光
    vec3 specular = toonSpec * lightColor * 0.8;

    // 最终颜色
    vec3 result = ambient + diffuse + specular;
    result = clamp(result, 0.0, 1.0);

    // 添加轮廓线效果 (简单的黑边)
    float edge = 1.0 - dot(viewDir, norm);
    float outline = step(0.4, edge);

    // 如果是轮廓，绘制黑色，否则绘制卡通色
    FragColor = vec4(mix(result, vec3(0.0), outline), 1.0);
}
