#version 330 core
in vec3 ourColor; // vertex 셰이더로부터 색상 전달
out vec4 FragColor;

void main()
{
    FragColor = vec4(ourColor, 1.0);
}
