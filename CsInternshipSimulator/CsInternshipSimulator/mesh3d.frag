#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 Tex;
} fs_in;

// These names match your Mesh::Draw() uniform naming
uniform sampler2D uDiffMap1;
// If your model has spec maps and you want them:
uniform sampler2D uSpecMap1;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform bool uHasDiffuseMap;
uniform vec3 uKd;

uniform float uCookProgress;

vec3 applyCooking(vec3 raw, float t)
{
    t = clamp(t, 0.0, 1.0);

    // warm cooked brown target
    vec3 cooked = vec3(0.36, 0.22, 0.12);

    vec3 col = mix(raw, cooked, t);

    // darken
    col *= mix(1.0, 0.78, t);

    // reduce saturation (approx)
    float gray = dot(col, vec3(0.299, 0.587, 0.114));
    col = mix(vec3(gray), col, mix(1.0, 0.65, t));

    return col;
}

void main()
{
     vec3 albedo = uHasDiffuseMap
        ? texture(uDiffMap1, fs_in.Tex).rgb
        : uKd;

    // Apply cooking tint to albedo (patty looks raw->cooked)
    vec3 base = applyCooking(albedo, uCookProgress);

    vec3 N = normalize(fs_in.Normal);
    vec3 L = normalize(lightPos - fs_in.WorldPos);
    vec3 V = normalize(viewPos - fs_in.WorldPos);

    float diff = max(dot(N, L), 0.0);

    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(V, R), 0.0), 32.0);

    vec3 ambient = 0.15 * base;
    vec3 diffuse = 0.85 * diff * base;
    vec3 specular = 0.18 * spec * vec3(1.0);

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
