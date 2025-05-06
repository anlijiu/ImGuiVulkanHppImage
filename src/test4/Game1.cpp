#include "Game1.h"
#include "Letterbox.h"
#include "Util.h"
#include "Init.h"
#include "CPUMesh.h"
#include "MeshDrawCommand.h"

#include "spdlog/spdlog.h"

void Game1::customInit()
{

    initSceneData(gfxDevice);

    createDrawImage(gfxDevice, params.renderSize);

    { // create camera
        static const float aspectRatio = (float)params.renderSize.x / (float)params.renderSize.y;

        camera.setUseInverseDepth(true);
        camera.init(cameraFovX, cameraNear, cameraFar, aspectRatio);
    }

    materialCache.init(gfxDevice);

    // drawImageId =
    //     gfxDevice.createDrawImage(drawImageFormat, params.renderSize, "game screen draw image");

    // this will create finalDrawImage
    onWindowResize();

    samples = gfxDevice.getMaxSupportedSamplingCount(); // needs to be called before
    meshPipeline.init(gfxDevice, drawImageFormat, depthImageFormat, samples);

    depthResolvePipeline.init(gfxDevice, depthImageFormat);

    postFXPipeline.init(gfxDevice, drawImageFormat);

    const std::vector<CPUMesh::Vertex> squareData = {
        // Front face (+Z)
        { .position = glm::vec3 {-0.5f, -0.5f,  0.5f}, .normal = { 0.0f,  0.0f,  0.1f}},
        { .position = glm::vec3 { 0.5f, -0.5f,  0.5f}, .normal = { 0.0f,  0.0f,  0.1f}},
        { .position = glm::vec3 { 0.5f,  0.5f,  0.5f}, .normal = { 0.0f,  0.0f,  0.1f}},
        { .position = glm::vec3 {-0.5f,  0.5f,  0.5f}, .normal = { 0.0f,  0.0f,  0.1f}},

        // Back face (-Z)
        { .position = glm::vec3 {-0.5f, -0.5f, -0.5f}, .normal = { 0.0f,  0.0f, -0.1f}},
        { .position = glm::vec3 { 0.5f, -0.5f, -0.5f}, .normal = { 0.0f,  0.0f, -0.1f}},
        { .position = glm::vec3 { 0.5f,  0.5f, -0.5f}, .normal = { 0.0f,  0.0f, -0.1f}},
        { .position = glm::vec3 {-0.5f,  0.5f, -0.5f}, .normal = { 0.0f,  0.0f, -0.1f}},

        // Left face (-X)
        { .position = glm::vec3 {-0.5f, -0.5f, -0.5f}, .normal = {-0.1f,  0.0f,  0.0f}},
        { .position = glm::vec3 {-0.5f, -0.5f,  0.5f}, .normal = {-0.1f,  0.0f,  0.0f}},
        { .position = glm::vec3 {-0.5f,  0.5f,  0.5f}, .normal = {-0.1f,  0.0f,  0.0f}},
        { .position = glm::vec3 {-0.5f,  0.5f, -0.5f}, .normal = {-0.1f,  0.0f,  0.0f}},

        // Right face (+X)
        { .position = glm::vec3 { 0.5f, -0.5f, -0.5f}, .normal = { 0.1f,  0.0f,  0.0f}},
        { .position = glm::vec3 { 0.5f, -0.5f,  0.5f}, .normal = { 0.1f,  0.0f,  0.0f}},
        { .position = glm::vec3 { 0.5f,  0.5f,  0.5f}, .normal = { 0.1f,  0.0f,  0.0f}},
        { .position = glm::vec3 { 0.5f,  0.5f, -0.5f}, .normal = { 0.1f,  0.0f,  0.0f}},

        // Top face (+Y)
        { .position = glm::vec3 {-0.5f,  0.5f, -0.5f}, .normal = { 0.0f,  0.1f,  0.0f}},
        { .position = glm::vec3 {-0.5f,  0.5f,  0.5f}, .normal = { 0.0f,  0.1f,  0.0f}},
        { .position = glm::vec3 { 0.5f,  0.5f,  0.5f}, .normal = { 0.0f,  0.1f,  0.0f}},
        { .position = glm::vec3 { 0.5f,  0.5f, -0.5f}, .normal = { 0.0f,  0.1f,  0.0f}},

        // Bottom face (-Y)
        { .position = glm::vec3 {-0.5f, -0.5f, -0.5f}, .normal = { 0.0f, -0.1f,  0.0f}},
        { .position = glm::vec3 {-0.5f, -0.5f,  0.5f}, .normal = { 0.0f, -0.1f,  0.0f}},
        { .position = glm::vec3 { 0.5f, -0.5f,  0.5f}, .normal = { 0.0f, -0.1f,  0.0f}},
        { .position = glm::vec3 { 0.5f, -0.5f, -0.5f}, .normal = { 0.0f, -0.1f,  0.0f}},
   };

    // const std::vector<CPUMesh::Vertex> squareData =
    // {
    //     { .position = glm::vec3 { -0.5f,  0.5f, 0.0f } }, 
    //     { .position = glm::vec3 { 0.0f, 1.0f, 0.0f } }, 
    //     { .position = glm::vec3 { 1.0f, 1.0f, 0.0f } },
    //     { .position = glm::vec3 { 0.0f, 1.0, 0.0f } },
    //     { .position = glm::vec3 { 0.5f, 0.5f, 0.0f } },
    //     { .position = glm::vec3 { 0.0f, 1.0f, 0.0f } },
    //     { .position = glm::vec3 { 1.0f, 0.0f, 0.0f } },
    //     { .position = glm::vec3 { 0.0f, 1.0, 0.0f } },
    //     { .position = glm::vec3 { 0.5f, -0.5f, 0.0f } },
    //     { .position = glm::vec3 { 0.0f, 1.0f, 0.0f } },
    //     { .position = glm::vec3 { 0.0f, 0.0f, 0.0f } },
    //     { .position = glm::vec3 { 1.0f, 1.0, 0.0f } },
    //     { .position = glm::vec3 { -0.5f, -0.5f, 0.0f } },
    //     { .position = glm::vec3 { 0.0f, 1.0f, 0.0f } },
    //     { .position = glm::vec3 { 0.0f, 1.0f, 0.0f } },
    //     { .position = glm::vec3 { 0.0f, 1.0, 0.0f } },
    // };
    // 6 个顶点索引，对应两个三角形，即绘制一个矩形（实际是正方形）
    std::vector<uint32_t> squareIndices = {
        // Front face
        0, 1, 2, 2, 3, 0,
        // Back face
        4, 6, 5, 6, 4, 7,
        // Left face
        8, 9, 10, 10, 11, 8,
        // Right face
        12, 14, 13, 14, 12, 15,
        // Top face
        16, 17, 18, 18, 19, 16,
        // Bottom face
        20, 22, 21, 22, 20, 23,
    };
 
    CPUMesh mesh{.name = "square"};
    mesh.indices = squareIndices;
    mesh.vertices.resize(squareData.size());
    for (std::size_t i = 0; i < squareData.size(); ++i) {
        mesh.vertices[i].position = squareData[i].position;
        mesh.vertices[i].normal = squareData[i].normal;
        mesh.vertices[i].tangent = glm::vec4{squareData[i].position, 0.5f};
    }


    MeshId id = meshCache.addMesh(gfxDevice, mesh);

    Material material {
        .baseColor = {1.0f, 0.0f, 0.0f, 1.0f },
        .name = "",
    };
    testMaterialId = materialCache.addMaterial(gfxDevice, material);

    glm::mat4 transformMatrix = glm::mat4(.1f); // Identity matrix
    transformMatrix = glm::translate(transformMatrix , glm::vec3(0.0f, 0.0f, 0.0f));
    // transformMatrix = glm::rotate(transformMatrix , glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Optional
    // transformMatrix = glm::scale(transformMatrix , glm::vec3(1.0f)); // Uniform scale

    math::Sphere localSphere;
    localSphere.center = glm::vec3(0.0f);
    localSphere.radius = glm::sqrt(20.75f);
    meshDrawCommands.push_back(MeshDrawCommand{
        .meshId = id,
        .transformMatrix = transformMatrix,
        .worldBoundingSphere = localSphere,
        .materialId = testMaterialId,
        // .castShadow = castShadow,
    });


    depthResolvePipeline.init(gfxDevice, depthImageFormat);
}

