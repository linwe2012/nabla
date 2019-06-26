layout (location = $ID) in vec3 aPos;

layout (location = $ID) in vec3 aNormalMap; //@ NormalMap

layout (location = $ID) in vec3 aTangent; //@ Bitangent && Tangent

layout (location = $ID) in vec3 aBitangent; //@ Bitangent && Tangent

layout (location = $ID) in vec2 aTexCoords; //@ TexCoords



out vec3 FragPos;

out vec3 NormalMap;

out vec2 TexCoords; //@ TexCoords

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 world_pos = model * vec4(aPos, 1.0);
    FragPos = world_pos.xyz;

    TexCoords = aTexCoords; //@ TexCoords

    mat3 normalMatrix = transpose(inverse(mat3(model))); //@ NormalMap
    NormalMap = normalMatrix * aNormalMap; //@ NormalMap
    NormalMap = (0, 0, 0); //@ !NormalMap


    gl_Position = projection * view * world_pos;
}
