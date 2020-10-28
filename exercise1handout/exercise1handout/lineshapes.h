#pragma once

#include <vector>
#include <GL/Glew.h>
#include "maths_funcs.h"

struct Lines {

	std::vector<vec3> points;
	std::vector<vec3> colors;
	std::vector<unsigned int> indices;

	GLuint vao;
	GLuint points_vbo;
	GLuint colors_vbo;
	GLuint index_vbo;

	int model_matrix_location;
	
	void add(GLfloat* vertex_data, GLfloat* color_data, size_t vertexCount, GLuint* indices_data, size_t indexCount);
	void clear();

	void load_to_gpu();
	void get_shader_locations(GLuint shader_programme);
	void set_shader_data(GLuint shader_programme, const mat4& referenceFrameMatrix);
	void render(GLuint shader_programme);
};


struct Shapes {
	static void addArrow(Lines& lines, const vec3& from, const vec3& to, const vec3& color);
	static void addGrid(Lines& lines, const vec3& from, const vec3& to, const vec3& color, int divs);
};
