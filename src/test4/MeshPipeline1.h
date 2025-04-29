#pragma once

#ifndef _MESH_PIPELINE1_H_
#define _MESH_PIPELINE1_H_

#include <glm/mat4x4.hpp>

#include <vector>

#include <vulkan/vulkan.h>

class GfxDevice;
class MeshCache1;
class Camera;
struct GPUImage;
struct GPUBuffer;
struct MeshDrawCommand;
struct MaterialCache1;

class MeshPipeline1 {

using MaterialId = std::uint32_t;
public:
    void init(
        GfxDevice& gfxDevice,
        VkFormat drawImageFormat,
        VkFormat depthImageFormat,
        VkSampleCountFlagBits samples);
    void cleanup(VkDevice device);

    void draw(
        VkCommandBuffer cmd,
        VkExtent2D renderExtent,
        const GfxDevice& gfxDevice,
        const MeshCache1& meshCache,
        const MaterialCache1& materialCache,
        const Camera& camera,
        const GPUBuffer& sceneDataBuffer,
        const std::vector<MeshDrawCommand>& drawCommands,
        const MaterialId testMaterialId);

private:
    struct PushConstants {
        glm::mat4 transform;
        VkDeviceAddress sceneDataBuffer;
        VkDeviceAddress vertexBuffer;
        std::uint32_t materialId;
        std::uint32_t padding;
    };

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
};


#endif /* ifndef _MESH_PIPELINE1_H_ */
