#version 450

#extension GL_EXT_debug_printf : enable

layout (location = 0) in vec3 color;
layout (location = 0) out vec4 outColor;


void main() { 

    debugPrintfEXT("simple_shader.frag in ");
    outColor = vec4(color, 1.0); //color RGB + alpha a 1
}
