#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 uvCoord;

void main() {
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    uvCoord = aTexCoord;
}