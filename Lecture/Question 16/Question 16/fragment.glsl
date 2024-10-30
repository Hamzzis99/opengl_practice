#version 330 core

in vec3 passColor;
out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(passColor, 1.0);
}
