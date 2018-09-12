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

uniform mat4 model;


void main(){
	
	vec4 WorldPosition = model * vec4(vPosition, 1.0);
	gl_ClipDistance[0] = dot(WorldPosition, ClippingPlane);
    gl_Position =  PV_Matrix * WorldPosition;
}