#pragma once

#ifdef WIN32
#include <direct.h>
#endif
#include <assert.h>

#include "gl_utils.h"
#include "maths_funcs.h"
#include "mesh.h"
#include "node.h"
#include "camera.h"
#include "lineshapes.h"

struct Exercise2 {

	GLFWwindow *window = NULL;

	static bool isInputEnabled ;
	static double mousePosX, mousePosY;
	static double prevMousePosX, prevMousePosY;

	static void onWindowsFocus(GLFWwindow* window, int focused) {

		if (focused) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwPollEvents();
			glfwGetCursorPos(window, &prevMousePosX, &prevMousePosY);
			isInputEnabled = true;
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			isInputEnabled = false;
		}
	}

	vec3  ambientColor = vec3(0.7f, 0.7f, 0.75f);

	Meshgroup meshGroup;
	std::vector<Meshgroup::Mesh*> gulls;

	Node meshGroupNode;
	float meshYaw = 0;
	//float meshYaw2 = 0;

	float camYaw = 0.0f;
	float camPitch = 0.0f;

	Camera camera;
	Node camNode;
	vec3 cameraPosition;
	mat4 cameraRotation;
		
	Node sceneRoot;
	GLuint mesh_shader_index;
	GLuint lines_shader_index;

	Lines grid;
	Lines axis;

	void init(int width, int height) {
		isInputEnabled = true;

		// init
		//restart_gl_log();
		startGlContext(&window, width, height);

		mesh_shader_index = create_programme_from_files("test_vs.glsl", "test_fs.glsl");
		lines_shader_index = create_programme_from_files("lines_vs.glsl", "lines_fs.glsl");

		sceneRoot.init();

		meshGroupNode.init();
		sceneRoot.addChild(meshGroupNode);

		float scaleValue = 0.001f;
		meshGroupNode.scale = vec3(scaleValue, scaleValue, scaleValue);
		meshGroupNode.position = vec3(0, 20, 0);

		_chdir("../data/");
		Meshgroup::load_default_textures();
		
		_chdir("../data/tiki_treasure/");
		meshGroup.load_from_file("scene.gltf");

		meshGroup.load_to_gpu();
		meshGroup.get_shader_uniforms(mesh_shader_index);

		assert(meshGroup.nodes.size() > 0);
		assert(meshGroup.meshes.size() > 0);

		meshGroupNode.addChild(meshGroup.nodes[0]);

		sceneRoot.updateHierarchy();

		axis.clear();

		for (size_t i = 0; i < meshGroup.nodes.size(); ++i) {

			const Node& meshRoot = meshGroup.nodes[i];

			vec3 pos = meshRoot.worldMatrix.getColumn(3);
			vec3 side = meshRoot.worldMatrix.getColumn(0);
			vec3 up = meshRoot.worldMatrix.getColumn(1);
			vec3 forward = meshRoot.worldMatrix.getColumn(2);

			Shapes::addArrow(axis, pos, pos+normalise(side), vec3(1, 0, 0));
			Shapes::addArrow(axis, pos, pos+normalise(up), vec3(0, 1, 0));
			Shapes::addArrow(axis, pos, pos+normalise(forward), vec3(0, 0, 1));
		}

		axis.load_to_gpu();

	
		vec3 gridColor(fmodf(ambientColor.v[0] + 0.5f, 1.f), fmodf(ambientColor.v[1] + 0.5f, 1.f), fmodf(ambientColor.v[2] + 0.5f, 1.f));
		Shapes::addGrid(grid, vec3(-5, 0, -5), vec3(5, 0, 5), gridColor, 10);
		grid.load_to_gpu();
		grid.get_shader_uniforms(lines_shader_index);


		// camera
		cameraPosition = vec3(0, 3, 20);
		cameraRotation = identity_mat4();

		camNode.init();
		camNode.position = cameraPosition;
		sceneRoot.addChild(camNode);

		camera.near = 0.1f;
		camera.far = 1000.0f;
		camera.fov = 67.0f * ONE_DEG_IN_RAD;// convert degrees to radians
		camera.aspect = (float)g_gl_width / (float)g_gl_height; // aspect ratio

		camera.updateProjection();
		camera.speed = 5.0f;
		camera.yaw_speed = 120.f;

		// tell GL to only draw onto a pixel if the shape is closer to the viewer
		glEnable(GL_DEPTH_TEST); // enable depth-testing
		glDepthFunc(GL_LESS);		 // depth-testing interprets a smaller value as "closer"
		glEnable(GL_CULL_FACE); // cull face
		glCullFace(GL_BACK);		// cull back face
		glFrontFace(GL_CCW);		// GL_CCW for counter clock-wise

		glfwPollEvents();
		glfwGetCursorPos(window, &prevMousePosX, &prevMousePosY);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetWindowFocusCallback(window, onWindowsFocus);
	}

	void update() {
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, 1);
		}

		static float previous_seconds = static_cast<float>(glfwGetTime());
		float current_seconds = static_cast<float>(glfwGetTime());
		float elapsed_seconds = current_seconds - previous_seconds;
		previous_seconds = current_seconds;

		_update_fps_counter(window);
		// wipe the drawing surface clear
		glClearColor(ambientColor.v[0], ambientColor.v[1], ambientColor.v[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_gl_width, g_gl_height);


		glfwPollEvents();
		if (isInputEnabled) {
			glfwGetCursorPos(window, &mousePosX, &mousePosY);
			float mouseDeltaX = static_cast<float>(mousePosX - prevMousePosX);
			float mouseDeltaY = static_cast<float>(mousePosY - prevMousePosY);
			prevMousePosX = mousePosX;
			prevMousePosY = mousePosY;

			camYaw += -mouseDeltaX * camera.yaw_speed * elapsed_seconds;
			camPitch += -mouseDeltaY * camera.yaw_speed * elapsed_seconds;
		}
		


		if (glfwGetKey(window, GLFW_KEY_A)) {
			camYaw += camera.yaw_speed * elapsed_seconds;
		}
		if (glfwGetKey(window, GLFW_KEY_D)) {
			camYaw -= camera.yaw_speed * elapsed_seconds;
		}
		if (glfwGetKey(window, GLFW_KEY_W)) {
			camPitch += camera.yaw_speed * elapsed_seconds;
		}
		if (glfwGetKey(window, GLFW_KEY_S)) {
			camPitch -= camera.yaw_speed * elapsed_seconds;
		}
		const float PitchLimit = 80;
		camPitch = camPitch > PitchLimit ? PitchLimit : camPitch;
		camPitch = camPitch < -PitchLimit ? -PitchLimit : camPitch;
		camYaw = fmodf(camYaw, 360);

		// TODO: 
		// camNode.rotation = quat_from_axis_deg(...)* ...;
		
		// TODO: use keys to modify cameraPosition here

		camNode.position = cameraPosition;

		mat4 cameraMatrix = translate( identity_mat4(), cameraPosition*-1.f);
		mat4 gridMatrix = translate(identity_mat4(), vec3(0,0,0));

		meshGroupNode.rotation = quat_from_axis_deg(meshYaw += elapsed_seconds * 10, 0, 1, 0);

		// whole scene hierarchy is updated here from root downwards
		sceneRoot.updateHierarchy();

		glUseProgram(mesh_shader_index);

		camera.get_shader_uniforms(mesh_shader_index);
		camera.set_shader_uniforms(mesh_shader_index, cameraMatrix );
		// TODO: camera.set_shader_uniforms(lines_shader_index, ...);
		

		meshGroup.set_shader_uniforms(mesh_shader_index,  ambientColor);
		meshGroup.render(mesh_shader_index);
	
		glUseProgram(0);

		glUseProgram(lines_shader_index);

		camera.get_shader_uniforms(lines_shader_index);
		camera.set_shader_uniforms(mesh_shader_index, cameraMatrix );
		// TODO: camera.set_shader_uniforms(lines_shader_index, ...);

		grid.get_shader_uniforms(lines_shader_index);
		//TODO: grid.set_shader_uniforms(lines_shader_index, ...);

		grid.set_shader_uniforms(lines_shader_index, gridMatrix);
		grid.render(lines_shader_index);

		axis.get_shader_uniforms(lines_shader_index);
		axis.set_shader_uniforms(lines_shader_index, sceneRoot.worldMatrix);

		axis.render(lines_shader_index);


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
