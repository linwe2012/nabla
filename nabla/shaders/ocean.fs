#version 330 core

in vec3 normal_vector;
in vec3 light_vector;
in vec3 halfway_vector;
in vec2 tex_coord;
in float fog_factor;
in vec3 position;
uniform vec3 view_pos;
// uniform sampler2D water;
out vec4 fragColor;
uniform samplerCube skybox;

void main (void) {
	//fragColor = vec4(1.0, 1.0, 1.0, 1.0);

	vec3 normal1         = normalize(normal_vector);
	vec3 light_vector1   = normalize(light_vector);
	vec3 halfway_vector1 = normalize(halfway_vector);

	vec4 c = vec4(1,1,1,1); //texture(water, tex_coord);

	vec4 emissive_color = vec4(1.0, 1.0, 1.0,  1.0);
	vec4 ambient_color  = vec4(0.0, 0.65, 0.75, 1.0);
	vec4 diffuse_color  = vec4(0.5, 0.65, 0.75, 1.0);
	vec4 specular_color = vec4(1.0, 0.25, 0.0,  1.0);

	float emissive_contribution = 0.00;
	float ambient_contribution  = 0.30;
	float diffuse_contribution  = 0.30;
	float specular_contribution = 1.80;

	float d = dot(normal1, light_vector1);
	bool facing = d > 0.0;

	fragColor = emissive_color * emissive_contribution +
		    ambient_color  * ambient_contribution  * c +
		    diffuse_color  * diffuse_contribution  * c * max(d, 0) +
                    (facing ?
			specular_color * specular_contribution * c * max(pow(dot(normal1, halfway_vector1), 120.0), 0.0) :
			vec4(0.0, 0.0, 0.0, 0.0));

	fragColor = fragColor * (1.0-fog_factor) + vec4(0.25, 0.75, 0.65, 1.0) * (fog_factor);
	vec3 view_dir = normalize(position - view_pos);
	float distance_ratio = 	min( 1.0, log( 1.0 / length( view_dir ) * 3000.0 + 1.0 ) );
	distance_ratio *= distance_ratio;
	distance_ratio = distance_ratio * 0.7 + 0.3;

	vec3 fresenl_normal = ( distance_ratio * normal_vector + vec3( 0.0, 1.0 - distance_ratio, 0.0 ) ) * 0.5;
	fresenl_normal = normalize(fresenl_normal);

	float fresenl = max(dot(-view_dir, fresenl_normal), 0.0);
	fresenl = pow(1.0-fresenl, 2.0);

	fragColor.rgb = texture(skybox, normal_vector).rgb * (fresenl) + fragColor.rgb * (1.0-fresenl);
	fragColor.a = 1.0;

}