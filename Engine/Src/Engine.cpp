#include <glad/glad.h>// must be included first
#include <GLFW/glfw3.h>// https://www.glfw.org/docs/3.3/
#define STB_IMAGE_IMPLEMENTATION// not sure what this is for
#include <stb_image.h>
#include "Lib.h"
int main(int argc, char** argv) {
	if(!glfwInit()) {
		return NULL;
	}
	Vector2 size(1920, 1080);
	GLFWwindow* window=glfwCreateWindow((int)size.x, (int)size.y, "Window", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwDestroyWindow(window);
		glfwTerminate();
		return NULL;
	}
	glViewport(0, 0, (int)size.x, (int)size.y);
	while(!glfwWindowShouldClose(window)) {
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return 1;
}