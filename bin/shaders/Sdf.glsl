@VERTEX QUAD()

layout(location = 0) out vec2 vertUV;

void main() 
{
	float vertexId = float(gl_VertexIndex);
	vec2 p = vec2(0);
	if (gl_VertexIndex == 0) p = vec2(0,0);
	else if (gl_VertexIndex == 1) p = vec2(0,1);
	else if (gl_VertexIndex == 2) p = vec2(1,1);
	else if (gl_VertexIndex == 3) p = vec2(0,0);
	else if (gl_VertexIndex == 4) p = vec2(1,1);
	else if (gl_VertexIndex == 5) p = vec2(1,0);
	p = p * 2.0 - 1.0;
	gl_Position = vec4(p,0,1);
	vertUV = p;
}

@FRAGMENT

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 vertUV;

uniform(hidden) mat4(camera:m) model;
uniform(hidden) mat4(camera:v) view;
uniform(hidden) mat4(camera:p) proj;
uniform(Ray Marching) int(1:200:50) rmIteractions;
uniform(Ray Marching) float(0:1:0.9) rmPrec;
uniform(Ray Marching) float(0:100:50) rmFar;

//////////////////////////////////////////////////////////

vec3 BUILTIN_GET_RAY_ORIGIN()
{
	vec3 ro = view[3].xyz + model[3].xyz;
	ro *= mat3(model);
	return -ro;
}

vec3 BUILTIN_GET_RAY_DIRECTION()
{
	vec4 ray_clip = vec4(vertUV.x, vertUV.y, -1, 1);
	vec4 ray_eye = inverse(proj) * ray_clip;
	vec3 rd = normalize(vec3(ray_eye.x, ray_eye.y, 1.));
	rd *= mat3(model);
	return -rd;
}

void BUILTIN_CALC_DEPTH(vec3 vRayDir, float vDist)
{
	vec3 eyeFwd = vec3(0., 0., -1.) * mat3(model);
	float depth = proj[3].z * dot(vRayDir, eyeFwd) / vDist - proj[2].z;
	gl_FragDepth = mix(0.0, 1.0, depth);
}

//////////////////////////////////////////////////////////
// [KEY[STAMP:NAME:DESCRIPTION
// DEFAULT_CODE_BLOCK
// ]]
		
[SLOT[global:global
/* 
	your global code
*/
]]

[SLOT[void:Init:init will be called before Ray Marched Loop
void Init()
{
/* 
	your init code
*/
}
]]

[SLOT[float(vec3):voxelSdf:will compute the voxelsdf
//uniform(shape)	float(0:2:1) uRadius;
float voxelSdf(vec3 p)
{
	// ex simple sdf sphere */
    return length(p) - 1.0;
}
]]

[SLOT[vec3(vec3,float):voxelNormal:will compute the normal of the current voxel sdf
vec3 voxelNormal(vec3 p, float prec)
{
	/* ex sdf normal */
	vec3 e = vec3(prec, 0, 0);
	return normalize(vec3(
		voxelSdf(p+e)-voxelSdf(p-e),
		voxelSdf(p+e.yxz)-voxelSdf(p-e.yxz),
		voxelSdf(p+e.zyx)-voxelSdf(p-e.zyx)));
}
]]

[SLOT[vec3(vec3,vec3):voxelColor:will compute the color of the current voxel
vec3 voxelColor(vec3 p, vec3 n)                                             
{
	/* ex basic normal color */
	return n * 0.5 + 0.5;
}
]]	

//////////////////////////////////////////////////////////

void main(void)
{
	Init();
	
	vec3 ro = BUILTIN_GET_RAY_ORIGIN();
	vec3 rd = BUILTIN_GET_RAY_DIRECTION();

	vec3 col = vec3(0.0);

	float s = 1., d = 0., md = rmFar;
	for (int i = 0; i < rmIteractions; i++)
	{
		if (s < 0.001 || d > md) break;
		s = voxelSdf(ro + rd * d);
		d += s * rmPrec;
	}
	
	if (d < md)
	{
		BUILTIN_CALC_DEPTH(rd, d);
		
		vec3 p = ro + rd * d;
		vec3 n = voxelNormal(p, 0.01);
		col = voxelColor(p, n);
	}
	else
	{
		discard;
	}
	
	fragColor = vec4(col,1);
}
