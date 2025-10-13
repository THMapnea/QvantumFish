#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <complex>

/*

	Structures

*/

//Structure that holds the size of the window created if it's needed to pass the values
typedef struct {
	int width;
	int height;
} WindowSize;



/*

	Prototypes

*/
void error_callback(int error, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);



/*

	Shaders Source Code

*/

//Vertex shader source: transforms vertex positions into clip space
const char* vertex_shader_source =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos, 1.0);\n"
"}\0";

//Fragment shader source: outputs a fixed orange color
const char* fragment_shader_source =
"#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";



/*

	MAIN

*/
int main() {

	//Here we declare all the variables we might need
	WindowSize main_window;
	main_window.width = 640;
	main_window.height = 480;

	//Set the callback function for GLFW errors
	glfwSetErrorCallback(error_callback);

	//Check if GLFW has been correctly initialized
	if (!glfwInit()) {
		std::cout << "Error loading GLFW" << std::endl;
		return -1;
	}

	//Create the window
	GLFWwindow* window = glfwCreateWindow(main_window.width, main_window.height, "Main", NULL, NULL);

	//Handle possible errors during window creation
	if (!window) {
		std::cout << "Error creating the window" << std::endl;
		glfwTerminate();
		return -1;
	}

	//Create the OpenGL current context
	glfwMakeContextCurrent(window);

	//Initialize GLAD only after creating the OpenGL context
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return -1;
	}

	//Set the callback function for dynamically resizing the viewport
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Define vertices for a simple triangle
	float vertices[] = {
		-0.5f, -0.5f, 0.0f,	//Left vertex
		 0.5f, -0.5f, 0.0f,	//Right vertex
		 0.0f,  0.5f, 0.0f	//Top vertex
	};

	/*

		Shader Compilation

	*/

	//error checking variables
	int success;
	char log[512];

	//Create and compile the vertex shader
	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);

	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex_shader, 512, NULL, log);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << log << std::endl;
	}

	//Create and compile the fragment shader
	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment_shader, 512, NULL, log);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << log << std::endl;
	}

	//Create the shader program and link both shaders
	unsigned int shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader_program, 512, NULL, log);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << log << std::endl;
	}

	//Once linked, shaders can be deleted
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);



	/*

		Buffers and VAO Setup

	*/

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	//Bind the Vertex Array Object first
	glBindVertexArray(VAO);

	//Copy vertex data into a buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Configure the vertex attribute pointer
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Unbind buffers for safety
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);



	/*

		Main Rendering Loop

	*/
	while (!glfwWindowShouldClose(window)) {

		//Clear the color buffer and set a background color
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//Use our shader program
		glUseProgram(shader_program);

		//Bind the VAO and draw the triangle
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		//Swap the front and back buffers
		glfwSwapBuffers(window);

		//Poll for events (keyboard, mouse, window close, etc.)
		glfwPollEvents();
	}

	/*

		Cleanup

	*/
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shader_program);

	//Terminate the GLFW library
	glfwTerminate();

	return 0;
}



/*

	Function that handles error callback

*/
void error_callback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
}



/*

	Callback function that handles resizing

*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	//When the function gets called the viewport is adjusted to the new window size
	glViewport(0, 0, width, height);
}
