#version 330 core

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

in vec2 MeshXZ;
in float Height;
in vec3 Normals;
in vec2 TexCoords;

uniform mat4 GroundTranslation;

out mat3 models;
out vec3 norms;
out vec2 PassTexCoord;
varying vec3 Pos_ViewSpace;
varying vec4 LightPosition_Viewspace;
#define tex_multiplier 16

void main(){
	vec4 vPos = vec4(MeshXZ.x, Height, MeshXZ.y, 1.0);

	models = mat3(transpose(inverse( ViewMatrix * GroundTranslation)));
	norms = Normals;
	PassTexCoord = TexCoords * tex_multiplier;
	vec4 WorldPosition = GroundTranslation * vPos;
	gl_ClipDistance[0] = dot(WorldPosition, ClippingPlane);
	Pos_ViewSpace = vec3(ViewMatrix * WorldPosition);
	LightPosition_Viewspace = ViewMatrix * LightPosition;
    gl_Position =  PV_Matrix * WorldPosition;
}
