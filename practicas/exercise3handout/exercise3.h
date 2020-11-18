#pragma once

#ifdef WIN32
#include <direct.h>
#endif
#include <assert.h>
#include <array>

#include "gl_utils.h"
#include "maths_funcs.h"
#include "mesh.h"
#include "node.h"
#include "camera.h"
#include "lineshapes.h"
	
constexpr int NumSpheres = 4;

struct Exercise3 {

	GLFWwindow *window = NULL;

	static bool isInputEnabled ;
	static double mousePosX, mousePosY;
	static double prevMousePosX, prevMousePosY;
	int windowsWidth, windowsHeight;

	static void onWindowsFocus(GLFWwindow* window, int focused) {

		if (focused) {
			//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwPollEvents();
			glfwGetCursorPos(window, &prevMousePosX, &prevMousePosY);
			isInputEnabled = true;
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			isInputEnabled = false;
		}
	}

	// exercise 3 spaces
		
	// getRay direction from mouse coords
	//  see shaders for hints
	// get world mouse position
	// ray sphere intersection for selection 

	
	static vec3 getWorldMousePosition(float mouse_x, float mouse_y, float windowsWidth, float windowsHeight, const mat4& projMat, const mat4& viewMat) {

		// Paso 1.1
		float coord_x_mouse = (mouse_x * 2.f) / windowsWidth; // pasamos las coordenada 'x' e 'y' del ratón a coordenadas cubo
		float coord_y_mouse = (windowsHeight - mouse_y) * 2.f / windowsHeight; // las coordenadas llegan como 0 y 1, necesitamos multiplicar x2
		

		coord_x_mouse -= 1.f; coord_y_mouse -= 1.f; //para luego restar ambas por -1 y que queden como los límites: -1 y 1 como se pide en el ejercicio.

		vec4 sol = vec4(coord_x_mouse, coord_y_mouse, -1, 1);

		// Paso 1.2
		sol = inverse(projMat) * sol; // se multiplica por la inversa y pasa a espacio de cámara
		sol = homogeneous(sol); // se homogeiniza el resultado

		// Paso 1.3
		sol = inverse(viewMat) * sol; // se multiplica por la inversa de la matriz View para pasar a espacio de mundo

		return sol;

	}

	/* check if a ray and a sphere intersect. if not hit, returns false. it rejects
	intersections behind the ray caster's origin, and sets intersection_distance to
	the closest intersection */

	struct Ray {
		vec3 origin;
		vec3 direction;
	};

	// ANALYTIC SOLUTION

	// as in http://viclw17.github.io/2018/07/16/raytracing-ray-sphere-intersection/
	//static bool raySphereIntersection(const Ray& ray, vec3 C, float r, float* intersection_distance) {

	//	const vec3& A = ray.origin;
	//	const vec3& B = ray.direction;
	//	
	//	assert(fabsf(length(B) - 1) < 1e-03);
	//	
	//	 work out components of quadratic
	//	vec3 A_C = A - C;
	//	float a = 1;
	//	float b = 2*dot(B, A_C);
	//	float c = dot(A_C, A_C) - r * r;
	//	float discriminant = b * b - 4*a*c;
	//	
	//	if (discriminant < 0.0f) { // ray misses
	//		return false; 
	//	}
	//	if (discriminant > 0.0f) { // ray hits at two points
	//		 get the 2 intersection distances along ray
	//		float t_a = -b + sqrtf(discriminant);
	//		float t_b = -b - sqrtf(discriminant);
	//		*intersection_distance = t_b;
	//		 if behind viewer, throw one or both away
	//		if (t_a < 0.0) {
	//			if (t_b < 0.0) { return false; }
	//		}
	//		else if (t_b < 0.0) {
	//			*intersection_distance = t_a;
	//		}

	//		return true;
	//	}
	//	 check for ray hitting once (skimming the surface)
	//	if (0.0f == discriminant) {
	//		 if behind viewer, throw away
	//		float t = -b + sqrtf(discriminant);
	//		if (t < 0.0f) { return false; }
	//		*intersection_distance = t;
	//		return true;
	//	}
	//	return false;
	//}

	// as in https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection

	static bool solveQuadratic(const float& a, const float& b, const float& c, float& x0, float& x1)
	{
		float discr = b * b - 4 * a * c;
		if (discr < 0) return false;
		else if (discr == 0) x0 = x1 = -0.5 * b / a;
		else {
			float q = (b > 0) ?
				-0.5 * (b + sqrt(discr)) :
				-0.5 * (b - sqrt(discr));
			x0 = q / a;
			x1 = c / q;
	}
		if (x0 > x1) std::swap(x0, x1);

		return true;
}

