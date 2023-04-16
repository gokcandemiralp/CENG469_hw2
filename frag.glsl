#version 330 core

in vec4 color;
in vec2 TexCoord;

out vec4 fragColor;

uniform sampler2D ourTexture;

void main(void){
    TexCoord;
    // fragColor = vec4(TexCoord.x, 0, TexCoord.y, 1);
    fragColor = texture(ourTexture, TexCoord);
}
