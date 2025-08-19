#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragUV;

layout (location = 0) out vec4 outColor;

struct PointLight {
  vec4 position; //ignore w
  vec4 color; //w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 inverseView;
  vec4 ambientLightColor; //w is intensity 
  PointLight pointLights[10];
  int numLights;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

void main() {
  vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  vec3 specularLight = vec3(0.0);
  vec3 surfaceNormal = normalize(fragNormalWorld);

  vec3 cameraPositionWorld = ubo.inverseView[3].xyz;
  vec3 viewDirection = normalize(cameraPositionWorld - fragPosWorld);

  for(int i = 0; i < ubo.numLights; i++) {
    PointLight light = ubo.pointLights[i];
    vec3 directionToLight = light.position.xyz - fragPosWorld ;
    vec3 lightColor = light.color.xyz * light.color.w;
    float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
    directionToLight = normalize(directionToLight);

    float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
    vec3 intensity = light.color.xyz * light.color.w * attenuation;
    diffuseLight += intensity * cosAngIncidence;

    //specular light
    vec3 halfAngle = normalize(directionToLight + viewDirection);
    float blinnTerm = dot(surfaceNormal, halfAngle);
    blinnTerm = clamp(blinnTerm, 0.0, 1.0);
    blinnTerm = pow(blinnTerm, 16.0);
    specularLight += intensity * blinnTerm;
  }
	//outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
	vec4 texColor = texture(texSampler, fragUV);
  vec3 base = fragColor * texColor.rgb;     // 顶点色与纹理色混合 (可按需调整)
  vec3 lighting = diffuseLight * base + specularLight * base;
  outColor = vec4(lighting, texColor.a);
}
