#include "DivisionLines.h"

DivisionLines::DivisionLines()
    : m_vao(0), m_vbo(0), m_shader(0)
    , m_color(0.0f, 0.7f, 0.9f)
    , m_lineWidth(2.0f)
    , m_pulseSpeed(2.0f) {
}

DivisionLines::~DivisionLines() {
    cleanup();
}

bool DivisionLines::initialize() {
    if (!compileShaders()) {
        std::cerr << "Failed to compile division lines shaders" << std::endl;
        return false;
    }

    setupBuffers();
    return true;
}

bool DivisionLines::compileShaders() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 color;
        uniform float time;
        uniform float pulseSpeed;
        void main() {
            vec3 glowColor = color;
            float pulse = sin(time * pulseSpeed) * 0.1 + 0.9;
            glowColor *= pulse;
            FragColor = vec4(glowColor, 0.8);
        }
    )";

    // Compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check vertex shader compilation
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
        return false;
    }

    // Compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(vertexShader);
        return false;
    }

    // Create shader program
    m_shader = glCreateProgram();
    glAttachShader(m_shader, vertexShader);
    glAttachShader(m_shader, fragmentShader);
    glLinkProgram(m_shader);

    // Check shader program linking
    glGetProgramiv(m_shader, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_shader, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}

void DivisionLines::setupBuffers() {
    // Division lines vertices in normalized device coordinates
    float vertices[] = {
        // Vertical line (center X)
        0.0f,  1.0f,  // top
        0.0f, -1.0f,  // bottom
        // Horizontal line (center Y)
        -1.0f, 0.0f,  // left
         1.0f, 0.0f   // right
    };

    // Create VAO and VBO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void DivisionLines::render(float time, int windowWidth, int windowHeight) {
    if (m_shader == 0 || windowWidth == 0 || windowHeight == 0) return;

    // Save current state
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    GLboolean prevDepthTest = glIsEnabled(GL_DEPTH_TEST);
    GLint prevProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);
    GLfloat prevLineWidth;
    glGetFloatv(GL_LINE_WIDTH, &prevLineWidth);

    // Set up for 2D rendering (full screen)
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, windowWidth, windowHeight);

    // Use division lines shader
    glUseProgram(m_shader);
    glUniform3f(glGetUniformLocation(m_shader, "color"), m_color.r, m_color.g, m_color.b);
    glUniform1f(glGetUniformLocation(m_shader, "time"), time);
    glUniform1f(glGetUniformLocation(m_shader, "pulseSpeed"), m_pulseSpeed);

    // Render lines
    glLineWidth(m_lineWidth);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINES, 0, 4);

    // Restore state
    if (prevDepthTest) glEnable(GL_DEPTH_TEST);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
    glUseProgram(prevProgram);
    glLineWidth(prevLineWidth);
}

void DivisionLines::cleanup() {
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_shader) {
        glDeleteProgram(m_shader);
        m_shader = 0;
    }
}