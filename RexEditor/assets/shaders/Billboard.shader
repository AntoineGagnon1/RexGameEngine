#pragma vertex
#pragma using ModelData
#pragma using SceneData

layout(location = POSITION) in vec3 aPos;
layout(location = TEXCOORDS) in vec2 aUV;

uniform vec3 CameraRightWS;
uniform vec3 CameraUpWS;

out vec2 TexCoords;

void main()
{
	TexCoords = aUV;
	vec3 worldPos = modelToWorld[3].xyz;
	vec2 scale = vec2(length(modelToWorld[0].xyz), length(modelToWorld[1].xyz));
	gl_Position = viewToScreen * worldToView * modelToWorld * vec4(worldPos + (CameraRightWS * aPos.x * scale.x) + (CameraUpWS * aPos.y * scale.y), 1);
}

#pragma fragment

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D texture;

void main()
{
	FragColor = texture(texture, TexCoords);
}