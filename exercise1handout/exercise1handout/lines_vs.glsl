#version 410

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_color;

uniform mat4 model, view, proj;

out vec3 lerp_color;

void main() {
	gl_Position =  proj * view * model * vec4 (vertex_position, 1.0);
	lerp_color = vertex_color;
}
