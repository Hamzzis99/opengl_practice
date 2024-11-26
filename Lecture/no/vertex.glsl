#version 330 core
layout (location = 0) in vec3 vPos; //--- 응용 프로그램에서 받아온 도형 좌표값
layout (location = 1) in vec3 in_Color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 color;

out vec3 out_Color;
void main()
{
	gl_Position = projection * view * model * vec4(vPos, 1.0);
	
	out_Color = color;
}
