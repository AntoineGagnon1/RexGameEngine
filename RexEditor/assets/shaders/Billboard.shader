#pragma vertex
#pragma using ModelData
#pragma using SceneData

layout(location = POSITION) in vec3 aPos;
layout(location = TEXCOORDS) in vec2 aUV;

uniform vec3 CameraRightWS;
uniform vec3 CameraUpWS;

out vec2 TexCoords;
out float distance;

void main()
{
	TexCoords = aUV;

	vec2 scale = vec2(length(modelToWorld[0].xyz), length(modelToWorld[1].xyz));
	vec4 worldPos = modelToWorld * vec4(modelToWorld[3].xyz + (CameraRightWS * aPos.x * scale.x) + (CameraUpWS * aPos.y * scale.y), 1);

	distance = length(worldPos.xyz - cameraPos);

	gl_Position = viewToScreen * worldToView * worldPos;
}

#pragma fragment

out vec4 FragColor;
in vec2 TexCoords;
in float distance;

uniform sampler2D texture;

const float FADE_DISTANCE = 25.0f;

void main()
{
	vec4 color = texture(texture, TexCoords);
	color.a *= 1.0f - (distance / FADE_DISTANCE); // Fade when far away 

	FragColor = color;
}