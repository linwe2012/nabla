out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuseSpec;
uniform sampler2D gAlbedo;
uniform sampler2D gMetaRoughAO;
uniform sampler2D gEntity;

struct PointLight {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
    float Radius;
};

struct SpotLight {
    vec3 Position;
    vec3 Color;
    vec3 Direction;

    float Cutoff;
    float OuterCutoff;

    float Linear;
    float Quadratic;
};

const float PI = 3.14159265359;
const int NR_LIGHTS = 32;
const int MAX_SPOT_LIGHT = 32;
uniform PointLight points[NR_LIGHTS];
uniform SpotLight spots[MAX_SPOT_LIGHT];
// uniform vec3 viewPos;


uniform int num_points; // point lights
uniform int num_spots;

uniform vec3 viewPos;
// uniform samplerCube skybox;

/*
vec3 getNormalFromMap(vec)
{
    vec3 tangentNormal = texture(gNormal, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(FragPos);
    vec3 Q2  = dFdy(FragPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}*/

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}


void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gDiffuseSpec, TexCoords).rgb;
    float Specular = texture(gDiffuseSpec, TexCoords).a;

    ////////////////////// PBR ///////////////////
    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;
    float Metallic = texture(gMetaRoughAO, TexCoords).r;
    float Roughness = texture(gMetaRoughAO, TexCoords).g;
    float AO = texture(gMetaRoughAO, TexCoords).b;
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);
    // vec3 viewDir  = normalize(the_fucking - FragPos);
    
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, Albedo, Metallic);
    // reflectance equation
    vec3 Lo = vec3(0.0);


    for(int i = 0; i < num_points; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(points[i].Position - FragPos);
        vec3 H = normalize(viewDir + L);
        float distance = length(points[i].Position - FragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = points[i].Color * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(Normal, H, Roughness);   
        float G   = GeometrySmith(Normal, viewDir, L, Roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);
           
        vec3 nominator    = NDF * G * F; 
        float denominator = 4 * max(dot(Normal, viewDir), 0.0) * max(dot(Normal, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 specular = nominator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - Metallic;	  
        
        // scale light by NdotL
        float NdotL = max(dot(Normal, L), 0.0);        
        
        // add to outgoing radiance Lo
        Lo += (kD * Albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }
    {
        vec3 ambient = vec3(0.03) * Albedo * AO;
        lighting += ambient + Lo;
    }


    /////////////////////// END PBR//////////////

    for(int i = 0; i < num_points; ++i)
    {
        // calculate distance between light source and current fragment
        float distance = length(points[i].Position - FragPos);
        //if(distance < lights[i].Radius)
       // {
            // diffuse
            vec3 lightDir = normalize(points[i].Position - FragPos);
            vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * points[i].Color;
            // specular
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
            vec3 specular = points[i].Color * spec * Specular;
            // attenuation
            float attenuation = 1.0 / (1.0 + points[i].Linear * distance + points[i].Quadratic * distance * distance);
            diffuse *= attenuation;
            specular *= attenuation;
            lighting += diffuse + specular;
       // }
    }    

    for(int i = 0; i < num_spots; ++i)
    {
        float distance = length(spots[i].Position - FragPos);
        

        vec3 lightDir = normalize(spots[i].Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * spots[i].Color;

        // vec3 halfwayDir = normalize(lightDir + viewDir); 
        // float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 reflectDir = reflect(-lightDir, Normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = spots[i].Color * spec * Specular;

        float theta = dot(lightDir, normalize(-spots[i].Direction));
        float epsilon   = spots[i].Cutoff - spots[i].OuterCutoff;
        float intensity = clamp((theta - spots[i].OuterCutoff) / epsilon, 0.0, 1.0);
        diffuse  *= intensity;
        specular *= intensity;

        //if(theta > spots[i].Cutoff) {
            //float attenuation = 1.0 / (1.0 + spots[i].Linear * distance + spots[i].Quadratic * distance * distance);
            //diffuse *= attenuation;
            //specular *= attenuation;
            //lighting += diffuse + specular;
       //}

        float attenuation = 1.0 / (1.0 + spots[i].Linear * distance + spots[i].Quadratic * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;
    }

    //vec3 sky_reflect = reflect(-viewDir, Normal);
    //vec4 sky_color = vec4(texture(skybox, sky_reflect).rgb, 1.0);
    FragColor = vec4(lighting, 1.0);
    //FragColor = vec4(lighting, 1.0) * 0.5 + sky_color * 0.5; // +  vec4(texture(gPicker, TexCoords).rgb, 1.0) * 0.8;
    // FragColor = vec4(lighting * 0.05, 0.01)  + vec4(e, 0.99);
    // FragColor = vec4(texture(gDiffuseSpec, TexCoords).rgb, 0);
}