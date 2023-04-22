#version 330 core

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;

layout (location = 0) in vec3 aPos;

out vec3 TexCoord;

void main(void){
    TexCoord = vec3(aPos.x, aPos.y, -aPos.z);
    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(aPos, 1);
}

