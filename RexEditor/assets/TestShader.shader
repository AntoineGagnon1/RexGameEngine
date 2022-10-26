#pragma version 330 core // Will default to version 330 core if not specified

#pragma vertex
layout(location = POSITION) in vec3 aPos;

void main()
{ 
	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}


#pragma fragment
out vec4 FragColor; 

void main()
{ 
	FragColor = vec4(0.0f, 0.0f, 1.0f, 1.0f); 
}