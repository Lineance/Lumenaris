#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float shininess; // 光泽度

// 纹理支持
uniform sampler2D diffuseTexture;
uniform bool useTexture;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    // 获取基础颜色（材质颜色或纹理颜色）
    vec3 baseColor;
    if (useTexture) {
        baseColor = texture(diffuseTexture, TexCoord).rgb;
    } else {
        baseColor = objectColor;
    }

    // 对暗的材质颜色进行亮度补偿
    float brightness = dot(baseColor, vec3(0.299, 0.587, 0.114)); // 计算亮度
    if (brightness < 0.3) {
        // 如果颜色太暗，进行亮度增强
        float factor = 0.3 / max(brightness, 0.01);
        baseColor *= min(factor, 3.0); // 最多增强3倍
    }

    // 环境光 - 增加强度以补偿暗的材质颜色
    float ambientStrength = 0.8;
    vec3 ambient = ambientStrength * baseColor * lightColor;

    // 漫反射 - 增加强度
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * 1.2 * baseColor * lightColor;

    // 镜面反射
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * lightColor * 0.5;

    vec3 result = (ambient + diffuse + specular);

    result = clamp(result, 0.0, 1.0);

    // Gamma校正 - 将颜色从线性空间转换到sRGB空间
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, 1.0);
}