#pragma version 420 core // Will default to version 330 core if not specified (4.2 is also the minimum version)

#pragma vertex

#pragma using SceneData // Get the scene data (viewMatrix, projectionMatrix)
#pragma using ModelData // Get the model data (modelMatrix)

layout(location = POSITION) in vec3 aPos;
out vec3 pos;

void main()
{ 
	pos = aPos;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}


#pragma fragment
in vec3 pos;
out vec4 FragColor; 

void main()
{ 
	FragColor = vec4(pos, 1.0f);
}