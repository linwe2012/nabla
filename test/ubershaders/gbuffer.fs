layout (location = $ID) out vec3 gPosition;

layout (location = $ID) out vec3 gNormal;

layout (location = $ID) out vec4 gDiffuseSpec;

layout (location = $ID) out vec3 gAlbedo;

layout (location = $ID) out vec3 gMetaRoughAO;


in vec3 FragPos;

in vec3 NormalMap;

in vec2 TexCoords; //@ TexCoords

uniform sampler2D DiffuseMap; //@ DiffuseMap

uniform sampler2D SpecularMap; //@ SpecularMap

uniform vec3 Diffuse; //@ Diffuse
uniform float Specular; //@ Specular
uniform float Ambient;

uniform vec3 Albedo;
uniform float Metallic;
uniform float Roughness;
uniform float AO;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;

    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(NormalMap);
 
    // and the diffuse per-fragment color
    gDiffuseSpec.rgb = texture(DiffuseMap, TexCoords).rgb; //@ DiffuseMap && TexCoords
    gDiffuseSpec.rgb = Diffuse.rgb; //@ Diffuse

    // store specular intensity in gDiffuseSpec's alpha component
    gDiffuseSpec.a = texture(SpecularMap, TexCoords).r; //@ SpecularMap && TexCoords
    gDiffuseSpec.a = Specular; //@ Specular

    gAlbedo = Albedo;
    gMetaRoughAO.r = Metallic;
    gMetaRoughAO.g = Roughness;
    gMetaRoughAO.b = AO;
}