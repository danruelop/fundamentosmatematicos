#ifndef _GL_UTILS_H_
#define _GL_UTILS_H_

#include <GL/glew.h>		// include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <stdarg.h>			// used by log functions to have variable number of args
/*------------------------------GLOBAL VARIABLES------------------------------*/
extern int g_gl_width;
extern int g_gl_height;
//extern GLFWwindow *g_window;
/*--------------------------------LOG FUNCTIONS-------------------------------*/
bool restart_gl_log();
bool gl_log( const char *message, ... );
/* same as gl_log except also prints to stderr */
bool gl_log_err( const char *message, ... );
/*--------------------------------GLFW3 and GLEW------------------------------*/
bool startGlContext(GLFWwindow** window,int width, int height);
void glfw_error_callback( int error, const char *description );
void glfw_framebuffer_size_callback( GLFWwindow *window, int width, int height );
void _update_fps_counter( GLFWwindow *window );
/*-----------------------------------SHADERS----------------------------------*/
bool parse_file_into_str( const char *file_name, char *shader_str, int max_len );
void print_shader_info_log( GLuint shader_index );
bool create_shader( const char *file_name, GLuint *shader, GLenum type );
bool is_programme_valid( GLuint sp );
bool create_programme( GLuint vert, GLuint frag, GLuint *programme );
/* just use this func to create most shaders; give it vertex and frag files */
GLuint create_programme_from_files( const char *vert_file_name,
																		const char *frag_file_name );
/*----------------------------------TEXTURES----------------------------------*/
bool load_texture( const char *file_name, GLuint *tex );
bool load_image_data(const char *file_name, unsigned char** image_data, int& x, int& y, int& n);
void unload_image_data(unsigned char* image_data);
void load_texture_to_gpu(unsigned char* image_data, GLuint* tex, int x, int y, int n);
#endif
