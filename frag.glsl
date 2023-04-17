#version 330 core

in vec4 color;
in vec2 TexCoord;

out vec4 fragColor;

uniform sampler2D ourTexture;

void main(void){
    // fragColor = vec4(TexCoord.x, 0, TexCoord.y, 1);
    // fragColor = texture(ourTexture, TexCoord);
    // fragColor = vec4(0.9,0.9,0.9,1);
    fragColor = color;
}
