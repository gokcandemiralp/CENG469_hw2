#version 330 core

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;

layout (location = 0) in vec3 aPos;
layout (std140) uniform Matrices{
    mat4 projectionMatrix;
    mat4 view;
};

out vec3 TexCoord;

void main(void){
    TexCoord = vec3(aPos.x, aPos.y, -aPos.z);
    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(aPos, 1);
}

