#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 1) readonly buffer CubesData {
    mat4 model;
    vec3 color;
} cubesData[];

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    mat4 model = cubesData[gl_InstanceIndex].model;
    gl_Position = ubo.proj * ubo.view * model * vec4(inPosition, 1.0);
    fragColor = inColor * cubesData[gl_InstanceIndex].color;
}
