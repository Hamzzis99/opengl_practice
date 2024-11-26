#version 330 core
layout(location = 0) in vec3 position; // 위치 속성
layout(location = 1) in vec3 color;    // 색상 속성

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 ourColor; // 프래그먼트 셰이더로 색상 전달

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    ourColor = color;
}
