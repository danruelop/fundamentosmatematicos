#pragma once

#include "maths_funcs.h"
#include <GL/glew.h>

struct Camera {

	float speed;
	float yaw_speed;

	float near;
	float far;
	float fov;
	float aspect;

	mat4 proj_mat;
	
	int view_mat_location; 
	int proj_mat_location;

	void init();
	void updateProjection() ;

	void get_shader_locations(GLuint shader_programme) ;
	void set_shader_data(GLuint shader_programme, mat4 view) ;
};

