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
layout(location = 2) out flat vec4 vInstanceShading;
layout(location = 3) out flat float vInstanceOpacity;
layout(location = 4) out flat float vIsReflection;
layout(location = 5) out vec2 vLocalPos;

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

void main()
{
    // Reconstruct instance model matrix from 4 vec4 rows
    mat4 instanceModel = mat4(instanceModel_row0, instanceModel_row1, instanceModel_row2, instanceModel_row3);

    gl_Position = viewProjectionMatrix * instanceModel * vec4(position, 1.0);
    vTexCoord = texCoord;
    vLocalPos = position.xy;

    // Flip texture vertically per-instance when requested (reflection)
    if (instanceFlip != 0.0) {
        vTexCoord.y = 1.0 - vTexCoord.y;
    }

    // Pass per-instance values to fragment shader as flat (no interpolation)
    vInstanceShading = instanceShading1;
    vInstanceOpacity = instanceOpacity;
    vIsReflection = instanceFlip;
}
