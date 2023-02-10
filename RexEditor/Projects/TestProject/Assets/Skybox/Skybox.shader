#pragma version 420 core

#pragma vertex
#pragma using SceneData
#pragma using ModelData
layout(location = POSITION) in vec3 aPos;

out vec3 TexCoords;


void main()
{
    TexCoords = aPos;
    vec4 pos = viewToScreen * worldToView * modelToWorld * vec4(aPos, 1.0);
    gl_Position = pos.xyww; // Trick to always set the z component to 1
}

#pragma fragment

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    FragColor = vec4(texture(skybox, TexCoords).rgb, 1.0f);
}