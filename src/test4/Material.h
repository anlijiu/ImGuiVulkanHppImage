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
    std::uint32_t diffuseTex;//用于存储漫反射贴图的纹理
    std::uint32_t normalTex;//法线贴图

    // 1. BaseColor（RGB三通道-sRGB空间) = 漫反射（电介质反射颜色）+金属反射。
    // 2. Matallic(Grayscale灰度图-Linear线性空间) 作为一个0-1的黑白值遮罩调整金属与非金属。
    // 3. Roughness（Grayscale灰度图-Linear线性空间）灰度图值 0代表光滑表面，而1代表粗糙表面。
    std::uint32_t metallicRoughnessTex;//金属度和粗糙

    std::uint32_t emissiveTex;//即自发光贴图（Emissive Texture），是一种用于模拟物体自身发光效果的纹理。
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
