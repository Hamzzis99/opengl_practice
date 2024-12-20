#version 330 core
layout(location = 0) in vec3 positionAttribute;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(positionAttribute, 1.0);
}