void Game1::createDrawImage(
    GfxDevice& gfxDevice,
    const glm::ivec2& drawImageSize,
    bool firstCreate)
{
    const auto drawImageExtent = VkExtent3D{
        .width = (std::uint32_t)drawImageSize.x,
        .height = (std::uint32_t)drawImageSize.y,
        .depth = 1,
    };

    { // setup draw image
        VkImageUsageFlags usages{};
        usages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        usages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        usages |= VK_IMAGE_USAGE_SAMPLED_BIT;

        auto createImageInfo = vkutil::CreateImageInfo{
            .format = drawImageFormat,
            .usage = usages,
            .extent = drawImageExtent,
            .samples = samples,
        };
        // reuse the same id if creating again
        drawImageId = gfxDevice.createImage(createImageInfo, "draw image", nullptr, drawImageId);

        if (firstCreate) {
            createImageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // no MSAA
            postFXDrawImageId = gfxDevice.createImage(createImageInfo, "post FX draw image");
        }
    }

    if (firstCreate) { // setup resolve image
        VkImageUsageFlags usages{};
        usages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        usages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        usages |= VK_IMAGE_USAGE_SAMPLED_BIT;

        const auto createImageInfo = vkutil::CreateImageInfo{
            .format = VK_FORMAT_R16G16B16A16_SFLOAT,
            .usage = usages,
            .extent = drawImageExtent,
        };
        resolveImageId = gfxDevice.createImage(createImageInfo, "resolve image");
    }

    { // setup depth image
        auto createInfo = vkutil::CreateImageInfo{
            .format = depthImageFormat,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .extent = drawImageExtent,
            .samples = samples,
        };

        // reuse the same id if creating again
        depthImageId = gfxDevice.createImage(createInfo, "depth image", nullptr, depthImageId);

        if (firstCreate) {
            createInfo.samples = VK_SAMPLE_COUNT_1_BIT; // NO MSAA
            resolveDepthImageId = gfxDevice.createImage(createInfo, "depth resolve");
        }
    }
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

    if (drawImageId != NULL_IMAGE_ID) { // destroy previous image
        const auto& finalDrawImage = gfxDevice.getImage(drawImageId);
        if (finalDrawImage.getSize2D() == finalDrawImageSize) {
            // same size - no need to re-create
            return;
        }
        gfxDevice.destroyImage(finalDrawImage);
    }

    drawImageId = gfxDevice.createDrawImage(
        drawImageFormat, finalDrawImageSize, "final draw image", drawImageId);
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
    depthResolvePipeline.cleanup(gfxDevice.getDevice());

    postFXPipeline.cleanup(gfxDevice.getDevice());
}

