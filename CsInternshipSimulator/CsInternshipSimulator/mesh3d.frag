#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 Tex;
} fs_in;

// Diffuse + optional specular map
uniform sampler2D uDiffMap1;
uniform sampler2D uSpecMap1;

uniform vec3 lightPos;
uniform vec3 viewPos;

// Material (MTL-style)
uniform bool  uHasDiffuseMap;   // true if you bound a real diffuse texture
uniform vec3  uKd;              // diffuse color (MTL: Kd)
uniform vec3  uKs;              // specular color (MTL: Ks)
uniform float uNs;              // shininess (MTL: Ns)

// Optional: if you have spec maps for some meshes
uniform bool uHasSpecMap;

// Cooking visual
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
    // --- Base color
    vec3 kdSafe = (length(uKd) > 0.0001) ? uKd : vec3(1.0);  // avoid default-black when uniform not set
    vec3 albedo = uHasDiffuseMap ? texture(uDiffMap1, fs_in.Tex).rgb : kdSafe;

    vec3 base = applyCooking(albedo, uCookProgress);

    // --- Lighting vectors
    vec3 N = normalize(fs_in.Normal);
    vec3 L = normalize(lightPos - fs_in.WorldPos);
    vec3 V = normalize(viewPos - fs_in.WorldPos);

    float diff = max(dot(N, L), 0.0);

    // --- Specular (Phong)
    vec3 ksSafe = (length(uKs) > 0.0001) ? uKs : vec3(0.04); // sane default
    float nsSafe = (uNs > 1.0) ? uNs : 32.0;

    vec3 specColor = ksSafe;
    if (uHasSpecMap) {
        specColor *= texture(uSpecMap1, fs_in.Tex).rgb;
    }

    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(V, R), 0.0), nsSafe);

    // --- Mix
    vec3 ambient  = 0.18 * base;
    vec3 diffuse  = 0.82 * diff * base;
    vec3 specular = 0.22 * spec * specColor;



    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
