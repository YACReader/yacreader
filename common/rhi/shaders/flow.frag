#version 450

// Inputs from vertex shader
layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec4 vColor;
layout(location = 2) in float vIsReflection;

// Output
layout(location = 0) out vec4 fragColor;

// Uniform buffer
layout(std140, binding = 0) uniform UniformBuffer
{
    mat4 viewProjectionMatrix;
    vec3 backgroundColor;
    float _pad0;
    vec3 shadingColor;
    float _pad1;
    float reflectionUp;
    float reflectionDown;
    int isReflection;
    float _pad2;
};

// Texture and sampler
layout(binding = 1) uniform sampler2D coverTexture;

void main()
{
    vec4 texColor = texture(coverTexture, vTexCoord);

    // Apply shading: multiply texture by vColor.r to darken
    float shadingAmount = vColor.r;

    // For reflections, apply gradient fade (darker at bottom, fading to black)
    if (vIsReflection > 0.5) {
        // vTexCoord.y goes from 1 (top of reflection, near cover) to 0 (bottom, far from cover)
        // We want it brightest near the cover and fading away
        float gradientFade = mix(0.0, 0.33, vTexCoord.y);
        shadingAmount *= gradientFade;
    }

    fragColor = vec4(texColor.rgb * shadingAmount, texColor.a);
}
