#include "MeshCache1.h"

#include "CPUMesh.h"
#include "GfxDevice.h"
#include "Util.h"
#include "Math/Util.h"

MeshId MeshCache1::addMesh(GfxDevice& gfxDevice, const CPUMesh& cpuMesh)
{
    auto gpuMesh = GPUMesh{
        .numVertices = (std::uint32_t)cpuMesh.vertices.size(),
        .numIndices = (std::uint32_t)cpuMesh.indices.size(),
        .minPos = cpuMesh.minPos,
        .maxPos = cpuMesh.maxPos,
        .hasSkeleton = cpuMesh.hasSkeleton,
    };

    std::vector<glm::vec3> positions(cpuMesh.vertices.size());
    for (std::size_t i = 0; i < cpuMesh.vertices.size(); ++i) {
        positions[i] = cpuMesh.vertices[i].position;
    }
    gpuMesh.boundingSphere = util::calculateBoundingSphere(positions);

    uploadMesh(gfxDevice, cpuMesh, gpuMesh);
    const auto id = meshes.size();
    meshes.push_back(std::move(gpuMesh));
    return id;
}

void MeshCache1::uploadMesh(GfxDevice& gfxDevice, const CPUMesh& cpuMesh, GPUMesh& gpuMesh) const
{
    // create index buffer
    const auto indexBufferSize = cpuMesh.indices.size() * sizeof(std::uint32_t);
    gpuMesh.indexBuffer = gfxDevice.createBuffer(
        indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    // create vertex buffer
    const auto vertexBufferSize = cpuMesh.vertices.size() * sizeof(CPUMesh::Vertex);
    gpuMesh.vertexBuffer = gfxDevice.createBuffer(
        vertexBufferSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

    const auto staging =
        gfxDevice
            .createBuffer(vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    // copy data
    void* data = staging.info.pMappedData;
    memcpy(data, cpuMesh.vertices.data(), vertexBufferSize);
    memcpy((char*)data + vertexBufferSize, cpuMesh.indices.data(), indexBufferSize);

    gfxDevice.immediateSubmit([&](VkCommandBuffer cmd) {
        const auto vertexCopy = VkBufferCopy{
            .srcOffset = 0,
            .dstOffset = 0,
            .size = vertexBufferSize,
        };
        vkCmdCopyBuffer(cmd, staging.buffer, gpuMesh.vertexBuffer.buffer, 1, &vertexCopy);

        const auto indexCopy = VkBufferCopy{
            .srcOffset = vertexBufferSize,
            .dstOffset = 0,
            .size = indexBufferSize,
        };
        vkCmdCopyBuffer(cmd, staging.buffer, gpuMesh.indexBuffer.buffer, 1, &indexCopy);
    });

    gfxDevice.destroyBuffer(staging);

    const auto vtxBufferName = cpuMesh.name + " (vtx)";
    const auto idxBufferName = cpuMesh.name + " (idx)";
    vkutil::
        addDebugLabel(gfxDevice.getDevice(), gpuMesh.vertexBuffer.buffer, vtxBufferName.c_str());
    vkutil::addDebugLabel(gfxDevice.getDevice(), gpuMesh.indexBuffer.buffer, idxBufferName.c_str());

    if (gpuMesh.hasSkeleton) {
        // create skinning data buffer
        const auto skinningDataSize = cpuMesh.vertices.size() * sizeof(CPUMesh::SkinningData);
        gpuMesh.skinningDataBuffer = gfxDevice.createBuffer(
            skinningDataSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);

        const auto staging =
            gfxDevice.createBuffer(skinningDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

        // copy data
        void* data = staging.info.pMappedData;
        memcpy(data, cpuMesh.skinningData.data(), skinningDataSize);

        gfxDevice.immediateSubmit([&](VkCommandBuffer cmd) {
            const auto vertexCopy = VkBufferCopy{
                .srcOffset = 0,
                .dstOffset = 0,
                .size = skinningDataSize,
            };
            vkCmdCopyBuffer(cmd, staging.buffer, gpuMesh.skinningDataBuffer.buffer, 1, &vertexCopy);
        });

        gfxDevice.destroyBuffer(staging);
    }
}

const GPUMesh& MeshCache1::getMesh(MeshId id) const
{
    return meshes.at(id);
}

const std::vector<GPUMesh>& MeshCache1::allMeshes() const {
    return meshes;
}

void MeshCache1::cleanup(const GfxDevice& gfxDevice)
{
    for (const auto& mesh : meshes) {
        gfxDevice.destroyBuffer(mesh.indexBuffer);
        gfxDevice.destroyBuffer(mesh.vertexBuffer);
        if (mesh.hasSkeleton) {
            gfxDevice.destroyBuffer(mesh.skinningDataBuffer);
        }
    }
}
