#version 450

layout (location = 0) in vec3 inCol;
layout (location = 1) in vec3 inPos;
layout (location = 2) in vec3 inNor;

layout (location = 0) out vec4 outColor;

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

void main() 
{
    vec3 diffuseLight = vec3(.005, 0.005, 0.005);
    vec3 specularLight = vec3(0);
    vec3 surfaceNormal = normalize(inNor);
    
    vec3 cameraPos = ubo.cameraPos;
    vec3 viewDir = normalize(cameraPos - inPos);

    for (int i = 0; i < ubo.numPointLights; i++) {
        PointLight pointLight = ubo.pointLights[i];
        vec3 dirToLight = pointLight.position.xyz - inPos;
        float attenuation = 1.0 / dot(dirToLight, dirToLight);
        dirToLight = normalize(dirToLight);

        float cosAngInc = max(dot(surfaceNormal, dirToLight), 0);
        vec3 intensity = pointLight.color.xyz * pointLight.color.w * attenuation;

        diffuseLight += intensity * cosAngInc;

        vec3 halfAng = normalize(dirToLight + viewDir);
        float blinnTerm = dot(surfaceNormal, halfAng);
        blinnTerm = clamp(blinnTerm,0,1);
        blinnTerm = pow(blinnTerm, 32);
        specularLight += intensity * blinnTerm;
    }
    outColor = vec4(diffuseLight * inCol + specularLight * inCol, 1);
}