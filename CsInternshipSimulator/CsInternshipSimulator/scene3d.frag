#version 330 core
in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D uTex0;
uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform int uLightEnabled;

void main(){
    vec3 color = texture(uTex0, vTex).rgb;
    if(uLightEnabled==0){
        FragColor = vec4(color,1.0);
        return;
    }

    vec3 ambient = 0.25 * color;
    vec3 n = normalize(vNormal);
    vec3 l = normalize(uLightPos - vFragPos);
    float diff = max(dot(n,l),0.0);
    vec3 diffuse = diff * color;
    vec3 viewDir = normalize(uViewPos - vFragPos);
    vec3 reflectDir = reflect(-l,n);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    vec3 specular = vec3(0.25) * spec;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
