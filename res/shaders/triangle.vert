#version 330 core

layout (location = 0) in vec3 position;
//layout (location = 1) in vec3 color;
layout (location = 1) in vec3 normal;
//layout (location = 2) in vec2 tex_coord;

out vec3 vertex_color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{	
	gl_Position = projection * view * model * vec4(position, 1.f);
	//vertex_color = color;
	vertex_color = normal;
	//vertex_color = vec3(tex_coord, 0.f);
}
