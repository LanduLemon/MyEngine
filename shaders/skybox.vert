#version 450

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 fragPosWorld;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;
  vec4 ambientLightColor;
  vec3 lightPosition;
  vec4 lightColor;
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

void main() {
  vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
  fragPosWorld = positionWorld.xyz;
  
  // 移除view矩阵的平移部分，保持天空盒始终居中
  mat4 viewNoTranslation = mat4(mat3(ubo.view));
  gl_Position = ubo.projection * viewNoTranslation * positionWorld;
  
  // 确保天空盒在最远处
  gl_Position = gl_Position.xyww;
}