#version 330 core

in vec3 TexCoord;

out vec4 fragColor;

uniform samplerCube skybox;

void main(void){
    fragColor = texture(skybox, TexCoord);
}
