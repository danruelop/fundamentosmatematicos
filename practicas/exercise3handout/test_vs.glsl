#version 410

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 uvs0;
layout(location = 3) in vec2 uvs1;
layout(location = 4) in vec4 vtangent;

uniform mat4 model, view, proj;

out vec4 test_tan;

out vec2 st;
out vec3 view_dir_tan;
out vec3 light_dir_tan;
out float vertex_distance;

void main() {
	gl_Position =  proj * view * model * vec4 (vertex_position, 1.0);
	mat3 modelRot = mat3(model);
	vertex_distance = (modelRot*vertex_position).z;
	st = uvs0;
}