void Game1::customUpdate(float dt)
{
    if (!gameDrawnInWindow) {
        const auto& finalDrawImage = gfxDevice.getImage(drawImageId);
        const auto blitRect = util::
            calculateLetterbox(finalDrawImage.getSize2D(), gfxDevice.getSwapchainSize(), true);
        gameWindowPos = {blitRect.x, blitRect.y};
        gameWindowSize = {blitRect.z, blitRect.w};
    }
}

void Game1::customDraw()
{
    auto cmd = gfxDevice.beginFrame();

    const auto& drawImage = gfxDevice.getImage(drawImageId);
    const auto& depthImage = gfxDevice.getImage(depthImageId);

    vkutil::transitionImage(
        cmd, drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // clear screen
    const auto clearColor = glm::vec4{1.f, 1.f, 0.f, 1.f};
    vkutil::clearColorImage(cmd, drawImage.getExtent2D(), drawImage.imageView, clearColor);

#if 1
    {
        SceneData sceneData {
            .camera = camera,
            .ambientColor = LinearColor::FromRGB(255, 0, 0),
            .ambientIntensity = .5f,
            .fogColor = LinearColor::FromRGB(0, 255, 0),
            .fogDensity = .5f
            // .ambientColor = level.getAmbientLightColor(),
            // .ambientIntensity = level.getAmbientLightIntensity(),
        };
        // upload scene data - can only be done after shadow mapping was finished
        const auto gpuSceneData = GPUSceneData{
            .view = sceneData.camera.getView(),
            .proj = sceneData.camera.getProjection(),
            .viewProj = sceneData.camera.getViewProj(),
            .cameraPos = glm::vec4{sceneData.camera.getPosition(), 1.f},
            .ambientColor = LinearColorNoAlpha{sceneData.ambientColor},
            .ambientIntensity = sceneData.ambientIntensity,
            .fogColor = LinearColorNoAlpha{sceneData.fogColor},
            .fogDensity = sceneData.fogDensity,
            // .cascadeFarPlaneZs =
            //     glm::vec4{
            //         csmPipeline.cascadeFarPlaneZs[0],
            //         csmPipeline.cascadeFarPlaneZs[1],
            //         csmPipeline.cascadeFarPlaneZs[2],
            //         0.f,
            //     },
            // .csmLightSpaceTMs = csmPipeline.csmLightSpaceTMs,
            // .csmShadowMapId = (std::uint32_t)csmPipeline.getShadowMap(),
            // .pointLightFarPlane = pointLightMaxRange,
            // .lightsBuffer = lightDataBuffer.getBuffer().address,
            // .numLights = (std::uint32_t)lightDataGPU.size(),
            // .sunlightIndex = sunlightIndex,
            .materialsBuffer = materialCache.getMaterialDataBufferAddress(),
        };
        sceneDataBuffer.uploadNewData(
            cmd, gfxDevice.getCurrentFrameIndex(), (void*)&gpuSceneData, sizeof(GPUSceneData));
    }
#endif

    {
        ZoneScopedN("Geometry");
        TracyVkZoneC(gfxDevice.getTracyVkCtx(), cmd, "Geometry", tracy::Color::ForestGreen);
        vkutil::cmdBeginLabel(cmd, "Geometry");
        vkutil::transitionImage(
            cmd,
            drawImage.image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        vkutil::transitionImage(
            cmd,
            depthImage.image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

        const auto renderInfo = vkutil::createRenderingInfo({
            .renderExtent = drawImage.getExtent2D(),
            .colorImageView = drawImage.imageView,
            .colorImageClearValue = glm::vec4{0.f, 0.f, 0.f, 1.f},
            .depthImageView = depthImage.imageView,
            .depthImageClearValue = 0.f,
            .resolveImageView = VK_NULL_HANDLE,
        });
        vkCmdBeginRendering(cmd, &renderInfo.renderingInfo);

        meshPipeline.draw(
            cmd,
            drawImage.getExtent2D(),
            gfxDevice,
            meshCache,
            materialCache,
            camera,
            sceneDataBuffer.getBuffer(),
            meshDrawCommands,
            testMaterialId);

        vkCmdEndRendering(cmd);
        vkutil::cmdEndLabel(cmd);
    }

    // { // Sync Geometry/Sky with next pass
    //     const auto imageBarrier = VkImageMemoryBarrier2{
    //         .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    //         .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    //         .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
    //         .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
    //         .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
    //         .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    //         .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    //         .image = drawImage.image,
    //         .subresourceRange = vkinit::imageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT),
    //     };
    //     const auto depthBarrier = VkImageMemoryBarrier2{
    //         .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    //         .srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
    //         .srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    //         .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
    //         .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
    //         .oldLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
    //         .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    //         .image = depthImage.image,
    //         .subresourceRange = vkinit::imageSubresourceRange(VK_IMAGE_ASPECT_DEPTH_BIT),
    //     };
    //     const auto barriers = std::array{imageBarrier, depthBarrier};
    //     const auto dependencyInfo = VkDependencyInfo{
    //         .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    //         .imageMemoryBarrierCount = barriers.size(),
    //         .pImageMemoryBarriers = barriers.data(),
    //     };
    //     vkCmdPipelineBarrier2(cmd, &dependencyInfo);
    // }
    // 
        vkutil::transitionImage(
            cmd,
            depthImage.image,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

#if 0
    { // post FX
        ZoneScopedN("Post FX");
        TracyVkZoneC(gfxDevice.getTracyVkCtx(), cmd, "Post FX", tracy::Color::Purple);
        vkutil::cmdBeginLabel(cmd, "Post FX");

        const auto& postFXDrawImage = gfxDevice.getImage(postFXDrawImageId);
        vkutil::transitionImage(
            cmd,
            postFXDrawImage.image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        const auto renderInfo = vkutil::createRenderingInfo({
            .renderExtent = postFXDrawImage.getExtent2D(),
            .colorImageView = postFXDrawImage.imageView,
        });

        vkCmdBeginRendering(cmd, &renderInfo.renderingInfo);
        postFXPipeline.draw(cmd, gfxDevice, postFXDrawImage, depthImage, sceneDataBuffer.getBuffer());
        vkCmdEndRendering(cmd);

        vkutil::cmdEndLabel(cmd);
    }
#endif

    // finish frame
    const auto devClearBgColor = test4::rgbToLinear(97, 120, 159);
    const auto endFrameProps = GfxDevice::EndFrameProps{
        .clearColor = devClearBgColor /*gameDrawnInWindow ? devClearBgColor : LinearColor::Black()*/,
        .copyImageIntoSwapchain = !gameDrawnInWindow,
        .drawImageBlitRect = {gameWindowPos.x, gameWindowPos.y, gameWindowSize.x, gameWindowSize.y},
        .drawImageLinearBlit = false,
        .drawImGui = drawImGui,
    };
    gfxDevice.endFrame(cmd, drawImage, endFrameProps);
}
