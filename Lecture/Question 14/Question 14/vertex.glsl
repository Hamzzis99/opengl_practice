#version 330 core

layout(location = 0) in vec3 positionAttribute;
layout(location = 1) in vec3 colorAttribute;

uniform mat4 transform;

out vec3 ourColor;

void main()
{
    gl_Position = transform * vec4(positionAttribute, 1.0);
    ourColor = colorAttribute;
}
