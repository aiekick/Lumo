@NODE

NAME(2D Shape)

@VERTEX QUAD()

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aUV;
layout(location = 0) out vec2 vertUV;

void main() 
{
	gl_Position = vec4(aPosition,0,1);
	vertUV = aUV;
}

@FRAGMENT

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec2 vertUV;

[SLOT[float(vec2):shape:will compute the shape
uniform float(0:2:0.5) shape_radius;
//#define shape_radius 0.95
float shape(vec2 p)
{
	return length(p) - shape_radius; 
}
]]	

//////////////////////////////////////////////////////////

void main(void)
{
	vec2 uv = vertUV * 2.0 - 1.0;
	
	float d = shape(uv);
	float c = smoothstep(0.0, 0.025, d);
	
	fragColor = vec4(c, c, c, 1.0);
}
