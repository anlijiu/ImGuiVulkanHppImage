#include "Game1.h"
#include "Letterbox.h"
#include "Util.h"
#include "CPUMesh.h"

#include "spdlog/spdlog.h"

void Game1::customInit()
{

    initSceneData(gfxDevice);

    { // create camera
        static const float aspectRatio = (float)params.renderSize.x / (float)params.renderSize.y;

        camera.setUseInverseDepth(true);
        camera.init(cameraFovX, cameraNear, cameraFar, aspectRatio);
    }

    materialCache.init(gfxDevice);

    gameScreenDrawImageId =
        gfxDevice.createDrawImage(drawImageFormat, params.renderSize, "game screen draw image");

    // this will create finalDrawImage
    onWindowResize();

    samples = gfxDevice.getMaxSupportedSamplingCount(); // needs to be called before
    meshPipeline.init(gfxDevice, drawImageFormat, depthImageFormat, samples);

    const std::vector<CPUMesh::Vertex> squareData =
    {
        { .position = glm::vec3 { -0.5f,  0.5f, 1.0f } }, 
        { .position = glm::vec3 { 0.0f, 1.0f, 1.0f } }, 
        { .position = glm::vec3 { 1.0f, 1.0f, 1.0f } },
        { .position = glm::vec3 { 0.0f, 1.0, 1.0f } },
        { .position = glm::vec3 { 0.5f, 0.5f, 1.0f } },
        { .position = glm::vec3 { 0.0f, 1.0f, 1.0f } },
        { .position = glm::vec3 { 1.0f, 0.0f, 1.0f } },
        { .position = glm::vec3 { 0.0f, 1.0, 1.0f } },
        { .position = glm::vec3 { 0.5f, -0.5f, 1.0f } },
        { .position = glm::vec3 { 0.0f, 1.0f, 1.0f } },
        { .position = glm::vec3 { 0.0f, 0.0f, 1.0f } },
        { .position = glm::vec3 { 1.0f, 1.0, 1.0f } },
        { .position = glm::vec3 { -0.5f, -0.5f, 1.0f } },
        { .position = glm::vec3 { 0.0f, 1.0f, 1.0f } },
        { .position = glm::vec3 { 0.0f, 1.0f, 1.0f } },
        { .position = glm::vec3 { 0.0f, 1.0, 1.0f } },
    };
    // 6 个顶点索引，对应两个三角形，即绘制一个矩形（实际是正方形）
    std::vector<uint32_t> squareIndices = { 0,3,1, 3,2,1 }; // 6 indices
 
    CPUMesh mesh{.name = "square"};
    mesh.indices = squareIndices;
    mesh.vertices.resize(squareData.size());
    for (std::size_t i = 0; i < squareData.size(); ++i) {
        mesh.vertices[i].position = squareData[i].position;
    }
    meshCache.addMesh(gfxDevice, mesh);

    Material material{ .baseColor = {1.0f, 0.0f, 0.0f, 1.0f}};
    testMaterialId = materialCache.addMaterial(gfxDevice, material);
}

void Game1::initSceneData(GfxDevice& gfxDevice)
{
    sceneDataBuffer.init(
        gfxDevice,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        sizeof(GPUSceneData),
        graphics::FRAME_OVERLAP,
        "scene data");

}

void Game1::onWindowResize()
{
    bool integerScale = false;
    const auto blitRect =
        util::calculateLetterbox(params.renderSize, params.windowSize, integerScale);
    const auto finalDrawImageSize = glm::ivec2{blitRect.z, blitRect.w};

    if (finalDrawImageId != NULL_IMAGE_ID) { // destroy previous image
        const auto& finalDrawImage = gfxDevice.getImage(finalDrawImageId);
        if (finalDrawImage.getSize2D() == finalDrawImageSize) {
            // same size - no need to re-create
            return;
        }
        gfxDevice.destroyImage(finalDrawImage);
    }

    finalDrawImageId = gfxDevice.createDrawImage(
        drawImageFormat, finalDrawImageSize, "final draw image", finalDrawImageId);
}

void Game1::loadAppSettings()
{
    // const std::filesystem::path appSettingsPath{"assets/data/default_app_settings.json"};
    // JsonFile file(appSettingsPath);
    // if (!file.isGood()) {
    //     fmt::println("failed to load app settings from {}", appSettingsPath.string());
    //     return;
    // }
    // 
    // const auto loader = file.getLoader();
    // loader.getIfExists("renderResolution", params.renderSize);
    // loader.getIfExists("vSync", vSync);
    // if (params.windowSize == glm::ivec2{}) {
    //     loader.getIfExists("windowSize", params.windowSize);
    // } // otherwise it was already set by dev settings
    // 
    // params.version = Version{
    //     .major = 0,
    //     .minor = 1,
    //     .patch = 0,
    // };
}

void Game1::customCleanup()
{
    gfxDevice.waitIdle();
    meshPipeline.cleanup(gfxDevice.getDevice());
}

void Game1::customUpdate(float dt)
{
    if (!gameDrawnInWindow) {
        const auto& finalDrawImage = gfxDevice.getImage(finalDrawImageId);
        const auto blitRect = util::
            calculateLetterbox(finalDrawImage.getSize2D(), gfxDevice.getSwapchainSize(), true);
        gameWindowPos = {blitRect.x, blitRect.y};
        gameWindowSize = {blitRect.z, blitRect.w};
    }
}

void Game1::customDraw()
{
    auto cmd = gfxDevice.beginFrame();

    const auto& drawImage = gfxDevice.getImage(gameScreenDrawImageId);
    vkutil::transitionImage(
        cmd, drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // clear screen
    const auto clearColor = glm::vec4{1.f, 1.f, 0.f, 1.f};
    vkutil::clearColorImage(cmd, drawImage.getExtent2D(), drawImage.imageView, clearColor);

    // apply mesh Pipeline
    const auto& finalDrawImage = gfxDevice.getImage(finalDrawImageId);

    {
        ZoneScopedN("Geometry");
        TracyVkZoneC(gfxDevice.getTracyVkCtx(), cmd, "Geometry", tracy::Color::ForestGreen);
        vkutil::cmdBeginLabel(cmd, "Geometry");
        vkutil::transitionImage(
            cmd,
            drawImage.image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        const auto renderInfo = vkutil::createRenderingInfo({
            .renderExtent = finalDrawImage.getExtent2D(),
            .colorImageView = finalDrawImage.imageView,
            .colorImageClearValue = glm::vec4{0.f, 0.f, 0.f, 1.f},
        });

        vkCmdBeginRendering(cmd, &renderInfo.renderingInfo);

        meshPipeline.draw(
            cmd,
            finalDrawImage.getExtent2D(),
            gfxDevice,
            meshCache,
            materialCache,
            camera,
            sceneDataBuffer.getBuffer(),
            testMaterialId);

        vkCmdEndRendering(cmd);
        vkutil::cmdEndLabel(cmd);
    }

    // finish frame
    const auto devClearBgColor = test4::rgbToLinear(97, 120, 159);
    const auto endFrameProps = GfxDevice::EndFrameProps{
        .clearColor = gameDrawnInWindow ? devClearBgColor : LinearColor::Black(),
        .copyImageIntoSwapchain = !gameDrawnInWindow,
        .drawImageBlitRect = {gameWindowPos.x, gameWindowPos.y, gameWindowSize.x, gameWindowSize.y},
        .drawImageLinearBlit = false,
        .drawImGui = drawImGui,
    };
    gfxDevice.endFrame(cmd, finalDrawImage, endFrameProps);
}
