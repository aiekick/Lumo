@SDF
[SLOT:OUT[void:CODE

void Init()
{
	
}

uniform float(0:0.5:0.003) uThick;
uniform float(0:5:1.2) uCoef;
uniform float(0:1:0) uSmooth;

vec3 smoothAbs(vec3 p) {
	return sqrt(p*p + abs(uSmooth));
}

float de( vec3 p ) 
{
	float s=2.,r2;
	p=abs(p);
		for(int i=0; i<12;i++) {
		p=1.-smoothAbs(p-1.);
		r2=uCoef/dot(p,p);
		p*=r2; s*=r2;
	}
	return length(cross(p,normalize(vec3(1))))/s - uThick;
}

float getSDF(vec3 p)
{
	return de(p);
}

vec3 getNormal( vec3 pos, float prec )
{
    vec2 e = vec2( prec, 0. );
    vec3 n = vec3(
        getSDF(pos+e.xyy) - getSDF(pos-e.xyy),
        getSDF(pos+e.yxy) - getSDF(pos-e.yxy),
        getSDF(pos+e.yyx) - getSDF(pos-e.yyx));
    return normalize(n);
}

vec3 getColor(vec3 p, vec3 n)
{
	return n * 0.5 + 0.5;
}

]]
