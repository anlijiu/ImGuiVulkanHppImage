#!/bin/bash

mkdir -p build/shaders

glslc src/test1/shaders/shader.vert -o build/shaders/shader.vert.spv
glslc src/test1/shaders/shader.frag -o build/shaders/shader.frag.spv

#################################################################### 

dxc -spirv -T "vs_6_0" -E "mainVS" src/test1/shaders/ibl.hlsl -Fo build/shaders/ibl.hlsl.vert.spv
dxc -spirv -T "vs_6_0" -E "mainVS" src/test1/shaders/skybox.hlsl -Fo build/shaders/skybox.hlsl.vert.spv

dxc -spirv -T "ps_6_0" -E "mainPS" src/test1/shaders/ibl.hlsl -Fo build/shaders/ibl.hlsl.frag.spv
dxc -spirv -T "ps_6_0" -E "mainPS" src/test1/shaders/skybox.hlsl -Fo build/shaders/skybox.hlsl.frag.spv

##################################################################

glslc src/test3/shaders/simple_shader.vert -o build/shaders/simple_shader.vert.spv
glslc src/test3/shaders/simple_shader.frag -o build/shaders/simple_shader.frag.spv

