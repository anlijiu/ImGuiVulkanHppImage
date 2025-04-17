#version 450

layout(location = 0) in vec2 pos;
layout(location = 1) in vec3 color;

layout(location= 0) out vec3 fragColor; //out-> verrà passato nel pipeline fino al frag shader


void main() { //once per vertex
    gl_Position = vec4(pos, 0.0, 1.0);
    // 4 dim vector, x,y,layer,normalization(1.0 non cambia nulla)
    fragColor = color;
}