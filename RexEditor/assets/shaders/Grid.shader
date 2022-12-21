// Shader for the SceneView grid
#pragma vertex
#pragma using SceneData
#pragma using ModelData

layout(location = POSITION) in vec3 aPos;

out vec2 WorldPos; // x and z coords
out vec2 PlayerPos;

void main()
{
    PlayerPos = vec2(modelToWorld[3][0], modelToWorld[3][2]);
    WorldPos = (modelToWorld * vec4(aPos, 1.0)).xz;
    gl_Position = viewToScreen * worldToView * modelToWorld * vec4(aPos, 1.0);
}

#pragma fragment

#extension GL_OES_standard_derivatives : enable
out vec4 FragColor;

in vec2 WorldPos;
in vec2 PlayerPos;

uniform float falloffDistance;
uniform int gridSize;

void main()
{
    // License: CC0 (http://creativecommons.org/publicdomain/zero/1.0/)
    // From : https://madebyevan.com/shaders/grid/

    // Compute anti-aliased world-space grid lines
    vec2 coord = WorldPos / gridSize;

    vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
    float line = min(grid.x, grid.y);

    // Line color and alpha
    float color = 1.0 - min(line, 1.0);
    float alpha = (line == 0.0f ? 0.0f : 1.0f);

    // Fade if far away
    float distance = length(WorldPos - PlayerPos) / falloffDistance;
    alpha *= 1.0f - pow(distance, 0.5f);

    // Apply gamma correction
    color = pow(color, 1.0 / 1.5);

    if(abs(WorldPos.y) < 0.1f) // X axis (red)
        FragColor = vec4(vec3(color, 0.0f, 0.0f), alpha);
    else if (abs(WorldPos.x) < 0.1f) // Z axis (blue)
        FragColor = vec4(vec3(0.0f, 0.0f, color), alpha);
    else
        FragColor = vec4(vec3(color), alpha);
}