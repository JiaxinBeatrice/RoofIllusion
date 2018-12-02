#version 150 core

in vec3 position;
in vec3 inNormal;

const vec3 inLightDir = normalize(vec3(0,1,-.3));

out vec3 Color;
out vec3 Normal;
out vec3 pos;
out vec3 lightDir;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;
uniform vec3 inColor;

void main() {
   Color = inColor;
   gl_Position = proj * view * model * vec4(position,1.0);
   pos = (view * model * vec4(position,1.0)).xyz;
   lightDir = (view * vec4(inLightDir,0.0)).xyz; //It's a vector!
   vec4 norm4 = transpose(inverse(view*model)) * vec4(inNormal,0.0);
   Normal = normalize(norm4.xyz);
}