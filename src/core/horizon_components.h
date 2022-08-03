#ifndef CORE_HORIZON_COMPONENTS_H
#define CORE_HORIZON_COMPONENTS_H

#include "core/horizon_camera.h"
#include "core/horizon_model.h"
#include "core/horizon_transform.h"
#include "core/horizon_controller.h"

#define MAX_POINT_LIGHTS 10

namespace horizon {

struct PointLight {
    glm::vec4 position;  // vec4 for 1 float padding, its ignored
    glm::vec4 color;     // w is intensity
    alignas(16) float radius;
};

struct UBO {
    glm::mat4 PV;
    PointLight pointLights[MAX_POINT_LIGHTS];
    alignas(16) glm::vec3 cameraPos;
    int numPointLights;
};

} // namespace horizon

#endif