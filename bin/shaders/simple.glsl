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

//////////////////////////////////////////////////////////
// [KEY[STAMP:NAME:DESCRIPTION
// DEFAULT_CODE_BLOCK
// ]]

[SLOT[float(vec2):shape:will compute the shape
float shape(vec2 p)                                             
{
	return length(p) - 1.0;
}
]]	

//////////////////////////////////////////////////////////

void main(void)
{
	vec2 uv = vertUV;
	
	float d = shape(uv);
	float c = smoothstep(0.0, 0.1, d);
	
	fragColor = vec4(c, c, c, 1.0);
}
