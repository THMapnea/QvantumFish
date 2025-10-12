#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<iostream>
#include<vector>
#include<complex>




/*

	Structures

*/
//structure that holds the size of the windows created if it's needed to pass the values
typedef struct {
	int width;
	int height;
}WindowSize;

/*

	Prototypes

*/
void error_callback(int error, const char* description);



/*

	MAIN

*/
int main() {
	//here we declare all the variables we might need
	WindowSize main_window;
	main_window.width = 640;
	main_window.height = 480;

	//setting the callback funtion for glfw
	glfwSetErrorCallback(error_callback);


	//check if glfw has been correctly initialized
	if (!glfwInit())
	{
		std::cout << "Error loading GLFW" << std::endl;
		return -1;
	}

	//create the window
	GLFWwindow* window = glfwCreateWindow(main_window.width, main_window.height, "Main", NULL, NULL);

	//handle possible errors
	if (!window)
	{
		std::cout << "Error creating the window" << std::endl;
		glfwTerminate();
		return -1;
	}

	//create the openGL current context
	glfwMakeContextCurrent(window);


	//check if glad has been correctly initializedonly after creating the context
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	while (!glfwWindowShouldClose(window))
	{
		// Clear the color buffer
		glClear(GL_COLOR_BUFFER_BIT);
		
		//handle the viewport creation to tell glfw the size of the rendering window
		glViewport(0, 0, main_window.width, main_window.width);

		//rendering must go here

		// Swap the front and back buffers
		glfwSwapBuffers(window);

		// Poll for events (keyboard, mouse, window close, etc.)
		glfwPollEvents();
	}

	//terminate the glfw library
	glfwTerminate();
	return 0;
}






/*

	Function that handles error callback

*/
void error_callback(int error, const char* description){
	fprintf(stderr, "Error: %s\n", description);
}