#pragma once

#ifndef _GAME1_H_
#define _GAME1_H_

#include "App.h"
#include "Camera.h"
#include "MeshCache1.h"
#include "MaterialCache.h"
#include "MeshPipeline1.h"
#include "MeshDrawCommand.h"
#include "NBuffer.h"


class Game1 : public App {
public:
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
    MaterialCache materialCache;
    std::vector<MeshDrawCommand> meshDrawCommands;
    std::vector<std::size_t> sortedMeshDrawCommands;

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
