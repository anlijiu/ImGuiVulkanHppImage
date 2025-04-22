#pragma once

#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include <string>

#include <glm/vec4.hpp>

#include "Color.h"
#include "IdTypes.h"

struct MaterialData {
    LinearColor baseColor;
    glm::vec4 metalRoughnessEmissive;
    std::uint32_t diffuseTex;
    std::uint32_t normalTex;
    std::uint32_t metallicRoughnessTex;
    std::uint32_t emissiveTex;
};

struct Material {
    LinearColor baseColor{1.f, 1.f, 1.f, 1.f};
    float metallicFactor{0.f};
    float roughnessFactor{0.7f};
    float emissiveFactor{0.f};

    ImageId diffuseTexture{NULL_IMAGE_ID};
    ImageId normalMapTexture{NULL_IMAGE_ID};
    ImageId metallicRoughnessTexture{NULL_IMAGE_ID};
    ImageId emissiveTexture{NULL_IMAGE_ID};

    std::string name;
};

#endif /* ifndef _MATERIAL_H_ */
