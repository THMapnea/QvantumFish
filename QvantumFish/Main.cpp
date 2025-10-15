#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>

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

//Vertex shader source: applies rotation and projection
const char* vertex_shader_source =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 transform;\n"
"void main()\n"
"{\n"
"   gl_Position = transform * vec4(aPos, 1.0);\n"
"}\0";

//Fragment shader source: outputs neon cyan-green color
const char* fragment_shader_source =
"#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(0.0, 1.0, 0.8, 1.0);\n"  // neon green-cyan glow
"}\0";



/*

    Function to generate sphere vertices in Cartesian coordinates

*/
void generateSphere(std::vector<float>& vertices, unsigned int sectors = 36, unsigned int stacks = 18) {

    float radius = 0.7f; //Sphere radius
    float sectorStep = 2 * M_PI / sectors;
    float stackStep = M_PI / stacks;

    for (unsigned int i = 0; i <= stacks; ++i) {
        float stackAngle = M_PI / 2 - i * stackStep; // from pi/2 to -pi/2
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (unsigned int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);
            vertices.insert(vertices.end(), { x, y, z });
        }
    }
}



/*

    MAIN

*/
int main() {

    //Here we declare all the variables we might need
    WindowSize main_window;
    main_window.width = 800;
    main_window.height = 800;

    //Set the callback function for GLFW errors
    glfwSetErrorCallback(error_callback);

    //Check if GLFW has been correctly initialized
    if (!glfwInit()) {
        std::cout << "Error loading GLFW" << std::endl;
        return -1;
    }

    //Create the window
    GLFWwindow* window = glfwCreateWindow(main_window.width, main_window.height, "Neon Sphere", NULL, NULL);

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



    /*

        Sphere Vertices Generation

    */
    std::vector<float> vertices;
    generateSphere(vertices, 72, 36); // more sectors for smooth sphere

    //Generate line indices for the wireframe (latitude and longitude lines)
    std::vector<unsigned int> indices;
    unsigned int sectors = 72, stacks = 36;
    for (unsigned int i = 0; i < stacks; ++i) {
        for (unsigned int j = 0; j < sectors; ++j) {
            unsigned int first = i * (sectors + 1) + j;
            unsigned int second = first + sectors + 1;
            // Each quad gives two lines (longitude and latitude)
            indices.push_back(first);
            indices.push_back(first + 1);
            indices.push_back(first);
            indices.push_back(second);
        }
    }



    /*

        Shader Compilation

    */

    int success;
    char log[512];

    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, NULL, log);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << log << std::endl;
    }

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, log);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << log << std::endl;
    }

    unsigned int shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, log);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << log << std::endl;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);



    /*

        Buffers and VAO Setup

    */
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);



    /*

        Main Rendering Loop

    */
    glUseProgram(shader_program);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe mode

    GLint transformLoc = glGetUniformLocation(shader_program, "transform");

    while (!glfwWindowShouldClose(window)) {

        //Calculate rotation over time
        float time = (float)glfwGetTime();
        float angleX = time * 0.5f;
        float angleY = time * 0.3f;

        //Build simple rotation matrices manually
        float cosX = cosf(angleX), sinX = sinf(angleX);
        float cosY = cosf(angleY), sinY = sinf(angleY);

        //Combined rotation matrix around X and Y
        float transform[16] = {
            cosY,  sinX * sinY,  cosX * sinY,  0.0f,
            0.0f,  cosX,        -sinX,         0.0f,
           -sinY,  sinX * cosY,  cosX * cosY,  0.0f,
            0.0f,  0.0f,         0.0f,         1.0f
        };

        //Clear the color buffer and set a black background
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //Use our shader program and send transformation
        glUseProgram(shader_program);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform);

        //Bind the VAO and draw the sphere lines
        glBindVertexArray(VAO);
        glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);

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
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shader_program);
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
