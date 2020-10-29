#version 410

// inputs: texture coordinates, and view and light directions in tangent space
in vec2 st;
in vec3 view_dir_tan;
in vec3 light_dir_tan;

// the normal map texture
uniform sampler2D normal_map;
uniform sampler2D diffuse_map;
uniform vec3 diffuse_base_color;
uniform vec3 ambient_color;

// output colour
out vec4 frag_colour;

in vec4 test_tan;

void main() {
	vec3 diffuse_texture_color = texture (diffuse_map, st).rgb;
	vec3 diffuse_color = diffuse_base_color * diffuse_texture_color;

	frag_colour.rgb = diffuse_color;
	frag_colour.a = 1.0;}
