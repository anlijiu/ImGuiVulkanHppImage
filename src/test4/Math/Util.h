#pragma once

#include <span>

#include <glm/vec3.hpp>

#include "Math/Sphere.h"

namespace util
{
math::Sphere calculateBoundingSphere(std::span<glm::vec3> positions);
glm::vec3 smoothDamp(
    const glm::vec3& current,
    glm::vec3 target,
    glm::vec3& currentVelocity,
    float smoothTime,
    float dt,
    float maxSpeed = std::numeric_limits<float>::max());
}
