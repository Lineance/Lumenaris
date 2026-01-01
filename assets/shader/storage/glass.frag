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
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(lightPos - FragPos);

    // 菲涅耳效应：边缘更亮
    float fresnel = pow(1.0 - max(dot(viewDir, norm), 0.0), 2.0);

    // 反射方向
    vec3 reflectDir = reflect(-viewDir, norm);

    // 模拟环境反射（简单的天空色渐变）
    vec3 skyColor = vec3(0.5, 0.7, 1.0); // 天蓝色
    vec3 groundColor = vec3(0.3, 0.3, 0.3); // 地面灰色
    float reflectY = reflectDir.y * 0.5 + 0.5; // 从-1,1映射到0,1
    vec3 reflectionColor = mix(groundColor, skyColor, reflectY);

    // 玻璃的基础颜色（浅蓝色）
    vec3 glassColor = vec3(0.7, 0.9, 1.0) * 0.3;

    // 漫反射（玻璃对光有轻微的散射）
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * 0.2;

    // 镜面反射（玻璃的高光）
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor * 0.8;

    // 玻璃效果：混合反射和透射
    vec3 glassReflection = mix(glassColor, reflectionColor, fresnel * 0.8);
    vec3 finalColor = glassReflection + diffuse + specular;

    // Gamma校正 - 将颜色从线性空间转换到sRGB空间
    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    // 玻璃的透明度
    float alpha = 0.7 + fresnel * 0.3; // 边缘更透明

    FragColor = vec4(finalColor, alpha);
}
