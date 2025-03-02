#version 330 core

// All of the following variables could be defined in the OpenGL
// program and passed to this shader as uniform variables. This
// would be necessary if their values could change during runtim.
// However, we will not change them and therefore we define them 
// here for simplicity.

vec3 I = vec3(1, 1, 1);          // point light intensity
vec3 Iamb = vec3(0.8, 0.8, 0.8); // ambient light intensity
vec3 ka = vec3(0.3, 0.3, 0.3);   // ambient reflectance coefficient
vec3 ks = vec3(0.8, 0.8, 0.8);   // specular reflectance coefficient
vec3 lightPos = vec3(5, 5, 5);   // light position in world coordinates

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;
uniform vec3 eyePos;

layout(location=0) in vec3 inVertex;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 aTexCoord;

out vec4 color;
out vec2 TexCoord;

void main(void){
	vec4 pWorld = modelingMatrix * vec4(inVertex, 1);
	vec3 nWorld = inverse(transpose(mat3x3(modelingMatrix))) * inNormal;

	vec3 L = normalize(lightPos - vec3(pWorld));
	vec3 V = normalize(eyePos - vec3(pWorld));
	vec3 H = normalize(L + V);
	vec3 N = normalize(nWorld);
	float NdotH = dot(N, H); // for specular component
	vec3 specularColor = I * ks * pow(max(0, NdotH), 100);

	color = vec4(specularColor, 1);
    TexCoord = aTexCoord;
    
    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(inVertex, 1);
}

