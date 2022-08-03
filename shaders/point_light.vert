#version 450

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout (location = 0) out vec2 outPos;

struct PointLight {
  vec4 position; // ignore w
  vec4 color; // w is intensity
  float radius;
};

layout (set = 0, binding = 0) uniform UBO {
    mat4 PV;
    PointLight pointLights[10];
    vec3 cameraPos;
    int numPointLights;
} ubo;
layout(push_constant) uniform Push {
  mat4 view;    
  PointLight pointLight;
} push;


void main() {
  outPos = OFFSETS[gl_VertexIndex];
  vec3 cameraRightWorld = {push.view[0][0], push.view[1][0], push.view[2][0]};
  vec3 cameraUpWorld = {push.view[0][1], push.view[1][1], push.view[2][1]};

  vec3 positionWorld = push.pointLight.position.xyz
    + push.pointLight.radius * outPos.x * cameraRightWorld
    + push.pointLight.radius * outPos.y * cameraUpWorld;

  gl_Position = ubo.PV * vec4(positionWorld, 1.0);
}