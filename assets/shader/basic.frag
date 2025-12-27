#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 viewPos; // 摄像机世界坐标，需要从C++代码传入

void main() {
    // 环境光分量：即使不被直接照亮，也有微弱的光
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * objectColor;

    // 漫反射光照分量：塑造立体感的核心
    vec3 norm = normalize(Normal); // 归一化法线
    vec3 lightDir = normalize(lightPos - FragPos); // 计算光源方向（世界空间）
    float diff = max(dot(norm, lightDir), 0.0); // 点乘计算强度
    vec3 diffuse = diff * objectColor;

    // 组合最终颜色
    vec3 result = (ambient + diffuse);
    FragColor = vec4(result, 1.0);
}