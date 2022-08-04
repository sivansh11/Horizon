#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inCol;
layout (location = 2) in vec3 inNor;
layout (location = 3) in vec2 inUV;

layout (location = 0) out vec3 outPos;
layout (location = 1) out vec2 outUV;

layout (push_constant) uniform Push {
    mat4 model;
}push;

layout (set = 0, binding = 0) uniform UBO {
    mat4 PV;
} ubo;


void main() {
    outPos  = vec3((push.model * vec4(inPos, 1)));
    outUV = inUV;
    gl_Position = ubo.PV * vec4(outPos, 1);
}

