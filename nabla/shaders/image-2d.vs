#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
out vec3 TexCoords;

void main()
{
    TexCoords = model * aPos;
    gl_Position =  vec4(aPos, 1.0);
}
