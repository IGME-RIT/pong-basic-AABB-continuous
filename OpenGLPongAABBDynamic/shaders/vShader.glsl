

#version 430

layout (location=0) in vec3 position;
uniform mat4 worldMatrix;


void main()
{
 // gl_Position is inbuilt vertex shader output variable of type vec4.
	
	gl_Position  = worldMatrix * vec4(position,1);



}
