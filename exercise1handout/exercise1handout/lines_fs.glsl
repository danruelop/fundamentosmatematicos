#version 410

in vec3 lerp_color;

// output colour
out vec4 frag_colour;

void main() {
	frag_colour.rgb = lerp_color;
	frag_colour.a = 1.0;
}
