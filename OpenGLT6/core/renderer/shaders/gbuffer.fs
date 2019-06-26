#version 330 core
layout (location = 0) out vec3 gPosition;

#ifdef ON_NormalsMap
layout (location = 1) in vec3 aNormal;
#endif

#ifdef ON_Albedo || ON_AlbedoMap
layout (location = 2) out vec4 gAlbedoSpec;
#endif

#ifdef ON_Texture
in vec2 TexCoords;
#endif

in vec3 FragPos;

#ifdef ON_NormalsMap
in vec3 Normal;
#endif

#ifdef ON_DiffuseMap
uniform sampler2D texture_diffuse1;
#endif

#ifdef ON_SpecularMap
uniform sampler2D texture_specular1;
#endif

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
#endif 

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