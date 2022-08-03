#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

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


const float M_PI = 3.1415926538;

void main() {
  float dis = sqrt(dot(fragOffset, fragOffset));
  if (dis >= 1.0) {
    discard;
  }

  float cosDis = 0.5 * (cos(dis * M_PI) + 1.0); // ranges from 1 -> 0
  outColor = vec4(push.pointLight.color.xyz + 0.5 * cosDis, cosDis);
}
