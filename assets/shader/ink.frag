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

    // 水墨效果：光照越强，墨色越淡
    float inkIntensity = 1.0 - diff;

    // 添加一些噪波让水墨效果更自然
    vec3 inkColor = vec3(0.1, 0.05, 0.02); // 深褐色墨色

    // 墨色浓度：从淡到浓的渐变
    float concentration = smoothstep(0.0, 1.0, inkIntensity);

    // 基础墨色
    vec3 baseInk = mix(vec3(0.9, 0.85, 0.8), inkColor, concentration);

    // 添加镜面高光（水墨中的亮斑）
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);

    // 高光区域变亮（墨色变淡）
    float highlight = smoothstep(0.9, 1.0, spec);
    baseInk = mix(baseInk, vec3(0.95, 0.9, 0.85), highlight * 0.5);

    // 添加边缘墨色加深效果
    float fresnel = 1.0 - max(dot(viewDir, norm), 0.0);
    float edgeDarkening = smoothstep(0.7, 1.0, fresnel);
    baseInk = mix(baseInk, inkColor * 0.5, edgeDarkening * 0.3);

    FragColor = vec4(baseInk, 1.0);
}
