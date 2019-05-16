#version 330 core
layout (location = 0) in vec3 aPos;

#ifdef ON_NormalsMap
layout (location = 1) in vec3 aNormal;
#endif

#ifdef ON_Texture
layout (location = 2) in vec2 aTexCoords;
#endif

out vec3 FragPos;

#ifdef ON_Texture
out vec2 TexCoords;
#endif

#ifdef ON_NormalsMap
out vec3 Normal;
#endif

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 world_pos = model * vec4(aPos, 1.0);
    FragPos = world_pos.xyz;

#ifdef ON_Texture
    TextCoords = aTexCoords;
#endif

#ifdef ON_NormalsMap
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalMatrix * aNormal;
#endif

    gl_Position = projection * view * worldPos;
}
