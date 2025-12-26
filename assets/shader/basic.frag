#version 330 core
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightPos;

void main() {
    // 简化光照计算
    float dist = length(lightPos - gl_FragCoord.xyz);
    float attenuation = 1.0 / (1.0 + 0.1 * dist * dist);
    FragColor = vec4(objectColor * attenuation, 1.0);
}