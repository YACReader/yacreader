#version 450

// Per-vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

// Per-instance attributes (mat4 split into 4 vec4s for better D3D11 compatibility)
layout(location = 2) in vec4 instanceModel_row0;
layout(location = 3) in vec4 instanceModel_row1;
layout(location = 4) in vec4 instanceModel_row2;
layout(location = 5) in vec4 instanceModel_row3;
layout(location = 6) in vec4 instanceShading1;
layout(location = 7) in float instanceOpacity;
layout(location = 8) in float instanceFlip;

// Outputs to fragment shader
layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec4 vColor;
layout(location = 2) out float vIsReflection;

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

void main()
{
    // Reconstruct instance model matrix from 4 vec4 rows
    mat4 instanceModel = mat4(instanceModel_row0, instanceModel_row1, instanceModel_row2, instanceModel_row3);

    gl_Position = viewProjectionMatrix * instanceModel * vec4(position, 1.0);
    vTexCoord = texCoord;

    // Flip texture vertically per-instance when requested (reflection)
    if (instanceFlip != 0.0) {
        vTexCoord.y = 1.0 - texCoord.y;
    }

    float leftUpShading = instanceShading1.x;
    float leftDownShading = instanceShading1.y;
    float rightUpShading = instanceShading1.z;
    float rightDownShading = instanceShading1.w;

    float leftShading = mix(leftDownShading, leftUpShading, (position.y + 0.5));
    float rightShading = mix(rightDownShading, rightUpShading, (position.y + 0.5));
    float shading = mix(leftShading, rightShading, (position.x + 0.5));

    vColor = vec4(shading * instanceOpacity);
    vIsReflection = instanceFlip;
}
