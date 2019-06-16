#version 330 core
uniform sampler2D image;
out vec4 FragColor;

in vec3 TexCoords;

void main()
{
    FragColor = texture(image, TexCoords);
}