#version 330 core

layout(location = 0) in vec2 aPos; 

uniform float uFill;    
uniform float uLeft;   
uniform float uRight;
uniform float uTop;
uniform float uBottom;

void main()
{
    float xLocal = aPos.x * uFill;

    float xNDC = mix(uLeft, uRight, xLocal);
    float yNDC = mix(uBottom, uTop, aPos.y);  // 0 -> bottom, 1 -> top

    gl_Position = vec4(xNDC, yNDC, 0.0, 1.0);
}
