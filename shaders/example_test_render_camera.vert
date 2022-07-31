#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inCol;
layout (location = 2) in vec3 inNor;
layout (location = 3) in vec2 inUV;

layout (push_constant) uniform Push {
    mat4 PVM;
}push;

layout(location = 0) out vec3 fragColor;

void main()
{
    gl_Position = push.PVM * vec4(inPos, 1.0);
    fragColor = inCol;
}