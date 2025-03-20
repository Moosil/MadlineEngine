//we will be using glsl version 4.5 syntax
#version 450

//const array of positions for the triangle
const vec3 positions[3] = vec3[3](
vec3(1.f,1.f, 0.0f),
vec3(-1.f,1.f, 0.0f),
vec3(0.f,-1.f, 0.0f)
);

void main()
{
	//output the position of each vertex
	gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
}
