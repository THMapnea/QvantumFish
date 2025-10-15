#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>

/*

	Shader Compilation

*/

// vertex shader
const char* vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// fragment shader
const char* fragment_shader_source = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 color;

void main() {
	FragColor = vec4(color, 1.0);
}
)";

/*
	Utility function to compile shaders
*/
unsigned int compileShader(unsigned int type, const char* source) {
	unsigned int shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	int success;
	char log[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, log);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << log << std::endl;
	}
	return shader;
}

/*
	Create sphere vertices (wireframe)
*/
std::vector<float> generateSphere(float radius, int sectorCount, int stackCount) {
	std::vector<float> vertices;
	for (int i = 0; i <= stackCount; ++i) {
		float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / stackCount;
		float xy = radius * cosf(stackAngle);
		float z = radius * sinf(stackAngle);

		for (int j = 0; j <= sectorCount; ++j) {
			float sectorAngle = j * 2 * glm::pi<float>() / sectorCount;
			float x = xy * cosf(sectorAngle);
			float y = xy * sinf(sectorAngle);
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
		}
	}
	return vertices;
}

/*
	Create cone vertices for axis arrows
*/
std::vector<float> generateCone(float height, float radius, int segments) {
	std::vector<float> vertices;
	for (int i = 0; i <= segments; ++i) {
		float theta = i * 2 * glm::pi<float>() / segments;
		float x = radius * cos(theta);
		float y = radius * sin(theta);
		vertices.insert(vertices.end(), { x, y, 0.0f });       // base circle
		vertices.insert(vertices.end(), { 0.0f, 0.0f, height }); // tip
	}
	return vertices;
}

/*
	Key callback
*/
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

int main() {
	// Initialize GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Neon Sphere + Axes", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Load GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD\n";
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode

	// Compile shaders
	unsigned int vertex_shader = compileShader(GL_VERTEX_SHADER, vertex_shader_source);
	unsigned int fragment_shader = compileShader(GL_FRAGMENT_SHADER, fragment_shader_source);

	unsigned int shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	int success;
	char log[512];
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader_program, 512, NULL, log);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << log << std::endl;
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	/*
		Create the sphere VAO/VBO
	*/
	std::vector<float> sphereVerts = generateSphere(1.0f, 40, 40);
	unsigned int sphereVAO, sphereVBO;
	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);
	glBindVertexArray(sphereVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sphereVerts.size() * sizeof(float), sphereVerts.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	/*
		Create axis lines
	*/
	float axes[] = {
		0,0,0, 1.2,0,0,
		0,0,0, 0,1.2,0,
		0,0,0, 0,0,1.2
	};
	unsigned int axisVAO, axisVBO;
	glGenVertexArrays(1, &axisVAO);
	glGenBuffers(1, &axisVBO);
	glBindVertexArray(axisVAO);
	glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	/*
		Create cone tips for the axes
	*/
	std::vector<float> coneVerts = generateCone(0.1f, 0.03f, 20);
	unsigned int coneVAO, coneVBO;
	glGenVertexArrays(1, &coneVAO);
	glGenBuffers(1, &coneVBO);
	glBindVertexArray(coneVAO);
	glBindBuffer(GL_ARRAY_BUFFER, coneVBO);
	glBufferData(GL_ARRAY_BUFFER, coneVerts.size() * sizeof(float), coneVerts.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Set up transformations
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(2.5f, 2.5f, 2.5f), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));

	// Main render loop
	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader_program);

		// Uniforms
		glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE, glm::value_ptr(view));

		// --- Draw rotating sphere ---
		glm::mat4 model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime() * glm::radians(25.0f), glm::vec3(0.2f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform3f(glGetUniformLocation(shader_program, "color"), 0.0f, 1.0f, 0.8f); // Neon cyan-green
		glBindVertexArray(sphereVAO);
		glDrawArrays(GL_LINE_STRIP, 0, sphereVerts.size() / 3);

		// --- Draw static axes ---
		model = glm::mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform3f(glGetUniformLocation(shader_program, "color"), 0.3f, 1.0f, 0.3f);
		glBindVertexArray(axisVAO);
		glDrawArrays(GL_LINES, 0, 6);

		// --- Draw cone tips ---
		glBindVertexArray(coneVAO);

		glm::mat4 tipX = glm::translate(glm::mat4(1.0f), glm::vec3(1.2f, 0.0f, 0.0f));
		tipX = glm::rotate(tipX, glm::radians(90.0f), glm::vec3(0, 1, 0));
		glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, glm::value_ptr(tipX));
		glDrawArrays(GL_LINES, 0, coneVerts.size() / 3);

		glm::mat4 tipY = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.2f, 0.0f));
		tipY = glm::rotate(tipY, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, glm::value_ptr(tipY));
		glDrawArrays(GL_LINES, 0, coneVerts.size() / 3);

		glm::mat4 tipZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.2f));
		glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, glm::value_ptr(tipZ));
		glDrawArrays(GL_LINES, 0, coneVerts.size() / 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
