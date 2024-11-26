#version 330 core
layout(location = 0) in vec3 position; // ��ġ �Ӽ�
layout(location = 1) in vec3 color;    // ���� �Ӽ�

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 ourColor; // �����׸�Ʈ ���̴��� ���� ����

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    ourColor = color;
}
