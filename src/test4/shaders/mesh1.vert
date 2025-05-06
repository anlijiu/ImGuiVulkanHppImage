#version 460

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_debug_printf : enable

#include "mesh_pcs.glsl"

layout (location = 0) out vec3 outPos;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec4 outTangent;
layout (location = 4) out mat3 outTBN;

void main()
{
    Vertex v = pcs.vertexBuffer.vertices[gl_VertexIndex];

    vec4 worldPos = pcs.transform * vec4(v.position, 1.0f);

    gl_Position = pcs.sceneData.viewProj * worldPos;
    gl_Position.w = 1.0f;
    debugPrintfEXT("outPos is %f %f %f %f", gl_Position.x, gl_Position.y, gl_Position.z, gl_Position.w);

    outPos = worldPos.xyz;
    debugPrintfEXT("outPos is %f %f %f ", outPos.x, outPos.y, outPos.z);

    outUV = vec2(v.uv_x, v.uv_y);
    debugPrintfEXT("outUV is %f %f ", outUV.x, outUV.y);

    // A bit inefficient, but okay - this is needed for non-uniform scale
    // models. See: http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/
    // Simpler case, when everything is uniform
    // outNormal = (pcs.transform * vec4(v.normal, 0.0)).xyz;
    outNormal = mat3(transpose(inverse(pcs.transform))) * v.normal;
    debugPrintfEXT("outNormal is %f %f %f  ", outNormal[0], outNormal[1], outNormal[2] );

    outTangent = v.tangent;
    debugPrintfEXT("outTangent is %f %f %f %f ", outTangent [0], outTangent [1], outTangent [2], outTangent [3] );

    vec3 T = normalize(vec3(pcs.transform * v.tangent));
    vec3 N = normalize(outNormal);
    vec3 B = cross(N, T) * v.tangent.w;
    outTBN = mat3(T, B, N);

    /* debugPrintfEXT("outTBN.T.x is %f", T.x); */

}
