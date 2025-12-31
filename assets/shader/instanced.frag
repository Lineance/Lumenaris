#version 330 core

// 来自顶点着色器的输入
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 InstanceColor;

// 输出颜色
out vec4 FragColor;

// 光照属性
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform bool useInstanceColor;

// 材质属性
uniform vec3 objectColor;
uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;

void main()
{
    // 环境光
    vec3 ambient = ambientStrength * lightColor;

    // 漫反射
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 镜面反射
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // 选择使用实例颜色还是物体颜色
    vec3 baseColor = useInstanceColor ? InstanceColor : objectColor;

    // 最终颜色
    vec3 result = (ambient + diffuse + specular) * baseColor;
    FragColor = vec4(result, 1.0);
}
