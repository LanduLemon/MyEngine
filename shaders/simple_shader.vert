#version 450

layout(location =0) in vec3 position;
layout(location =1) in vec3 color;
layout(location =2) in vec3 normal;
layout(location =3) in vec2 uv;

layout(location =0) out vec3 fragColor;

layout(push_constant) uniform Push{
	mat4 transform; //projectio * view *model 
	mat4 normalMatrix;
} push;

const vec3 DIRECTION_TO_LIGHTS = normalize(vec3(1.0, -3.0, -1.0));
const float AMBIENT = 0.02f;

void main(){
	gl_Position = push.transform * vec4(position, 1.0);

	//this is only correct in certain situtations
	//mat normalMatrix = transpose(inverse(push.transform));
	vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

	float lightIntensity = AMBIENT + max(dot(normalWorldSpace, DIRECTION_TO_LIGHTS),0);
	fragColor = lightIntensity * color;
}
