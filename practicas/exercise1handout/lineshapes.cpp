#include "lineshapes.h"

void Lines::add(GLfloat* vertex_data,  GLfloat* color_data, size_t vertexCount, GLuint* indices_data, size_t indexCount)
{
	size_t point_size = points.size();
	size_t index_size = indices.size();

	points.resize(point_size + vertexCount);
	colors.resize(point_size + vertexCount);
	indices.resize(index_size + indexCount);

	for (size_t i = 0; i < vertexCount; ++i) {
		points[point_size + i].v[0] = vertex_data[i*3];
		points[point_size + i].v[1] = vertex_data[i*3 + 1];
		points[point_size + i].v[2] = vertex_data[i*3 + 2];
	}
	for (size_t i = 0; i < vertexCount; ++i) {
		colors[point_size + i].v[0] = color_data[i*3];
		colors[point_size + i].v[1] = color_data[i*3 + 1];
		colors[point_size + i].v[2] = color_data[i*3 + 2];
	}
	for (size_t i = 0; i < indexCount; ++i) {
		indices[index_size + i] = point_size + indices_data[i];
	}
}

void Lines::clear() {
	points.clear();
	colors.clear();
	indices.clear();
}

void Lines::load_to_gpu() {

	size_t vertex_count = points.size();
	size_t index_count = indices.size();

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	size_t attribIx = 0;
	

	if (vertex_count) {
		glGenBuffers(1, &points_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glBufferData(GL_ARRAY_BUFFER, 3 * vertex_count * sizeof(GLfloat), &points[0].v[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(attribIx);
		glVertexAttribPointer(attribIx, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	}
	++attribIx;
	
	if (vertex_count) {
		glGenBuffers(1, &colors_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
		glBufferData(GL_ARRAY_BUFFER, 3 * vertex_count * sizeof(GLfloat), &colors[0].v[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(attribIx);
		glVertexAttribPointer(attribIx, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	++attribIx;

	if (index_count) {
		glGenBuffers(1, &index_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*index_count, &indices[0], GL_STATIC_DRAW);
	}
	glBindVertexArray(0);
}


void Lines::get_shader_locations(GLuint shader_programme) {
	model_matrix_location = glGetUniformLocation( shader_programme, "model" );
}

void Lines::set_shader_data(GLuint shader_programme, const mat4& referenceFrameMatrix) 
{
	glUniformMatrix4fv( model_matrix_location, 1, GL_FALSE, referenceFrameMatrix.m );
}

void Lines::render(GLuint shader_programme)
{
	size_t index_count = indices.size();
	glBindVertexArray(vao);
	glDrawElements(GL_LINES, index_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Shapes::addArrow(Lines & lines, const vec3 & from, const vec3 & to, const vec3 & color)
{
	// TODO: change following code to draw arrow point
	// remember: to obtain a perpendicular to vector to-from, you can use cross(to-from, up)
	// if you normalize and multiply result by a number #, you will get a vector with number # length
	// add your new positions to arrow_vertices, then the corresponding arrow_colors
	// finally draw lines with indices arrow_indices

	vec3 arrow_vertices[] = {
		from,
		to,
	};
	
	vec3 arrow_colors[] = {
		color,
		color,
	};

	unsigned int arrow_indices[] = {
		0,1, // draw line from arrow_vertices[0] to arrow_vertices[1]
	};

	lines.add(&arrow_vertices[0].v[0], &arrow_colors[0].v[0], 2, &arrow_indices[0], 2);
}

void Shapes::addGrid(Lines& lines, const vec3& from, const vec3& to, const vec3& color, int divs) {
	int steps = divs + 1;

	vec3 size = to - from;
	float x = size.v[0]/(steps-1);
	float y = size.v[2]/(steps-1);

	std::vector<vec3> vertices;
	vertices.resize(steps * steps);
	std::vector<vec3> colors;
	colors.resize(steps * steps);

	for (int i = 0; i < steps; ++i) {
		for (int j = 0; j < steps; ++j) {
			vertices[j + i * steps] = vec3(from.v[0] + j * x, 0.f, from.v[2] + i*y);
			colors[j + i * steps] = color;
		}
	}

	std::vector<unsigned int> indices;
	indices.resize(4 * steps);
	for (int i = 0; i < steps; ++i) {
		indices[i*2] = i;
		indices[i*2+1] = i+steps*(steps-1);
	}
	for (int i = 0; i < steps; ++i) {
		indices[steps*2 + i*2] = i*steps;
		indices[steps*2 + i*2+1] =i*steps + (steps-1);
	}

	lines.add(&vertices[0].v[0], &colors[0].v[0], vertices.size(), &indices[0], indices.size());
}
