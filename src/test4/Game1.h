#pragma once

#ifndef _GAME1_H_
#define _GAME1_H_

#include "App.h"
#include "Camera.h"
#include "MeshCache1.h"
#include "MaterialCache1.h"
#include "MeshPipeline1.h"
#include "MeshDrawCommand.h"
#include "NBuffer.h"


class Game1 : public App {
public:

    struct SceneData {
        const Camera& camera;
        LinearColor ambientColor;
        float ambientIntensity;
        LinearColor fogColor;
        float fogDensity;
    };
    void customInit() override;
    void loadAppSettings() override;
    void customCleanup() override;

    void customUpdate(float dt) override;
    void customDraw() override;

    ImageId getMainDrawImageId() const override { return finalDrawImageId; }

    void onWindowResize() override;

private:

    void initSceneData(GfxDevice& gfxDevice);

    Camera camera;
    float cameraNear{1.f};
    float cameraFar{200.f};
    float cameraFovX{glm::radians(45.f)};

    MeshCache1 meshCache;
    MaterialCache1 materialCache;
    MaterialId testMaterialId{0};

    std::vector<MeshDrawCommand> meshDrawCommands;
    std::vector<std::size_t> sortedMeshDrawCommands;

    struct GPUSceneData {
        // camera
        glm::mat4 view;
        glm::mat4 proj;
        glm::mat4 viewProj;
        glm::vec4 cameraPos;

        // ambient
        LinearColorNoAlpha ambientColor;
        float ambientIntensity;

        // fog
        LinearColorNoAlpha fogColor;
        float fogDensity;

        // CSM data
        glm::vec4 cascadeFarPlaneZs;
        // std::array<glm::mat4, CSMPipeline::NUM_SHADOW_CASCADES> csmLightSpaceTMs;
        std::array<glm::mat4, 3/*CSMPipeline::NUM_SHADOW_CASCADES*/> csmLightSpaceTMs;
        std::uint32_t csmShadowMapId;

        // Point light data
        float pointLightFarPlane;

        VkDeviceAddress lightsBuffer;
        std::uint32_t numLights;
        std::int32_t sunlightIndex;

        VkDeviceAddress materialsBuffer;
    };
    NBuffer sceneDataBuffer;

    glm::ivec2 gameWindowPos;
    glm::ivec2 gameWindowSize;

    VkFormat drawImageFormat{VK_FORMAT_R16G16B16A16_SFLOAT};
    VkFormat depthImageFormat{VK_FORMAT_D32_SFLOAT};
    ImageId gameScreenDrawImageId{NULL_IMAGE_ID}; // image to which game pixels are drawn to
    ImageId finalDrawImageId{NULL_IMAGE_ID}; // id of image which is drawn to the window

    MeshPipeline1 meshPipeline;

    VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_1_BIT};

    glm::vec2 playerPos;

    // dev
    bool gameDrawnInWindow{true};
    bool drawImGui{false};

};

#endif /* ifndef _GAME1_H_ */
