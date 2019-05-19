#version 330 core
layout (location = $ID) out vec3 gPosition;

layout (location = $ID) in vec3 aNormal; //@ NormalMap

layout (location = 2) out vec4 gAlbedoSpec; //@ AlbedoMap


in vec2 TexCoords; //@ TexCoords

in vec3 FragPos;

in vec3 Normal; //@ 

uniform sampler2D DiffuseMap; //@ ON

uniform sampler2D SpecularMap; //@ ON


#ifdef ON_Albedo
uniform vec4 albedo;
#endif


void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;

#ifdef ON_NormalsMap
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal); 

#ifdef ON_DiffuseMap
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;
#endif

#ifdef ON_SpecularMap
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(texture_specular1, TexCoords).r;
#endif

#ifdef ON_Albedo
    gAlbedoSpec = albedo;
#endif

}