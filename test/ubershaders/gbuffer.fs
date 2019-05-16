layout (location = $ID) out vec3 gPosition;

layout (location = $ID) out vec3 gNormal;

layout (location = $ID) out vec4 gAlbedoSpec;


in vec3 FragPos;

in vec3 NormalMap;

in vec2 TexCoords; //@ TexCoords

uniform sampler2D DiffuseMap; //@ DiffuseMap

uniform sampler2D SpecularMap; //@ SpecularMap

uniform vec4 albedo; //@ Albedo

uniform vec3 Diffuse; //@ Diffuse
uniform float Specular; //@ Specular
uniform float Ambient;
void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;

    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(NormalMap);
 
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(DiffuseMap, TexCoords).rgb; //@ DiffuseMap && TexCoords
    gAlbedoSpec.rgb = Diffuse.rgb; //@ Diffuse

    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(SpecularMap, TexCoords).r; //@ SpecularMap && TexCoords
    gAlbedoSpec.a = Specular; //@ Specular

    gAlbedoSpec = albedo; //@ Albedo
}