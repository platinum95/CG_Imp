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
in vec3 vTangeant;
in vec3 vBitangeant;
in vec2 TexCoord;
in vec4 BoneWeights;
in uvec4 BoneIDs;

out mat3 models;
out vec4 col;
out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    mat3 TBN;
} vs_out;  

uniform mat4 BoneMatrices[56];
uniform mat4 model;
varying vec3 Pos_ViewSpace;
varying vec4 LightPosition_Viewspace;
//uniform mat4 BoneMatrices;

#define VAL 5 

vec4 sanityCheck(mat4 checkit){
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			if(checkit[i][j] > VAL || checkit[i][j] < -VAL)
				return vec4(1.0, 1.0, 1.0, 1.0);
		}
	}
	return vec4(0.0, 0.0, 0.0, 1.0);
}

vec4 sanityCheckVec(vec4 checkit){
	for(int i = 0; i < 4; i++){
		if(checkit[i] > 100 )
			return vec4(1.0, 1.0, 1.0, 1.0);
		
	}
	return vec4(0.0, 0.0, 0.0, 1.0);
}

vec4 checkWeight(vec4 w){
	float sum = BoneWeights.x + BoneWeights.y + BoneWeights.z + BoneWeights.w;
	sum = sum - 1.0;
	if(abs(sum) < 0.001)
		return vec4(0.5, 0.5, 0.5, 1.0);
	if(sum > 0)
		return vec4(1, 1, 1, 1.0);
	else{
		sum = BoneWeights.z + BoneWeights.y + BoneWeights.x + BoneWeights.w;
		return vec4(sum, sum, sum, 1.0);

	}
}

void main(){

	mat4 BMatrix = mat4(1.0);
	
	BMatrix = BoneMatrices[BoneIDs.x] * BoneWeights.x;
	BMatrix += BoneMatrices[BoneIDs.y] * BoneWeights.y;
	BMatrix += BoneMatrices[BoneIDs.z] * BoneWeights.z;
	BMatrix += BoneMatrices[BoneIDs.w] * BoneWeights.w;
	
	float val = float(BoneIDs.x) / 56.0;
	mat4 TrueModel = model * BMatrix;
	mat4 test =  BMatrix;
	col = sanityCheck(test);

	models = mat3(transpose(inverse( ViewMatrix * TrueModel)));
	vec3 T = normalize(models * vTangeant);
	vec3 B = normalize(models * vBitangeant);
	vec3 N = normalize(models * vNormal);
	vs_out.TBN = mat3(T, B, N);
	vs_out.TexCoords = TexCoord;
	vec4 WorldPosition = TrueModel * vec4(vPosition, 1.0);
	gl_ClipDistance[0] = dot(WorldPosition, ClippingPlane);

	Pos_ViewSpace = vec3(ViewMatrix * WorldPosition);
	LightPosition_Viewspace = ViewMatrix * LightPosition;
    gl_Position =  PV_Matrix * WorldPosition;
//	col = sanityCheckVec(PV_Matrix * test * vec4(vPosition, 1.0));//vec4(val, val, val, 1.0);
	vs_out.FragPos = gl_Position.xyz;
}
