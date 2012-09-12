/*
 * distortions.cpp
 *
 *  Created on: Sep 12, 2012
 *      Author: Hesham Wahba
 */

#include "utils.h"

#include <stdio.h>

#define GLX_GLXEXT_PROTOTYPES
#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glut.h>
#include <GL/glu.h>

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

static GLuint make_program(GLuint vertex_shader, GLuint fragment_shader)
{
    GLint program_ok;

    GLuint program = glCreateProgram();
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
    return program;
}
static glsl_shader distortion_shader;
int init_distortion_shader() {
	distortion_shader.vertex_shader = make_shader(
	        GL_VERTEX_SHADER,
	        "distortions.v.glsl"
	);
	if (distortion_shader.vertex_shader == 0)
		return 0;

	distortion_shader.fragment_shader = make_shader(
		GL_FRAGMENT_SHADER,
		"distortions.f.glsl"
	);
	if (distortion_shader.fragment_shader == 0)
		return 0;

	distortion_shader.program = make_program(
			distortion_shader.vertex_shader,
			distortion_shader.fragment_shader
	);
	if (distortion_shader.program == 0)
		return 0;

	return 1;
}

void render_distorted_frame(GLuint textureId)
{
    glUseProgram(distortion_shader.program);

    glUniform1f(distortion_shader.uniforms.fade_factor, distortion_shader.fade_factor);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(distortion_shader.uniforms.textures[0], 0);

//    glBindBuffer(GL_ARRAY_BUFFER, distortion_shader.vertex_buffer);
    glVertexAttribPointer(
        distortion_shader.attributes.position,  /* attribute */
        2,                                /* size */
        GL_FLOAT,                         /* type */
        GL_FALSE,                         /* normalized? */
        sizeof(GLfloat)*2,                /* stride */
        (void*)0                          /* array buffer offset */
    );
    glEnableVertexAttribArray(distortion_shader.attributes.position);

//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, distortion_shader.element_buffer);
        glDrawElements(
            GL_TRIANGLE_STRIP,  /* mode */
            4,                  /* count */
            GL_UNSIGNED_SHORT,  /* type */
            (void*)0            /* element array buffer offset */
        );

	glDisableVertexAttribArray(distortion_shader.attributes.position);
}
