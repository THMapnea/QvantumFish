#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>

/*


TODO: add label to north and south and eventualli est and ovest

*/


const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

// Vertex shader
const char* vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";



// Fragment shader with glow effect and opacity
const char* fragment_shader_source = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 color;
uniform float time;
uniform float opacity;

void main(){
    // Cyan color with subtle pulsing effect
    vec3 glowColor = color;
    
    // Add a very subtle pulse to simulate CRT phosphor
    float pulse = sin(time * 2.0) * 0.05 + 0.95;
    glowColor *= pulse;
    
    // Apply opacity for depth effect
    FragColor = vec4(glowColor, opacity);
}
)";



// Compile shader
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



// Generate sphere with only 4 longitudinal lines
std::vector<float> generateMinimalSphere(float radius, int slices, int stacks) {
    std::vector<float> vertices;

    // Generate only 4 longitudinal lines (vertical)
    int selectedLongitudes[] = { 0, 8, 16, 24 }; // Evenly spaced around the sphere
    for (int k = 0; k < 4; ++k) {
        int i = selectedLongitudes[k];
        float lng = 2 * M_PI * i / slices;
        for (int j = 0; j <= stacks; ++j) {
            float lat = M_PI * (-0.5 + (float)j / stacks);
            float x = radius * cos(lat) * cos(lng);
            float y = radius * cos(lat) * sin(lng);
            float z = radius * sin(lat);
            vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
        }
    }

    // Generate latitude lines (horizontal) for sphere definition
    for (int j = 0; j <= stacks; ++j) {
        float lat = M_PI * (-0.5 + (float)j / stacks);
        float z = radius * sin(lat);
        float r = radius * cos(lat);
        for (int i = 0; i <= slices; ++i) {
            float lng = 2 * M_PI * i / slices;
            float x = r * cos(lng);
            float y = r * sin(lng);
            vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
        }
    }

    return vertices;
}

// Generate filled discs that slice through the sphere
std::vector<float> generateSliceDiscs(float radius, int segments) {
    std::vector<float> vertices;

    // Generate 3 discs at 1/4, 2/4, and 3/4 of sphere height
    float sliceHeights[] = { -radius * 0.5f, 0.0f, radius * 0.5f };

    for (int plane = 0; plane < 3; plane++) {
        float z = sliceHeights[plane];
        float sliceRadius = sqrt(radius * radius - z * z); // Radius of the slice circle

        // Create triangle fan for filled disc
        // Center point
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(z);

        // Outer points
        for (int i = 0; i <= segments; i++) {
            float angle = 2.0f * M_PI * i / segments;
            float x = sliceRadius * cos(angle);
            float y = sliceRadius * sin(angle);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    return vertices;
}



// Key input
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}



// Mouse input variables
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
float yaw = 0.0f;
float pitch = 0.0f;
bool firstMouse = true;
bool mousePressed = false;



// Mouse callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    if (mousePressed) {
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // Constrain pitch to avoid flipping
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    lastX = xpos;
    lastY = ypos;
}



// Mouse button callback
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mousePressed = true;
            firstMouse = true; // Reset first mouse to get smooth movement
        }
        else if (action == GLFW_RELEASE) {
            mousePressed = false;
        }
    }
}



int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "QvantumFish", NULL, NULL);
    if (!window) { std::cerr << "Failed to create window\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    // Set mouse callbacks
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n"; return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Enable blending for opacity effect
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
    if (!success) { glGetProgramInfoLog(shader_program, 512, NULL, log); std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << log << std::endl; }
    glDeleteShader(vertex_shader); glDeleteShader(fragment_shader);

    // Sphere VAO/VBO with only 4 longitudinal lines
    std::vector<float> sphereVerts = generateMinimalSphere(1.0f, 32, 32);
    unsigned int sphereVAO, sphereVBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVerts.size() * sizeof(float), sphereVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Slice discs VAO/VBO
    std::vector<float> discVerts = generateSliceDiscs(1.0f, 64);
    unsigned int discVAO, discVBO;
    glGenVertexArrays(1, &discVAO);
    glGenBuffers(1, &discVBO);
    glBindVertexArray(discVAO);
    glBindBuffer(GL_ARRAY_BUFFER, discVBO);
    glBufferData(GL_ARRAY_BUFFER, discVerts.size() * sizeof(float), discVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Calculate vertex counts for different parts
    int slices = 32;
    int stacks = 32;
    int verticesPerLongitude = stacks + 1;
    int verticesPerLatitude = slices + 1;
    int totalLongitudeVertices = 4 * verticesPerLongitude;
    int totalLatitudeVertices = (stacks + 1) * verticesPerLatitude;
    int verticesPerDisc = 66; // 64 segments + center + duplicate first vertex to close

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(2.5f, 2.5f, 2.5f), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));

    // Set line width for better visibility
    glLineWidth(2.0f);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        // Pure black background like oscilloscope
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader_program);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Pass time for pulsing effect
        float time = glfwGetTime();
        glUniform1f(glGetUniformLocation(shader_program, "time"), time);

        // Sphere with mouse rotation
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model));

        // Cyan color like oscilloscope glow
        glUniform3f(glGetUniformLocation(shader_program, "color"), 0.2f, 0.8f, 1.0f);
        glBindVertexArray(sphereVAO);

        // Draw only 4 longitudinal lines (vertical) with full opacity
        glUniform1f(glGetUniformLocation(shader_program, "opacity"), 1.0f);
        for (int i = 0; i < 4; ++i) {
            glDrawArrays(GL_LINE_STRIP, i * verticesPerLongitude, verticesPerLongitude);
        }

        // Draw latitude lines (horizontal) with reduced opacity for depth
        glUniform1f(glGetUniformLocation(shader_program, "opacity"), 0.3f); // Semi-transparent
        for (int j = 0; j <= stacks; ++j) {
            glDrawArrays(GL_LINE_STRIP, totalLongitudeVertices + j * verticesPerLatitude, verticesPerLatitude);
        }

        // Draw slice discs as very transparent filled triangles
        glUniform3f(glGetUniformLocation(shader_program, "color"), 0.15f, 0.6f, 0.8f); // Slightly different cyan
        glUniform1f(glGetUniformLocation(shader_program, "opacity"), 0.20f); // Very transparent (20% opacity)

        // Temporarily disable wireframe mode for discs
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindVertexArray(discVAO);
        for (int i = 0; i < 3; ++i) {
            glDrawArrays(GL_TRIANGLE_FAN, i * verticesPerDisc, verticesPerDisc);
        }
        // Re-enable wireframe mode for sphere
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}