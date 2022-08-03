#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inCol;
layout (location = 2) in vec3 inNor;
layout (location = 3) in vec2 inUV;

struct PointLight {
  vec4 position; // ignore w
  vec4 color; // w is intensity
  float radius;
};


layout (push_constant) uniform Push {
    mat4 model;
}push;

layout (set = 0, binding = 0) uniform UBO {
    mat4 PV;
    PointLight pointLights[10];
    vec3 cameraPos;
    int numPointLights;
} ubo;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outPos;
layout(location = 2) out vec3 outNor;

void main()
{
    vec4 pos = push.model * vec4(inPos, 1.0);
    gl_Position = ubo.PV * pos;
    outColor = inCol;
    outPos = pos.xyz;
    outNor = mat3(transpose(inverse(push.model))) * inNor;
}