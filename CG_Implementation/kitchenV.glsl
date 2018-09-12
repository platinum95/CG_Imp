#version 330


layout (std140) uniform CameraProjectionData
{ 
  mat4 ViewMatrix;
  mat4 ProjectionMatrix;
  mat4 PV_Matrix;
  vec4 CameraPosition;
  vec4 CameraOrientation;
  vec4 ClippingPlane;
};

layout (std140) uniform LightData
{ 
	vec4 LightPosition;
	vec3 LightColour;
	float Brightness;
};

in vec3 vPosition;
in vec3 vNormal;
in vec2 TexCoord;
out mat3 models;
out vec3 norms;
out vec2 TexCoords;

uniform mat4 model;
varying vec3 Pos_ViewSpace;
varying vec4 LightPosition_Viewspace;

void main(){
	models = mat3(transpose(inverse( ViewMatrix * model)));
	norms = vNormal;
	TexCoords = TexCoord;
	vec4 WorldPosition = model * vec4(vPosition, 1.0);
	gl_ClipDistance[0] = dot(WorldPosition, ClippingPlane);

	Pos_ViewSpace = vec3(ViewMatrix * WorldPosition);
	LightPosition_Viewspace = ViewMatrix * LightPosition;
    gl_Position =  PV_Matrix * WorldPosition;
}