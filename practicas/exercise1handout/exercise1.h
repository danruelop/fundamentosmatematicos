#pragma once

#ifdef WIN32
#include <direct.h>
#endif
#include "gl_utils.h"
#include "maths_funcs.h"
#include "camera.h"
#include "lineshapes.h"

struct Exercise1 {

	GLFWwindow *window = NULL;

	vec3  ambientColor = vec3(0.7f, 0.7f, 0.75f);
	GLuint lines_shader_index;

	Camera camera;
	mat4 cameraMatrix;
	vec3 cameraPosition;

	Lines cubeLines;
	mat4 cubeMatrix;
	mat4 cubeMatrixT;
	mat4 cubeMatrixR;
	vec3 cubePosition;

	Lines referenceFrameLines;
	mat4 referenceFrameMatrix;

	void init(int width, int height) {

		// init
		//restart_gl_log();
		startGlContext(&window, width, height);

		lines_shader_index = create_programme_from_files("lines_vs.glsl", "lines_fs.glsl");

		vec3 whiteColor = vec3(1.0f, 1.0f, 1.0f);

		vec3 cubeVertexPositions []= {
			{-1,-1,1},
			{1,-1,1},
			{1,1,1},
			{-1,1,1},
			{-1,-1,-1},
			{1,-1,-1},
			{1,1,-1},
			{-1,1,-1},
		};
		
		vec3 cubeVertexColors []= {
			whiteColor,
			whiteColor,
			whiteColor,
			whiteColor,
			whiteColor,
			whiteColor,
			whiteColor,
			whiteColor,
		};
		
		GLuint cubeLineIndices []= {
			0,1,
			1,2,
			2,3,
			3,0,
			4,5,
			5,6,
			6,7,
			7,4,

			0,4,
			1,5,
			2,6,
			3,7,

		};

		cubeLines.add(&cubeVertexPositions[0].v[0], &cubeVertexColors[0].v[0], 8, &cubeLineIndices[0], 12 * 2);
		cubeLines.load_to_gpu();
		cubeMatrix = identity_mat4();
		cubePosition = vec3(0, 0, 0);

		Shapes::addArrow(referenceFrameLines, vec3(0, 0, 0), vec3(1, 0, 0), vec3(1, 0, 0));
		Shapes::addArrow(referenceFrameLines, vec3(0, 0, 0), vec3(0, 1, 0), vec3(0, 1, 0));
		Shapes::addArrow(referenceFrameLines, vec3(0, 0, 0), vec3(0, 0, 1), vec3(0, 0, 1));

		referenceFrameLines.load_to_gpu();
		referenceFrameLines.get_shader_locations(lines_shader_index);

		referenceFrameMatrix = identity_mat4();

		// camera
		camera.near = 0.1f;
		camera.far = 1000.0f;
		camera.fov = 67.0f * ONE_DEG_IN_RAD;// convert degrees to radians
		camera.aspect = (float)g_gl_width / (float)g_gl_height; // aspect ratio

		camera.updateProjection();
		camera.speed = 5.0f;
		camera.yaw_speed = 120.f;

		cameraMatrix = identity_mat4();
		

		// 1.¿Sobre qué eje [x,y o z] tienes que actuar en cada caso?
		// la posición de la cámara depende de un vector de 3 dimensiones con 3 variables.
		// si queremos mover la cámara a la derecha aumentamos el valor de la variable x
		// si queremos mover la cámara a la izquierda disminuimos el valor de la variable x
		// si queremos mover la cámara arriba aumentamos el valor de la variable y
		// si queremos mover la cámara abajo disminuimos el valor de la variable y

		cameraPosition = vec3(1, 1, 7);

		// tell GL to only draw onto a pixel if the shape is closer to the viewer
		glEnable(GL_DEPTH_TEST); // enable depth-testing
		glDepthFunc(GL_LESS);		 // depth-testing interprets a smaller value as "closer"
		glEnable(GL_CULL_FACE); // cull face
		glCullFace(GL_BACK);		// cull back face
		glFrontFace(GL_CCW);		// GL_CCW for counter clock-wise

		// 3.¿Cómo utiliza la matriz de la cámara en el shader?

	}

	void update() {
		static float previous_seconds = static_cast<float>(glfwGetTime());
		float current_seconds = static_cast<float>(glfwGetTime());
		float elapsed_seconds = current_seconds - previous_seconds;
		previous_seconds = current_seconds;

		_update_fps_counter(window);
		// wipe the drawing surface clear
		glClearColor(ambientColor.v[0], ambientColor.v[1], ambientColor.v[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_gl_width, g_gl_height);

		// 4. utiliza el input de teclas en update() para mover la cámara de la siguiente forma:
		// 5. varía el código anterior para que el movimiento no dependa de la velocidad del ordenador, sino del tiempo que pasa (pista: usa el tiempo entre frames elapsed_seconds). 
		// Por ejemplo: mueve la cámara tres unidades por segundo.
		if (glfwGetKey(window, GLFW_KEY_DOWN)) {
			cameraPosition.y -= 1 *elapsed_seconds*3;
		}

		if (glfwGetKey(window, GLFW_KEY_D)) {

			cubePosition.x += 1 * elapsed_seconds *3;
			cubeMatrixT = translate(cubeMatrix, cubePosition);
			cubeMatrixR = rotate_z_deg(cubeMatrix, 0.01f);
			// esto no vale cubeMatrix = cubeMatrixT * cubeMatrixR;
		
			

		}
		

		glfwPollEvents();
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, 1);
		}

		// TODO: change following line to translate and rotate camera 
		// 2. ¿Por qué se multiplica cameraPosition por -1.f?
		// Se multiplica por -1 para poder ver el cubo de frente (?)
		// Cuánto más aumente el número más se alejará del cubo
		cameraMatrix = translate(identity_mat4(), cameraPosition*-1.f);

		// TODO: change following line to translate and rotate cube
		//cubeMatrix = translate(identity_mat4(), cubePosition);

		glUseProgram(lines_shader_index);

		camera.get_shader_locations(lines_shader_index);
		camera.set_shader_data(lines_shader_index, cameraMatrix);

		referenceFrameLines.get_shader_locations(lines_shader_index);
		referenceFrameLines.set_shader_data(lines_shader_index, referenceFrameMatrix);
		referenceFrameLines.render(lines_shader_index);
		
		cubeLines.get_shader_locations(lines_shader_index);
		cubeLines.set_shader_data(lines_shader_index, cubeMatrix);
		cubeLines.render(lines_shader_index);


		glUseProgram(0);

		// put the stuff we've been drawing onto the display
		glfwSwapBuffers(window);
	}

	bool isLoopGo() {
		return !glfwWindowShouldClose(window);
	}

	void terminate() {
		// close GL context and any other GLFW resources
		glfwTerminate();
	}
};

