#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;

out vec2 vTexCoord;

uniform float uX;      // pomeraj po x
uniform float uY;      // pomeraj po y
uniform float uS;      // skala (ako ti treba)
uniform int hasHat;    // ignorisi za sad
uniform int flipped;   // ignorisi za sad

void main()
{
    // primeni pomeraj i skalu
    vec2 pos = aPos * uS + vec2(uX, uY);
    gl_Position = vec4(pos, 0.0, 1.0);

    vTexCoord = aTex;
}
