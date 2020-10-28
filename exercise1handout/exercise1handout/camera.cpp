#include "camera.h"

#include <GL/glew.h>

#define _USE_MATH_DEFINES
#include <math.h>

void Camera::init() {
	speed = 0;
	yaw_speed = 0;
}

void Camera::updateProjection() {
	float range = tanf( fov * 0.5f ) * near;
	float Sx = ( 2.0f * near ) / ( range * aspect + range * aspect );
	float Sy = near / range;
	float Sz = -( far + near ) / ( far - near );
	float Pz = -( 2.0f * far * near ) / ( far - near );
	proj_mat = { Sx,	 0.0f, 0.0f, 0.0f,	0.0f, Sy,		0.0f, 0.0f, 0.0f, 0.0f, Sz,	 -1.0f, 0.0f, 0.0f, Pz,		0.0f };
}

void Camera::get_shader_locations(GLuint shader_programme) {

	view_mat_location = glGetUniformLocation( shader_programme, "view" );
	proj_mat_location = glGetUniformLocation( shader_programme, "proj" );
}

void Camera::set_shader_data(GLuint shader_programme, mat4 view) {
	// set uniforms
	glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view.m );
	glUniformMatrix4fv( proj_mat_location, 1, GL_FALSE, proj_mat.m );
}
