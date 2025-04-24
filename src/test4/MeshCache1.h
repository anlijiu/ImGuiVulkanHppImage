#pragma once

#ifndef _MESH_CACHE1_H_
#define _MESH_CACHE1_H_

#include <vector>

#include "GPUMesh.h"

class GfxDevice;
struct CPUMesh;

class MeshCache1 {
public:
    void cleanup(const GfxDevice& gfxDevice);

    MeshId addMesh(GfxDevice& gfxDevice, const CPUMesh& cpuMesh);
    const GPUMesh& getMesh(MeshId id) const;
    const std::vector<GPUMesh>& allMeshes() const;

private:
    void uploadMesh(GfxDevice& gfxDevice, const CPUMesh& cpuMesh, GPUMesh& gpuMesh) const;

    std::vector<GPUMesh> meshes;
};

#endif /* ifndef _MESH_CACHE1_H_ */