	// GEOMETRIC SOLUTION
	// Me parece más natural la solución geométrica porque soy capaz de verlo mejor espacialmente y comprenderlo.
	static bool raySphereIntersection(const Ray& ray, vec3 C, float r, float* intersection_distance) {

		const vec3& A = ray.origin;
		const vec3& B = ray.direction;
		
        float t0, t1, t; // solutions for t if the ray intersects 

        // geometric solution
        vec3 L = C - A; 
        float tca = dot(L,B); 
        // if (tca < 0) return false;
        float d2 = dot(L,L) - tca * tca; 
        if (d2 > r) return false; 
        float thc = sqrt(r - d2); 
        t0 = tca - thc; 
        t1 = tca + thc; 

        if (t0 > t1) std::swap(t0, t1); 
 
        if (t0 < 0) { 
            t0 = t1; // if t0 is negative, let's use t1 instead 
            if (t0 < 0) return false; // both t0 and t1 are negative 
        } 
 
        t = t0; 
 
        return true; 
} 


	static void onKeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods){

		Exercise3& exercise = *static_cast<Exercise3*>(glfwGetWindowUserPointer(window));
		if (key == GLFW_KEY_0) {
		
			vec3 mouseWorldPos = getWorldMousePosition(static_cast<float>(mousePosX), static_cast<float>(mousePosY), static_cast<float>(exercise.windowsWidth), static_cast<float>(exercise.windowsHeight), exercise.camera.proj_mat, exercise.camNode.worldInverseMatrix);
			const vec3 camPos = exercise.camNode.worldMatrix.getColumn(3);
			Ray ray;
			ray.origin = camPos;
			ray.direction = normalise(mouseWorldPos - camPos);

			exercise.axis.clear();

			Shapes::addArrow(exercise.axis, ray.origin, ray.origin + normalise(ray.direction), vec3(1, 0, 0));

			exercise.axis.load_to_gpu();
		}
	}

	/* this function is called when the mouse buttons are clicked or un-clicked */
	static void onMouseClicked(GLFWwindow* window, int button, int action, int mods) {
		Exercise3& exercise = *static_cast<Exercise3*>(glfwGetWindowUserPointer(window));

		if (GLFW_PRESS == action) {
			vec3 mouseWorldPos = getWorldMousePosition(static_cast<float>(exercise.mousePosX), static_cast<float>(exercise.mousePosY), static_cast<float>(exercise.windowsWidth), static_cast<float>(exercise.windowsHeight), exercise.camera.proj_mat, exercise.camNode.worldInverseMatrix);

			const vec3 camPos = exercise.camNode.worldMatrix.getColumn(3);
			Ray ray;
			ray.origin = camPos;
			ray.direction = normalise(mouseWorldPos - camPos);

			// check ray against all spheres in scene
			int closest_sphere_clicked = -1;
			float closest_intersection = 0.0f;
			for (int i = 0; i < NumSpheres; i++) {
				float t_dist = 0.0f;

				const vec3 spherePos = exercise.sphereNodes[i].worldMatrix.getColumn(3);
			
				if (raySphereIntersection(ray, spherePos, 1, &t_dist)) {
					if (-1 == closest_sphere_clicked || t_dist < closest_intersection) {
						closest_sphere_clicked = i;
						closest_intersection = t_dist;
					}
				}
			} // endfor
			exercise.selectedSphereIndex = closest_sphere_clicked;
			//printf("sphere %i was clicked\n", closest_sphere_clicked);
		}
	}


	vec3  ambientColor = vec3(0.7f, 0.7f, 0.75f);

	Meshgroup meshGroup;
	std::vector<Meshgroup::Mesh*> gulls;

	std::array<Node, NumSpheres> sphereNodes;
	std::array<vec3, NumSpheres> spherePositions = {vec3(0,0,1),vec3(2,0,0),vec3(-2,0,0), vec3(-2,0,-2)};
	std::array<vec3, NumSpheres> sphereScales = {vec3(1,1,1),vec3(1,1,1),vec3(1,1,1), vec3(1,1,1)};
	std::array<vec3, NumSpheres> sphereColor = {vec3(1,0,0),vec3(0,1,0),vec3(0,0,1), vec3(1,1,0)};
	int selectedSphereIndex = NumSpheres + 1;

	Node meshGroupNode;
	float meshYaw = 0;
	//float meshYaw2 = 0;

	float camYaw = 0.0f;
	float camPitch = 0.0f;

	Camera camera;
	Node camNode;
	vec3 cameraPosition;
		
	Node sceneRoot;
	GLuint mesh_shader_index;
	GLuint lines_shader_index;

	Lines grid;
	Lines axis;

	// exercise:
	// ray picking


	void init(int width, int height) {

		windowsWidth = width;
		windowsHeight = height;

		isInputEnabled = true;

		// init
		//restart_gl_log();
		startGlContext(&window, width, height);
		glfwSetWindowUserPointer(window, this);

		mesh_shader_index = create_programme_from_files("test_vs.glsl", "test_fs.glsl");
		lines_shader_index = create_programme_from_files("lines_vs.glsl", "lines_fs.glsl");

		sceneRoot.init();

		meshGroupNode.init();
		sceneRoot.addChild(meshGroupNode);
		
		for (int i = 0; i < NumSpheres; ++i) {
			sphereNodes[i].init();
			sphereNodes[i].position = spherePositions[i];
			meshGroupNode.addChild(sphereNodes[i]);
		}


		//float scaleValue = 0.001f;
		//meshGroupNode.scale = vec3(scaleValue, scaleValue, scaleValue);
		//meshGroupNode.position = vec3(0, 20, 0);

		_chdir("../data/");
		Meshgroup::load_default_textures();

		//_chdir("../data/DamagedHelmet/");
		//meshGroup.load_from_file("DamagedHelmet.gltf");
		
		//_chdir("../data/tiki_treasure/");
		//meshGroup.load_from_file("scene.gltf");
		
		_chdir("../data/sphere/");
		meshGroup.load_from_file("sphere.obj");
		meshGroup.load_to_gpu();
		meshGroup.get_shader_uniforms(mesh_shader_index);

		assert(meshGroup.nodes.size() > 0);
		assert(meshGroup.meshes.size() > 0);

		meshGroupNode.addChild(meshGroup.nodes[0]);


	
		vec3 gridColor(fmodf(ambientColor.v[0] + 0.5f, 1.f), fmodf(ambientColor.v[1] + 0.5f, 1.f), fmodf(ambientColor.v[2] + 0.5f, 1.f));
		Shapes::addGrid(grid, vec3(-5, 0, -5), vec3(5, 0, 5), gridColor, 10);
		grid.load_to_gpu();
		grid.get_shader_uniforms(lines_shader_index);


		// camera
		cameraPosition = vec3(0, 1, 6);

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

		sceneRoot.updateHierarchy();


		// tell GL to only draw onto a pixel if the shape is closer to the viewer
		glEnable(GL_DEPTH_TEST); // enable depth-testing
		glDepthFunc(GL_LESS);		 // depth-testing interprets a smaller value as "closer"
		glEnable(GL_CULL_FACE); // cull face
		glCullFace(GL_BACK);		// cull back face
		glFrontFace(GL_CCW);		// GL_CCW for counter clock-wise

		glfwPollEvents();
		glfwGetCursorPos(window, &prevMousePosX, &prevMousePosY);
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetWindowFocusCallback(window, onWindowsFocus);
		glfwSetMouseButtonCallback(window, onMouseClicked);
		glfwSetKeyCallback(window, onKeyPressed);

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

		camNode.rotation = quat_from_axis_deg(camYaw, 0, 1, 0)*quat_from_axis_deg(camPitch, 1, 0, 0);

		if (glfwGetKey(window, GLFW_KEY_PAGE_UP)) {
			camNode.position.v[1] += camera.speed * elapsed_seconds;
		}
		if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN)) {
			camNode.position.v[1] -= camera.speed * elapsed_seconds;
		}
		if (glfwGetKey(window, GLFW_KEY_UP)) {
			camNode.position -= camNode.localMatrix * vec4(0, 0, 1, 0) * camera.speed * elapsed_seconds;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN)) {
			camNode.position += camNode.localMatrix * vec4(0, 0, 1, 0) * camera.speed * elapsed_seconds;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT)) {
			camNode.position -= camNode.localMatrix * vec4(1, 0, 0, 0) * camera.speed * elapsed_seconds;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
			camNode.position += camNode.localMatrix * vec4(1, 0, 0, 0) * camera.speed * elapsed_seconds;
		}
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, 1);
		}
		mat4 cameraMatrix = translate(identity_mat4(), cameraPosition*-1.f);
		mat4 gridMatrix = translate(identity_mat4(), vec3(0,0,0));

		meshGroupNode.rotation = quat_from_axis_deg(meshYaw += elapsed_seconds * 10, 0, 1, 0);

		sceneRoot.updateHierarchy();

		glUseProgram(mesh_shader_index);

		camera.get_shader_uniforms(mesh_shader_index);
		camera.set_shader_uniforms(mesh_shader_index, camNode.worldInverseMatrix);
		//camera.set_shader_uniforms(mesh_shader_index, cameraMatrix );

		meshGroup.set_shader_uniforms(mesh_shader_index,  ambientColor);

		for (int i = 0; i < NumSpheres; ++i) {
			meshGroup.meshes[0].render(mesh_shader_index, sphereNodes[i].worldMatrix, i == selectedSphereIndex ? vec3(1, 1, 1): sphereColor[i]);
		}
	
		glUseProgram(0);

		glUseProgram(lines_shader_index);

		camera.get_shader_uniforms(lines_shader_index);
		camera.set_shader_uniforms(lines_shader_index, camNode.worldInverseMatrix);
		//camera.set_shader_uniforms(mesh_shader_index, cameraMatrix );

		grid.get_shader_uniforms(lines_shader_index);
		grid.set_shader_uniforms(lines_shader_index, sceneRoot.worldMatrix);
		//grid.set_shader_uniforms(lines_shader_index, gridMatrix);
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
