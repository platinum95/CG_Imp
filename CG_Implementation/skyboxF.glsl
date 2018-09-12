#version 330 

in vec3 textureCoords;
layout (location = 0) out vec4 FragColour;
layout (location = 1) out vec4 BrightColour;

uniform samplerCube BoxTexture;

void main(void){
    FragColour = texture(BoxTexture, textureCoords);
	BrightColour = vec4(0.0, 0.0, 0.0, 1.0);
}
