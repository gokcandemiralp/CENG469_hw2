#version 330 core

uniform mat4 modelingMatrix;
uniform vec3 eyePos;

layout(location=0) in vec3 aPos;
layout (std140) uniform Matrices{
    mat4 projectionMatrix;
    mat4 viewingMatrix;
};

out vec2 TexCoord;

void main(void){
    TexCoord = vec2(aPos.x*10, -aPos.z*10);
    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(aPos, 1);
}
