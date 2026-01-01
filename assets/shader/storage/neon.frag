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

    // 菲涅耳效果 - 边缘发光
    float fresnel = pow(1.0 - max(dot(viewDir, norm), 0.0), 2.0);

    // 漫反射强度
    float diff = max(dot(norm, lightDir), 0.0);

    // 霓虹的核心发光
    vec3 neonGlow = objectColor * 2.0; // 增强亮度

    // 边缘发光强度
    float edgeGlow = smoothstep(0.0, 1.0, fresnel);

    // 主体区域的暗色
    vec3 darkBase = objectColor * 0.1; // 很暗的基色

    // 漫反射区域的霓虹色
    vec3 diffuseGlow = objectColor * diff * 1.5;

    // 镜面高光（霓虹管的高亮）
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specularGlow = objectColor * spec * 3.0;

    // 合成霓虹效果
    vec3 finalColor = darkBase + diffuseGlow + specularGlow;

    // 添加边缘发光
    finalColor += neonGlow * edgeGlow * 2.0;

    // 确保颜色不会过亮
    finalColor = clamp(finalColor, 0.0, 1.0);

    // Gamma校正 - 将颜色从线性空间转换到sRGB空间
    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    // 霓虹通常不透明，但有轻微的透明度变化
    float alpha = 0.9 + edgeGlow * 0.1;

    FragColor = vec4(finalColor, alpha);
}
