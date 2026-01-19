#version 450

// Inputs from vertex shader
layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in flat vec4 vInstanceShading;
layout(location = 2) in flat float vInstanceOpacity;
layout(location = 3) in flat float vIsReflection;
layout(location = 5) in vec2 vLocalPos;

// Output
layout(location = 0) out vec4 fragColor;

// Uniform buffer
layout(std140, binding = 0) uniform UniformBuffer
{
    mat4 viewProjectionMatrix;
    vec3 backgroundColor;
    float _pad0;
    float reflectionUp;
    float reflectionDown;
    int isReflection;
    float _pad1;
};

// Texture and sampler
layout(binding = 1) uniform sampler2D coverTexture;

void main()
{
    vec4 texColor = texture(coverTexture, vTexCoord);

    // Compute per-pixel shading using per-instance corner shading values
    float leftUp = vInstanceShading.x;
    float leftDown = vInstanceShading.y;
    float rightUp = vInstanceShading.z;
    float rightDown = vInstanceShading.w;

    // Use interpolated local vertex position to match original vertex-based shading.
    // position.y in vertex ranges [-0.5..0.5], so map to [0..1] with +0.5
    float y = clamp(vLocalPos.y + 0.5, 0.0, 1.0);
    float x = clamp(vLocalPos.x + 0.5, 0.0, 1.0);
    float leftShading = mix(leftDown, leftUp, y);
    float rightShading = mix(rightDown, rightUp, y);
    float shading = mix(leftShading, rightShading, x);

    // Combine with per-instance opacity (opacity only darkens RGB, alpha preserved)
    float shadingAmount = shading * vInstanceOpacity;

    // For reflections, apply a gradient fade using reflection uniforms (darker further away)
    if (vIsReflection > 0.5) {
        float gradientFade = mix(reflectionUp, reflectionDown, vTexCoord.y);
        shadingAmount *= gradientFade;
    }

    // Subtle dithering to break up bands (very low amplitude, within 8-bit quantization)
    float rnd = fract(sin(dot(gl_FragCoord.xy, vec2(12.9898,78.233))) * 43758.5453);
    // scale to roughly +/- 0.5/255 range (adjust strength if needed)
    float dither = (rnd - 0.5) / 255.0;

    float shadedAmountDithered = clamp(shadingAmount + dither, 0.0, 1.0);

    fragColor = vec4(
        mix(backgroundColor, texColor.rgb, shadedAmountDithered),
        texColor.a
    );
}
