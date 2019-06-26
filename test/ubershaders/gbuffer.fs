layout (location = $ID) out vec3 gPosition;

layout (location = $ID) out vec3 gNormal;

layout (location = $ID) out vec4 gDiffuseSpec;

layout (location = $ID) out vec3 gAlbedo;

layout (location = $ID) out vec3 gMetaRoughAO;

layout (location = $ID) out vec3 gEntity;

in vec3 FragPos;

in vec3 NormalMap;

in vec2 TexCoords; //@ TexCoords

uniform vec3 Entity;

uniform sampler2D DiffuseMap; //@ DiffuseMap

uniform sampler2D SpecularMap; //@ SpecularMap

uniform sampler2D AlbedoMap; //@ AlbedoMap

uniform sampler2D MetallicMap; //@ MetallicMap

uniform sampler2D RoughnessMap; //@ RoughnessMap

uniform sampler2D AOMap; //@ AOMap

uniform vec3 Diffuse; //@ Diffuse
uniform float Specular; //@ Specular

uniform vec3 Albedo; //@ !AlbedoMap
uniform float Metallic; //@ !MetallicMap
uniform float Roughness; //@ !RoughnessMap
uniform float AO; //@ !AOMap



void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;

    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(NormalMap);
 
    // and the diffuse per-fragment color
    // gDiffuseSpec.rgb = vec3(TexCoords, 0.0); //@ DiffuseMap && TexCoords
    gDiffuseSpec.rgb = texture(DiffuseMap, TexCoords).rgb; //@ DiffuseMap && TexCoords
    gDiffuseSpec.rgb = Diffuse.rgb; //@ Diffuse

    // store specular intensity in gDiffuseSpec's alpha component
    gDiffuseSpec.a = texture(SpecularMap, TexCoords).r; //@ SpecularMap && TexCoords
    gDiffuseSpec.a = Specular; //@ Specular

    gAlbedo = Albedo; //@ !AlbedoMap
    gMetaRoughAO.r = Metallic; //@ !MetallicMap
    gMetaRoughAO.g = Roughness;  //@ !RoughnessMap
    gMetaRoughAO.b = AO; //@ !AOMap

    gAlbedo = texture(AlbedoMap, TexCoords).rgb; //@ AlbedoMap
    gMetaRoughAO.r = texture(MetallicMap, TexCoords).r; //@ MetallicMap
    gMetaRoughAO.g = texture(RoughnessMap, TexCoords).r;  //@ RoughnessMap
    gMetaRoughAO.b = texture(AOMap, TexCoords).r; //@ AOMap

    gEntity = Entity;
}