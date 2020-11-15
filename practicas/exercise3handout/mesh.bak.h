#pragma once

#include <vector>
#include <GL/Glew.h>
#include "node.h"

struct Meshgroup {

	// geometry

	// shader 

	struct Mesh{

		GLfloat *vp; // array of vertex points
		GLfloat *vn; // array of vertex normals
		GLfloat *vc; // array of vertex colors 
		GLfloat *vtans;
		std::vector<GLfloat *> uvs;
		GLuint *faces_indices;

		GLuint vao;
		GLuint points_vbo;
		GLuint normals_vbo;
		std::vector<GLuint> uvs_vbos;
		GLuint tangents_vbo;
		GLuint faces_vbo;

		int vertex_count;
		int face_count;
		int index_count;
		unsigned int MaterialIndex;

		GLuint nmap_tex;
		GLuint dmap_tex;

		vec3 diffuse_base_color;
		mat4 transform;
	};

	std::vector<Mesh> meshes;
	mat4 transform;

	int model_matrix_location;
	int normal_map_location;
	int diffuse_map_location;
	int diffuse_base_color_location;
	int ambient_color_location;

	bool load_from_file( const char* file_name, int index = 0) ;
	void load_to_gpu() ;

	void load_textures_to_gpu();
	void get_shader_locations(GLuint shader_programme);
	void set_shader_data(GLuint shader_programme, const Node& node, const vec3& ambient_color);
	
	void render(GLuint shader_programme);
};

