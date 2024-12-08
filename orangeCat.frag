#version 120

uniform sampler2D uTexture;     // Diffuse texture
uniform float uKa, uKd, uKs;    // Lighting coefficients
uniform float uShininess;       // Shininess for specular highlights

varying vec2 vST;               // Texture coordinates
varying vec3 vN;                // Normal vector
varying vec3 vL;                // Vector to light
varying vec3 vE;                // Vector to eye

const vec3 SPECULARCOLOR = vec3(0.925, 0.813, 0.582); // Specular highlight color

void main() {
    // Sample the diffuse texture
    vec3 textureColor = texture2D(uTexture, vST).rgb;

    // Normalize vectors
    vec3 Normal = normalize(vN);
    vec3 Light = normalize(vL);
    vec3 Eye = normalize(vE);

    // Ambient lighting
    vec3 ambient = uKa * textureColor;

    // Diffuse lighting
    float d = max(dot(Normal, Light), 0.0); // Only if light sees the point
    vec3 diffuse = uKd * d * textureColor;

    // Specular lighting
    float s = 0.0;
    if (d > 0.0) {
        vec3 reflection = normalize(reflect(-Light, Normal));
        float cosphi = dot(Eye, reflection);
        if (cosphi > 0.0) {
            s = pow(cosphi, uShininess);
        }
    }
    vec3 specular = uKs * s * SPECULARCOLOR;

    // Final color
    gl_FragColor = vec4(ambient + diffuse + specular, 1.0);
}
