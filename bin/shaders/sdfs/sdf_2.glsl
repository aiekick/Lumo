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

float de(vec3 pos)
{
    vec3 tpos=pos;
    tpos.xz=abs(.5-mod(tpos.xz,1.));
    vec4 p=vec4(tpos,1.);
    float y=max(0.,.35-abs(pos.y-3.35))/.35;
    for (int i=0; i<7; i++) {
		p.xyz = abs(p.xyz)-vec3(-0.02,1.98,-0.02);
		p=p*(2.0+0.*y)/clamp(dot(p.xyz,p.xyz),.4,1.)-vec4(0.5,1.,0.4,0.);
		p.xz*=mat2(-0.416,-0.91,0.91,-0.416);
    }
	return (length(max(abs(p.xyz)-vec3(0.1,5.0,0.1),vec3(0.0)))-0.05)/p.w;
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
