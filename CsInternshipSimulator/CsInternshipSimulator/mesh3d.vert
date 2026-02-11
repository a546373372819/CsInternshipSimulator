#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;

out VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 Tex;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 world = model * vec4(aPos, 1.0);
    vs_out.WorldPos = world.xyz;

    mat3 normalMat = transpose(inverse(mat3(model)));
    vs_out.Normal = normalize(normalMat * aNormal);

    vs_out.Tex = aTex;

    gl_Position = projection * view * world;
}
