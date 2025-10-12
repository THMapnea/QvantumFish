#include<GLFW/glfw3.h>
#include<iostream>
#include<vector>
#include<complex>



/*

	Prototypes

*/
void error_callback(int error, const char* description);




int main() {
	//setting the callback funtion for glfw
	glfwSetErrorCallback(error_callback);


	//check if glfw has been correctly initialized
	if (!glfwInit())
	{
		std::cout << "Error loading GLFW" << std::endl;
		return -1;
	}

	//create the window
	GLFWwindow* window = glfwCreateWindow(640, 480, "Main", NULL, NULL);

	//handle possible errors
	if (!window)
	{
		std::cout << "Error creating the window" << std::endl;
		glfwTerminate();
		return -1;
	}

	//create the openGL current context
	glfwMakeContextCurrent(window);

	while (!glfwWindowShouldClose(window))
	{
		// Clear the color buffer
		glClear(GL_COLOR_BUFFER_BIT);
		
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