#version 330 core

in vec3 vertex_color;

void main()
{
	gl_FragColor = vec4(vertex_color, 1.f);
	//gl_FragColor = vec4(1.f, 1.f, 1.f, 1.f);
}
