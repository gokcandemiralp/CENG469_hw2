#version 330 core

in vec2 TexCoord;

out vec4 fragColor;
in vec3 normal;
in vec4 specular;
in vec3 Position;

uniform sampler2D sampler;
uniform samplerCube skybox;

void main(void){
    vec3 I = normalize(Position);
    vec3 R = reflect(I, normalize(normal));
    fragColor = vec4(texture(skybox, R));
    // fragColor = texture(sampler, TexCoord) + specular;
    // fragColor = vec4(normal,1);
}
