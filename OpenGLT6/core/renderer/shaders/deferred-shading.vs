#version 330 core
layout (location = 0) in vec3 aPos;
#ifdef ON_Texture
layout (location = 1) in vec2 aTexCoords;
#endif

#ifdef ON_Texture
out vec2 TexCoords;
#endif

void main()
{
#ifdef ON_Texture
    TexCoords = aTexCoords;
#endif
    gl_Position = vec4(aPos, 1.0);
}