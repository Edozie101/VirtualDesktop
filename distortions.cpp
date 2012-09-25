/*
 * distortions.cpp
 *
 *  Created on: Sep 12, 2012
 *      Author: Hesham Wahba
 */
#include <stdio.h>

#include "utils.h"

#include "distortions.h"
#include "ibex.h"

typedef struct {
    GLuint vertex_shader, fragment_shader, program;

    struct {
        GLint fade_factor;
        GLint textures[2];
    } uniforms;

    struct {
        GLint position;
    } attributes;

    GLfloat fade_factor;
} glsl_shader;

static glsl_shader distortion_shader;

static void show_info_log(
    GLuint object,
    PFNGLGETSHADERIVPROC glGet__iv,
    PFNGLGETSHADERINFOLOGPROC glGet__InfoLog
)
{
    GLint log_length;
    char *log;

    glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
    log = (char*)malloc(log_length);
    glGet__InfoLog(object, log_length, NULL, log);
    fprintf(stderr, "%s", log);
    free(log);
}

static GLuint make_shader(GLenum type, const char *filename)
{
    GLint length;
    GLchar *source = (GLchar*)file_contents(filename, &length);
    GLuint shader;
    GLint shader_ok;

    if (!source)
        return 0;

    shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar**)&source, &length);
	free(source);
	glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
    if (!shader_ok) {
        fprintf(stderr, "Failed to compile %s:\n", filename);
        show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint program;
GLuint a_position;
GLuint a_texCoord;
GLuint offsetUniform;
GLuint textureUniform;
static GLuint make_program(GLuint vertex_shader, GLuint fragment_shader)
{
    GLint program_ok;

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok) {
        fprintf(stderr, "Failed to link shader program:\n");
        show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(program);
        return 0;
    }

    a_position = glGetAttribLocation(program, "a_position");
    a_texCoord = glGetAttribLocation(program, "a_texCoord");

    offsetUniform = glGetUniformLocation(program, "offsetUniform");

    textureUniform = glGetUniformLocation(program, "texture");

    return program;
}

static const GLfloat dataArray[] =
{
    -0.5, -1, 0, 1, -1, -1, 1,
    0.5, -1, 0, 1, 1, -1, 1,
    -0.5,  1, 0, 1, -1, 1, 1,
    0.5,  1, 0, 1, 1, 1, 1
};

static const GLushort indices[] =
{
    0,1,2,
    1,2,3
};

GLuint vao1;
GLuint _vertexBuffer;
GLuint _indexBuffer;
int setup_buffers() {
  glGenVertexArrays(1,&vao1);
  glBindVertexArray(vao1);

  glGenBuffers(1, &_vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(dataArray), dataArray, GL_STATIC_DRAW);

  glGenBuffers(1, &_indexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
  glEnableVertexAttribArray(a_position);
  glVertexAttribPointer(a_position, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*7, 0);
  glEnableVertexAttribArray(a_texCoord);
  glVertexAttribPointer(a_texCoord, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*7, (GLvoid*) (sizeof(GLfloat) * 4));

//  glEnableVertexAttribArray(0);
//  glBindBuffer(GL_ARRAY_BUFFER, 0);
//  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return 1;
}

void render_distorted_frame(const bool left, const GLuint textureId)
{
    glUseProgram(distortion_shader.program);

    glBindVertexArray(vao1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1f(offsetUniform, (left)?-0.5:0.5);
    glUniform1i(textureUniform, 0);

    glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_SHORT, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}

int init_distortion_shader()
{
        distortion_shader.vertex_shader = make_shader(
                GL_VERTEX_SHADER,
                "./resources/shaders/distortions.v.glsl"
        );
        if (distortion_shader.vertex_shader == 0)
                return 0;

        distortion_shader.fragment_shader = make_shader(
                GL_FRAGMENT_SHADER,
                "./resources/shaders/distortions.f.glsl"
        );
        if (distortion_shader.fragment_shader == 0)
          return 0;

        distortion_shader.program = make_program(
                        distortion_shader.vertex_shader,
                        distortion_shader.fragment_shader
        );
        if (distortion_shader.program == 0)
          return 0;

        if(!IRRLICHT && !OGRE3D) {
          bool success = setup_buffers();
          if(!success)
            return 0;
        }

        return 1;
}
