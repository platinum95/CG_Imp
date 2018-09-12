#version 330  

#define SCALE 500

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
out vec4 ClipspaceCoord;
out vec2 TexCoord;
out mat3 models;
out vec3 norms;
out vec2 TexCoords;
out vec3 Pos_ViewSpace;
out vec4 LightPosition_Viewspace;


void main(){
	vec4 vertexPos = vec4(vPosition * SCALE, 1);

	TexCoord = vec2((vPosition.x + 1.0) / 2.0, (vPosition.z + 1.0) / 2.0);
	ClipspaceCoord = PV_Matrix * vertexPos;
	gl_Position = ClipspaceCoord;

}