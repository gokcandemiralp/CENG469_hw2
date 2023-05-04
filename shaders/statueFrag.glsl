#version 330 core

in vec2 TexCoord;
in vec3 normal;
in vec4 specular;

out vec4 fragColor;

uniform sampler2D sampler;

void main(void){
    fragColor = texture(sampler, TexCoord) + specular;
}
