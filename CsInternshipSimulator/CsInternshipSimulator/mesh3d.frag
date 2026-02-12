#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 Tex;
} fs_in;

uniform sampler2D uDiffMap1;
uniform sampler2D uSpecMap1;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform bool  uHasDiffuseMap;
uniform bool  uHasSpecMap;
uniform vec3  uKd;   // from MTL Kd
uniform vec3  uKs;   // from MTL Ks
uniform float uNs;   // from MTL Ns

uniform float uCookProgress;

vec3 applyCooking(vec3 raw, float t)
{
    t = clamp(t, 0.0, 1.0);
    vec3 cooked = vec3(0.36, 0.22, 0.12);
    vec3 col = mix(raw, cooked, t);
    col *= mix(1.0, 0.78, t);
    float gray = dot(col, vec3(0.299, 0.587, 0.114));
    col = mix(vec3(gray), col, mix(1.0, 0.65, t));
    return col;
}

void main()
{
    vec3 albedo = uHasDiffuseMap ? texture(uDiffMap1, fs_in.Tex).rgb : uKd;
    vec3 base   = applyCooking(albedo, uCookProgress);

    vec3 N = normalize(fs_in.Normal);
    vec3 L = normalize(lightPos - fs_in.WorldPos);
    vec3 V = normalize(viewPos - fs_in.WorldPos);

    float diff = max(dot(N, L), 0.0);

    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), uNs);

    vec3 specColor = uKs;
    if (uHasSpecMap) specColor *= texture(uSpecMap1, fs_in.Tex).rgb;

    vec3 ambient  = 0.05 * base;
    vec3 diffuse  = 0.75 * diff * base;
    vec3 specular = 2 * spec * specColor;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
