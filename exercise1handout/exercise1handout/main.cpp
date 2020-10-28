#include <vector>
#include <GL/glew.h>	// include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assert.h>

#include "exercise1.h"

// keep track of window size for things like the viewport and the mouse cursor

int g_gl_width = 1024;
int g_gl_height = 786;

int main() {

	Exercise1 app;

	app.init(g_gl_width, g_gl_height);

	while (app.isLoopGo()) {
		app.update();
	}

	app.terminate();
	return 0;
}
