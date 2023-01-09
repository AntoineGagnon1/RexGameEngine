#pragma vertex

#pragma using SceneData // Get the scene data (viewMatrix, projectionMatrix)
#pragma using ModelData // Get the model data (modelMatrix)

layout(location = POSITION) in vec3 aPos;
layout(location = NORMAL) in vec3 aNormal;

out vec3 normal;
out vec3 worldPos;

void main()
{ 
	normal = mat3(modelToWorld) * aNormal;
	worldPos = vec3(modelToWorld * vec4(aPos, 1.0));
	gl_Position = viewToScreen * worldToView * modelToWorld * vec4(aPos, 1.0);
}


#pragma fragment

#pragma using Lighting
#pragma using SceneData
#pragma using PBR

in vec3 normal;
in vec3 worldPos;
out vec4 FragColor; 

[Slider(0, 1)]uniform vec3  albedo;
[Slider(0, 1)]uniform float metallic;
[Slider(0, 1)]uniform float roughness;
[Slider(0, 1)]uniform float ao;

void main()
{
    FragColor = vec4(GetPBRColorLit(albedo, metallic, roughness, ao, worldPos, cameraPos, normal), 1.0);
}
	