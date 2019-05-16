out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

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

const int NR_LIGHTS = 32;
const int MAX_SPOT_LIGHT = 32;
uniform PointLight points[NR_LIGHTS];
uniform SpotLight spots[MAX_SPOT_LIGHT];
uniform vec3 viewPos;
uniform int num_points; // point lights
uniform int num_spots;

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);
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

        //FragColor = vec4((diffuse + specular), 1.0);
    }

    FragColor = vec4(lighting, 1.0);
}