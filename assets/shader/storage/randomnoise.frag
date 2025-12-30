#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 lightPos;
uniform float grainSize;
uniform float grainIntensity;
uniform float blurAmount;
uniform float threshold;

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a)* u.y * (1.0 - u.x) + (d - b)* u.y * u.x;
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    float brightness = diff * 0.7 + 0.3;

    vec2 grainUV = FragPos.xy / grainSize;
    float grain = noise(grainUV * 1.0) * 0.5;
    grain += noise(grainUV * 2.0) * 0.25;
    grain += noise(grainUV * 4.0) * 0.125;
    grain = mix(brightness, grain, grainIntensity);

    float blur = 0.0;
    if(blurAmount > 0.0) {
        for(float x = -1.0; x <= 1.0; x += 1.0) {
            for(float y = -1.0; y <= 1.0; y += 1.0) {
                blur += noise(grainUV + vec2(x,y) * blurAmount * 0.1);
            }
        }
        blur /= 9.0;
    }

    float final = mix(grain, blur, blurAmount);
    float binary = (final > threshold) ? 1.0 : 0.0;
    
    vec3 result = binary * objectColor;
    FragColor = vec4(result, 1.0);
}