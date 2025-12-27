#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 lightPos;
uniform float grainIntensity;
uniform float threshold;
uniform float pixelBlockSize; // ✅ 新增：像素块大小（像素单位）

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

void main() {
    // 1. 标准光照计算亮度
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    float brightness = diff * 0.7 + 0.3;

    // ✅ 2. 强像素化：将屏幕划分为离散块
    //    每个块内的所有片段使用相同的噪声值
    vec2 screenUV = gl_FragCoord.xy / pixelBlockSize;
    vec2 blockCoord = floor(screenUV);           // 块坐标（整数）
    vec2 blockCenter = (blockCoord + 0.5) * pixelBlockSize; // 块中心（用于噪声种子）
    
    // ✅ 3. 基于块坐标生成噪声（确保块内一致）
    float grain = random(blockCoord * 0.1);      // 缩放避免重复
    grain = mix(brightness, grain, grainIntensity);

    // ✅ 4. 二值化（阈值0.5）
    float binary = (grain > threshold) ? 1.0 : 0.0;
    
    FragColor = vec4(binary * objectColor, 1.0);
}