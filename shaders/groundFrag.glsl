#version 330 core

in vec2 TexCoord;

out vec4 fragColor;

uniform sampler2D ground;

void main(void){
    fragColor = texture(ground, TexCoord);
}
