@COMMON

//uniform float(0:2:0.5) disk_radius;
#define disk_radius 0.5
float disk(vec2 p)
{
	return length(p) - disk_radius; 
}

//uniform vec2(0:2:0.5) quad_size;
//uniform float(0:5:0.01) quad_corner;
#define quad_size 0.5
#define quad_corner 0.01
float quad(vec2 p)
{
	return length(max(abs(p) - quad_size, 0.0)) - quad_corner; 
}
