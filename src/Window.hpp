#pragma once
#pragma once

#include <iostream>

#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "InputState.hpp"

class Window
{
public:
	Window();
	~Window();

	void updateGLFW();
	void updateMouse();
	bool getShouldClose();
	void terminate();
	GLFWwindow* getWindowPtr();

	const InputState& getInputState();
	static void mouseCallbackGLFW(GLFWwindow* window, double posX, double posY);
	static void framebufferResizeCallbackGLFW(GLFWwindow* window, int width, int height);

	const uint32_t width = 1600;
	const uint32_t height = 900;
private:
	GLFWwindow* window = nullptr;
	InputState inputState{};

	void recordKeypresses();

	void mouseCallback(double posX, double posY);
	void framebufferResizeCallback();
	bool updatedMouse = false;
};