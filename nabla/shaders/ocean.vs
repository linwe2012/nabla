#version 330 core

in vec3 vertex;
in vec3 normal;
in vec2 texture;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 light_position;
uniform float fog_decay;

out vec3 light_vector;
out vec3 normal_vector;
out vec3 halfway_vector;
out float fog_factor;
out vec2 tex_coord;
out vec3 position;


void main() {
	gl_Position = view * model * vec4(vertex, 1.0);
	fog_factor = min(-gl_Position.z/fog_decay, 1.0);
	gl_Position = projection * gl_Position;
	position = gl_Position.xyz;

	vec4 v = view * model * vec4(vertex, 1.0);
	vec3 normal1 = normalize(normal);

	light_vector = normalize((view * vec4(light_position, 1.0)).xyz - v.xyz);
	normal_vector = (inverse(transpose(view * model)) * vec4(normal1, 0.0)).xyz;
    halfway_vector = light_vector + normalize(-v.xyz);

	tex_coord = texture.xy;

}