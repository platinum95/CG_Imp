#version 330
in vec3 vPosition;
in vec2 TexCoord;

out vec2 PassTexCoord;


void main(){
	gl_Position = vec4(vPosition, 1);
	PassTexCoord = TexCoord;
}