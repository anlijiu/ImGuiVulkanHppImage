#!/bin/bash

mkdir -p build/shaders

glslc src/test1/shaders/shader.vert -o build/shaders/shader.vert.spv
glslc src/test1/shaders/shader.frag -o build/shaders/shader.frag.spv

#################################################################### 

glslc src/test2/shaders/passthrough_vert.vert -o build/shaders/passthrough_vert.spv
glslc src/test2/shaders/passthrough_frag.frag -o build/shaders/passthrough_frag.spv

#################################################################### 

glslc src/test4/shaders/mesh1.vert -o build/shaders/mesh1.vert.spv
glslc src/test4/shaders/mesh1.frag -o build/shaders/mesh1.frag.spv
glslc src/test4/shaders/mesh.vert -o build/shaders/mesh.vert.spv
glslc src/test4/shaders/mesh.frag -o build/shaders/mesh.frag.spv
glslc src/test4/shaders/fullscreen_triangle.vert -o build/shaders/fullscreen_triangle.vert.spv
glslc src/test4/shaders/depth_resolve.frag -o build/shaders/depth_resolve.frag.spv
glslc src/test4/shaders/postfx.frag -o build/shaders/postfx.frag.spv

#################################################################### 

dxc -spirv -T "vs_6_0" -E "mainVS" src/test1/shaders/ibl.hlsl -Fo build/shaders/ibl.hlsl.vert.spv
dxc -spirv -T "vs_6_0" -E "mainVS" src/test1/shaders/skybox.hlsl -Fo build/shaders/skybox.hlsl.vert.spv

dxc -spirv -T "ps_6_0" -E "mainPS" src/test1/shaders/ibl.hlsl -Fo build/shaders/ibl.hlsl.frag.spv
dxc -spirv -T "ps_6_0" -E "mainPS" src/test1/shaders/skybox.hlsl -Fo build/shaders/skybox.hlsl.frag.spv

##################################################################

glslc src/test3/shaders/simple_shader.vert -o build/shaders/simple_shader.vert.spv
glslc src/test3/shaders/simple_shader.frag -o build/shaders/simple_shader.frag.spv

