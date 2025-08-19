#version 450

layout(location = 0) in vec3 fragPosWorld;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform samplerCube skyboxTex;

void main() {
  vec3 direction = normalize(fragPosWorld);
  outColor = texture(skyboxTex, direction);
  // 天空渐变：上方蓝色，下方偏白
  // vec3 skyColorTop = vec3(0.5, 0.7, 1.0);    // 蓝色
  // vec3 skyColorBottom = vec3(0.8, 0.9, 1.0); // 淡蓝白色
  
  // float t = direction.y * 0.5 + 0.5; // 将 -1,1 映射到 0,1
  // vec3 skyColor = mix(skyColorBottom, skyColorTop, t);
  
  // outColor = vec4(skyColor, 1.0);
}