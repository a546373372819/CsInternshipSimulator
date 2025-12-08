#version 330 core

out vec4 FragColor;

uniform vec4 uColor;   // RGBA boja bara

void main()
{
    FragColor = uColor;
}
