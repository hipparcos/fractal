#include "renderer_hardware.h"

#include <stdio.h>
#include <stdlib.h>
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

/* static const char* fragment_shader = */
/*     "#version 330\n" */
/*     "in vec3 color;\n" */
/*     "void main() {\n" */
/*     "    gl_FragColor = vec4(color, 1.0);\n" */
/*     "}\n"; */

static SDL_Window* lwindow;
static SDL_GLContext lcontext;
static GLuint vao, vbuffer, vs, fs, program;
static GLint attrib_position; //, attrib_color;

/** read_file reads all content of filename at once.
 ** Caller is responsible for calling free on returned string. */
static char* read_file(const char* const restrict filename) {
    char* buffer = 0;
    long length;
    FILE* f = fopen(filename, "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        length = ftell (f);
        fseek(f, 0, SEEK_SET);
        buffer = malloc(length);
        if (buffer) {
            fread(buffer, 1, length, f);
        }
        fclose(f);
    } else {
        panicf("Error: can't read file `%s`.", filename);
    }
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

void rdr_hw_init(SDL_Window* window, struct fractal_info fi) {
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

    /* VAO. */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    /* Shaders. */
    vs = LoadShader(GL_VERTEX_SHADER, "vertex", vertex_shader);
    char* noise_fragment = read_file("./generator/noise.frag");
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

    /* Shader variables. */
    attrib_position = glGetAttribLocation(program, "in_position");
    glEnableVertexAttribArray(attrib_position);
    glVertexAttribPointer(attrib_position, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
    /* attrib_color = glGetAttribLocation(program, "in_color"); */
    /* glEnableVertexAttribArray(attrib_color); */
    /* glVertexAttribPointer(attrib_color, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 2)); */

    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);
}

void rdr_hw_free(void) {
    SDL_GL_DeleteContext(lcontext);
}

/* renderer interface */
void rdr_hw_set_generator(enum generator gen) {
}

void rdr_hw_set_center(double cx, double cy) {
}

void rdr_hw_set_dpp(double dpp) {
}

void rdr_hw_translate(double dx, double dy) {
}

void rdr_hw_zoom(double factor) {
}

void rdr_hw_resize(int width, int height) {
}

void rdr_hw_render(unsigned long max_iter) {
    int width, height;
    SDL_GetWindowSize(lwindow, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
    SDL_GL_SwapWindow(lwindow);
}
