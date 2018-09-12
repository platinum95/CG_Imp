#version 330                                                                        

layout (location = 0) out vec4 FragColour;
layout (location = 1) out vec4 BrightColour;

in mat3 models;
in vec4 col;
varying vec3 Pos_ViewSpace;
varying vec4 LightPosition_Viewspace;

uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;

layout (std140) uniform LightData
{ 
	vec4 LightPosition;
	vec3 LightColour;
	float Brightness;
};

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} fs_in; 

void main(){
	// ambient
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * LightColour; 

	//Diffuse
	vec3 norm = texture(normalTexture, fs_in.TexCoords).rgb;
	norm = normalize(norm * 2.0 - 1.0);   
	norm = normalize(fs_in.TBN * norm); 

	vec3 lightDir = vec3(normalize(LightPosition_Viewspace.xyz - Pos_ViewSpace));
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * LightColour;
	diffuse = diffuse * Brightness;

	//Specular
	vec3 CamDir = normalize(vec3(0,0,0) - Pos_ViewSpace);
	vec3 ReflectDir = reflect(-lightDir, norm);
	float SpecAmount = pow(max(dot(CamDir, ReflectDir), 0.0), 16);
	float SpecWeight = texture(specularTexture, fs_in.TexCoords).x;
	vec3 SpecularComponent = SpecWeight * SpecAmount * LightColour;  



	//Output
	vec3 result = (ambient + diffuse + SpecularComponent) * texture(diffuseTexture, fs_in.TexCoords).xyz;
	FragColour = vec4(result, 1.0);
	BrightColour = vec4(0.0, 0.0, 0.0, 1.0);
}
