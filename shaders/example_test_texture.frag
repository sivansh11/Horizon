#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

layout (push_constant) uniform Push {
    mat4 model;
}push;

layout (set = 0, binding = 0) uniform UBO {
    mat4 PV;
} ubo;

layout (set = 0, binding = 1) uniform sampler2D image;

layout (location = 0) out vec4 outFragCol;

void main() {
    outFragCol = vec4(texture(image, inUV));
}