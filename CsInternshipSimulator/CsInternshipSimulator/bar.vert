#version 330 core

layout(location = 0) in vec2 aPos; // (x,y) u opsegu [0,1]

uniform float uFill;    // 0..1, koliko bara je popunjeno
uniform float uLeft;    // NDC koordinate
uniform float uRight;
uniform float uTop;
uniform float uBottom;

void main()
{
    // aPos.x ide od 0 do 1; množimo sa uFill da isecemo deo koji je popunjen
    float xLocal = aPos.x * uFill;

    // Interpolacija od leve do desne ivice u NDC
    float xNDC = mix(uLeft, uRight, xLocal);
    float yNDC = mix(uBottom, uTop, aPos.y);  // 0 -> bottom, 1 -> top

    gl_Position = vec4(xNDC, yNDC, 0.0, 1.0);
}
