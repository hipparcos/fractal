#include "renderer_hardware.h"

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "panic.h"

/** vertices cover the entire screen. */
static const struct {
    float x, y;
    /* float r, g, b; */
} vertices[] = {
    { -1.0f, -1.0f, }, // 1.0f, 0.0f, 0.0f },
    { -1.0f,  1.0f, }, // 0.0f, 0.0f, 1.0f },
    {  1.0f, -1.0f, }, // 0.0f, 0.0f, 1.0f },
    {  1.0f, -1.0f, }, // 0.0f, 1.0f, 0.0f },
    {  1.0f,  1.0f, }, // 1.0f, 1.0f, 1.0f },
    { -1.0f,  1.0f, }, // 1.0f, 1.0f, 1.0f },
};

static const char* vertex_shader =
    "#version 330 core\n"
    "in vec2 in_position;\n"
    /* "in vec3 in_color;\n" */
    /* "out vec3 color;\n" */
    "void main() {\n"
    "    gl_Position = vec4(in_position, 0.0f, 1.0f);\n"
    /* "    color = in_color;\n" */
    "}\n";

static SDL_Window* lwindow;
static SDL_GLContext lcontext;
static GLuint vao, vbuffer, vs, fs, program;

/** read_file reads all content of filename at once.
 ** Caller is responsible for calling free on returned string. */
static char* read_file(const char* filename) {
    if (!filename) {
        return NULL;
    }
    FILE* file = fopen(filename, "r");
    if (!file) {
        panicf("Can't open `%s`.", filename);
    }
    fseek(file, 0L, SEEK_END);
    long length = ftell(file);
    if (length < 0) {
        fclose(file);
        panicf("Can't get size of `%s`.", filename);
    }
    fseek(file, 0L, SEEK_SET);
    char* buffer = malloc(length+1);
    if (!buffer) {
        fclose(file);
        panicf("Can't allocate buffer for `%s`.", filename);
    }
    if (fread(buffer, 1, length, file) < 0) {
        free(buffer);
        fclose(file);
        panicf("Can't read `%s`.", filename);
    }
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

GLuint LoadShader(GLenum type, const char* name, const char *src) {
    GLuint shader;
    GLint compiled;

    shader = glCreateShader(type);
    if (shader == 0)
        return 0;

    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char* infoLog = malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            panicf("Error: can't compile shader: %s\n%s", name, infoLog);
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void rdr_hw_init(SDL_Window* window) {
    lwindow = window;
    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    /* OpenGL init. */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    lcontext = SDL_GL_CreateContext(window);
    /* GLEW init. */
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        panicf("GLEW error: %s\n", glewGetErrorString(err));
    }

    /* VAO. */
    glGenVertexArrays(1, &vao);

    /* Shaders. */
    vs = LoadShader(GL_VERTEX_SHADER, "vertex", vertex_shader);
    char* noise_fragment = read_file("./generator/fractal.frag");
    fs = LoadShader(GL_FRAGMENT_SHADER, "fragment", noise_fragment);
    free(noise_fragment);
    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glUseProgram(program);

    /* Vertices. */
    glGenBuffers(1, &vbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);
}

void rdr_hw_free(void) {
    SDL_GL_DeleteContext(lcontext);
}

void rdr_hw_resize(int width, int height) {
    glViewport(0, 0, width, height);
}

void rdr_hw_render(struct fractal_info fi, double t, double dt) {
    fi.cy *= -1; // invert y coord to act like software renderer.

    int width, height;
    SDL_GetWindowSize(lwindow, &width, &height);
    glViewport(0, 0, width, height);
    glUseProgram(program);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbuffer);

    /* Shader variables. */
    GLint attrib_position = glGetAttribLocation(program, "in_position");
    glEnableVertexAttribArray(attrib_position);
    glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);

    /* Update uniform variables values. */
    /* u_window_size */
    GLint uniform_window_size = glGetUniformLocation(program, "u_window_size");
    float size[2] = {(float)width, (float)height};
    glUniform2fv(uniform_window_size, 1, size);
    /* u_center */
    GLint uniform_center = glGetUniformLocation(program, "u_center");
    float center[2] = {(float)fi.cx, (float)fi.cy};
    glUniform2fv(uniform_center, 1, center);
    /* u_dpp */
    GLint uniform_dpp = glGetUniformLocation(program, "u_dpp");
    float dpp[1] = { (float)fi.dpp };
    glUniform1fv(uniform_dpp, 1, dpp);
    /* u_max_iter */
    GLint uniform_max_iter = glGetUniformLocation(program,"u_max_iter");
    glUniform1i(uniform_max_iter, fi.max_iter);
    /* u_generator */
    GLint uniform_generator = glGetUniformLocation(program, "u_generator");
    glUniform1i(uniform_generator, fi.generator);
    /* u_j */
    GLint uniform_julia = glGetUniformLocation(program, "u_j");
    float julia[2] = { (float)fi.jx, (float)fi.jy };
    glUniform2fv(uniform_julia, 1, julia);
    /* u_t */
    GLint uniform_t = glGetUniformLocation(program, "u_t");
    float tf[1] = { (float)t };
    glUniform1fv(uniform_t, 1, tf);
    /* u_dt */
    GLint uniform_dt = glGetUniformLocation(program, "u_dt");
    float dtf[1] = { (float)dt };
    glUniform1fv(uniform_dt, 1, dtf);
    /* u_n */
    GLint uniform_n = glGetUniformLocation(program,"u_n");
    glUniform1i(uniform_n, fi.n);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

    SDL_GL_SwapWindow(lwindow);
}
