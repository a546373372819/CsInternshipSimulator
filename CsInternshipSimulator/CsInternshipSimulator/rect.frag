#version 330 core

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uTex0;
uniform float uCookProgress; // 0.0 = raw, 1.0 = cooked

void main()
{
    vec4 base = texture(uTex0, vTexCoord);

    // Braon ton koji želiš da patty dobije
vec3 cookedColor = vec3(0.25, 0.12, 0.05);

    // Miksiramo originalni piksel i braon boju
    vec3 finalColor = mix(base.rgb, cookedColor, uCookProgress);

    FragColor = vec4(finalColor, base.a);
}

 