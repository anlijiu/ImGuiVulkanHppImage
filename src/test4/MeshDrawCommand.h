#pragma once

#ifndef _MESH_DRAW_COMMAND_H_
#define _MESH_DRAW_COMMAND_H_

#include <glm/mat4x4.hpp>

#include "Math/Sphere.h"

#include "IdTypes.h"

struct SkinnedMesh;

struct MeshDrawCommand {
    MeshId meshId;
    glm::mat4 transformMatrix;

    // for frustum culling (视锥体剔除)
    math::Sphere worldBoundingSphere;

    // If set - mesh will be drawn with overrideMaterialId
    // instead of whatever material the mesh has
    MaterialId materialId{NULL_MATERIAL_ID};

    bool castShadow{true};

    // skinned meshes only
    const SkinnedMesh* skinnedMesh{nullptr};
    std::uint32_t jointMatricesStartIndex;
};

#endif /* ifndef _MESH_DRAW_COMMAND_H_ */
