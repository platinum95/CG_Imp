
#version 330 

in vec3 vPosition;
out vec3 textureCoords;


layout (std140) uniform CameraProjectionData
{ 
  mat4 ViewMatrix;
  mat4 ProjectionMatrix;
  mat4 PV_Matrix;
  vec4 CameraPosition;
  vec4 CameraOrientation;
  vec4 ClippingPlane;
};


mat4 scaler = mat4(
		1000, 0, 0, 0,
		0, 1000, 0, 0,
		0, 0, 1000, 0,
		0, 0, 0, 1);


void main(void){
	vec4 currentPos = scaler * vec4(vPosition, 1.0);
	mat4 view2 = ViewMatrix;
	view2[3][0] = 0;
	view2[3][1] = 0;
	view2[3][2]= 0;
	textureCoords = vPosition;
	gl_Position = ProjectionMatrix * view2 * currentPos;
}
