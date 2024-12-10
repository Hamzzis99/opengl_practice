#version 330 core

layout(location = 0) in vec3 positionAttribute;
layout(location = 1) in vec3 normalAttribute;
layout(location = 2) in vec2 uvAttribute;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 transform;
uniform mat4 viewTransform;
uniform mat4 projectionTransform;

void main()
{
    FragPos = vec3(transform * vec4(positionAttribute, 1.0));
    Normal = mat3(transpose(inverse(transform))) * normalAttribute;
    TexCoords = uvAttribute;
    
    gl_Position = projectionTransform * viewTransform * transform * vec4(positionAttribute, 1.0);
}
