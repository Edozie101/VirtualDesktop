/*
 * distortions.cpp
 *
 *  Created on: Sep 12, 2012
 *      Author: Hesham Wahba
 */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "utils.h"

#include "distortions.h"
#include "ibex.h"

#include <string.h>

#include "opengl_helpers.h"

#ifdef _WIN32
#include "ibex_win_utils.h"
#else
#ifdef __APPLE__
#include "ibex_mac_utils.h"
#endif
#endif

bool lensParametersChanged = true;
static GLuint lensFBO;
static GLuint lensTexture;

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
static glsl_shader distortion_shader_cache;

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
GLuint textureUniform;
GLuint lensTextureUniform;

GLuint ScreenCenterUniform;
GLuint LensCenterUniform;
GLuint ScaleUniform;
GLuint ScaleInUniform;
GLuint HmdWarpParamUniform;

char RiftMonitorName[33];
int RiftDisplayId;
float EyeDistance = 0.0640000030;
float DistortionK[4] = {1.00000000, 0.219999999, 0.239999995, 0.000000000};

static GLuint make_program(GLuint vertex_shader, GLuint fragment_shader)
{
  checkForErrors();
  std::cerr << "make_program" << std::endl;
  GLint program_ok;

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
  checkForErrors();
  if (!program_ok) {
    fprintf(stderr, "Failed to link shader program:\n");
    show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
    glDeleteProgram(program);
    return 0;
  } else {
    std::cerr << "Linked program" << std::endl;
  }

  a_position = glGetAttribLocation(program, "a_position");
  a_texCoord = glGetAttribLocation(program, "a_texCoord");
    
  textureUniform = glGetUniformLocation(program, "texture");
  glUniform1i(textureUniform, 0);
  lensTextureUniform = glGetUniformLocation(program, "lensTexture");
  glUniform1i(lensTextureUniform, 1);
    
  checkForErrors();
  std::cerr << "make_program end" << std::endl;
    
  return program;
}

static GLuint make_program_lens(GLuint vertex_shader, GLuint fragment_shader)
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
    
  LensCenterUniform = glGetUniformLocation(program, "LensCenter");
  ScreenCenterUniform = glGetUniformLocation(program, "ScreenCenter");
  ScaleUniform = glGetUniformLocation(program, "Scale");
  ScaleInUniform = glGetUniformLocation(program, "ScaleIn");
  HmdWarpParamUniform = glGetUniformLocation(program, "HmdWarpParam");
    
  return program;
}

static const GLfloat dataArray[] =
  {
    -1, -1, 0, 1, 0, 0, 1,
    0, -1, 0, 1, 0.5, 0, 1,
    -1,  1, 0, 1, 0, 1, 1,
    0,  1, 0, 1, 0.5, 1, 1
  };
static const GLfloat dataArray2[] =
  {
    0, -1, 0, 1, 0.5, 0, 1,
    1, -1, 0, 1, 1, 0, 1,
    0,  1, 0, 1, 0.5, 1, 1,
    1,  1, 0, 1, 1, 1, 1
  };
static const GLfloat dataArrayBoth[] =
  {
    -1, -1, 0, 1, 0, 0, 1,
    1, -1, 0, 1, 1, 0, 1,
    -1,  1, 0, 1, 0, 1, 1,
    1,  1, 0, 1, 1, 1, 1
  };

static const GLushort indices[] =
  {
    0,1,2,
    1,2,3
  };

static GLuint vao1;
static GLuint _vertexBuffer;
static GLuint _indexBuffer;
static GLuint vao2;
static GLuint _vertexBuffer2;
static GLuint _indexBuffer2;
static GLuint vaoBoth;
static GLuint _vertexBufferBoth;
static GLuint _indexBufferBoth;
int setup_buffers() {
  if(!GL_ARB_vertex_array_object)
    {
      std::cerr << "CAN'T GEN VAO!" << std::endl;
      exit(1);
    }
  std::cerr << "setup_buffers" << std::endl;
  checkForErrors();
  glGenVertexArrays(1,&vao1);
  glGenVertexArrays(1,&vao2);
    
  //    std::cerr << "gen vao1: __glewGenVertexArrays: " << __glewGenVertexArrays << std::endl;
  checkForErrors();
  std::cerr << "gen vao1 done" << std::endl;
    
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




  glBindVertexArray(vao2);

  glGenBuffers(1, &_vertexBuffer2);
  glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer2);
  glBufferData(GL_ARRAY_BUFFER, sizeof(dataArray2), dataArray2, GL_STATIC_DRAW);

  glGenBuffers(1, &_indexBuffer2);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer2);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer2);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer2);
  glEnableVertexAttribArray(a_position);
  glVertexAttribPointer(a_position, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*7, 0);
  glEnableVertexAttribArray(a_texCoord);
  glVertexAttribPointer(a_texCoord, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*7, (GLvoid*) (sizeof(GLfloat) * 4));
    
  glBindVertexArray(vaoBoth);

  glGenBuffers(1, &_vertexBufferBoth);
  glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferBoth);
  glBufferData(GL_ARRAY_BUFFER, sizeof(dataArrayBoth), dataArrayBoth, GL_STATIC_DRAW);

  glGenBuffers(1, &_indexBufferBoth);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferBoth);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferBoth);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferBoth);
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

void render_both_frames(const GLuint textureId)
{
  glUseProgram(distortion_shader.program);
    
  glBindVertexArray(vaoBoth);
    
  glActiveTexture(GL_TEXTURE0);
  glUniform1i(textureUniform, 0);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glActiveTexture(GL_TEXTURE1);
  glUniform1i(lensTextureUniform, 1);
  glBindTexture(GL_TEXTURE_2D, lensTexture);
  //
  //    glUniform1i(textureUniform, 0);
    
  glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_SHORT, 0);
    
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
  //
  glUseProgram(0);
}

void render_distorted_frame(const bool left, const GLuint textureId)
{
  glUseProgram(distortion_shader.program);

  if(left)
    glBindVertexArray(vao1);
  else
    glBindVertexArray(vao2);

  glActiveTexture(GL_TEXTURE0);
  glUniform1i(textureUniform, 0);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glActiveTexture(GL_TEXTURE1);
  glUniform1i(lensTextureUniform, 1);
  glBindTexture(GL_TEXTURE_2D, lensTexture);

  glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_SHORT, 0);
    
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);

  glUseProgram(0);
}

int init_distortion_shader()
{
  char path[2048];
  strcpy(path, mResourcePath);
  strcat(path, "/resources/shaders/distortions.v.glsl");
  distortion_shader.vertex_shader = make_shader(
						GL_VERTEX_SHADER,
						path
						);
  if (distortion_shader.vertex_shader == 0)
    return 0;

  strcpy(path, mResourcePath);
  strcat(path, "/resources/shaders/distortions.f.glsl");
  distortion_shader.fragment_shader = make_shader(
						  GL_FRAGMENT_SHADER,
						  path
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
    if(!success) {
      std::cerr << "init_distortion_shader: failure" << std::endl;
      return 0;
    }
  }
  std::cerr << "init_distortion_shader: success" << std::endl;

  return 1;
}

//--------------
bool setup_lens_fbo() {
  glGenFramebuffers(1, &lensFBO);
  glGenTextures(1, &lensTexture);
    
  glBindFramebuffer(GL_FRAMEBUFFER, lensFBO);
  glBindTexture(GL_TEXTURE_2D, lensTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height,0,//textureWidth, textureHeight, 0,
	       GL_RG, GL_FLOAT, 0);
  glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			  GL_TEXTURE_2D, lensTexture, 0);
  if (!checkForErrors()) {
    std::cerr << "Stage 1 - Problem generating lens FBO" << std::endl;
    exit(EXIT_FAILURE);
  }
    
  std::cout << "Generating lens FBO" << std::endl;
  std::cout << "FBO: " << textureWidth << "x" << textureHeight << std::endl;
    
  if (!checkForErrors() ||
      glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "Stage 2 - Problem generating lens FBO " << std::endl;
    exit(EXIT_FAILURE);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
  return 1;
}

int init_distortion_shader_cache()
{
  char path[2048];
  strcpy(path, mResourcePath);
  strcat(path, "/resources/shaders/distortions.v.glsl");
  distortion_shader_cache.vertex_shader = make_shader(
						      GL_VERTEX_SHADER,
						      path
						      );
  if (distortion_shader_cache.vertex_shader == 0)
    return 0;
    
  strcpy(path, mResourcePath);
  strcat(path, "/resources/shaders/distortions_cache.f.glsl");
  distortion_shader_cache.fragment_shader = make_shader(
							GL_FRAGMENT_SHADER,
							path
							);
  if (distortion_shader_cache.fragment_shader == 0)
    return 0;
    
  distortion_shader_cache.program = make_program_lens(
						      distortion_shader_cache.vertex_shader,
						      distortion_shader_cache.fragment_shader
						      );
  if (distortion_shader_cache.program == 0)
    return 0;
    
  if(!IRRLICHT && !OGRE3D) {
    bool success = setup_lens_fbo();
    if(!success) {
      std::cerr << "init_distortion_shader: failure" << std::endl;
      return 0;
    }
  }
  std::cerr << "init_distortion_cache_shader: success" << std::endl;
    
  return 1;
}

void render_distortion_lens(const bool left, const GLuint textureId)
{
  glUseProgram(distortion_shader_cache.program);
    
  if(left)
    glBindVertexArray(vao1);
  else
    glBindVertexArray(vao2);
    
  GLfloat scaleFactor = 0.8;//5.0/4.0;//1.0f;
  GLfloat DistortionXCenterOffset;
  if (left) {
    DistortionXCenterOffset = 0.25f;
  }
  else {
    DistortionXCenterOffset = -0.25f;
  }
  GLfloat x = (left) ? 0 : 0.5;
  GLfloat y = 0;
  GLfloat w = 0.5;
  GLfloat h = 1;
  GLfloat as = width/2.0/height;
  glUniform2f(LensCenterUniform, x + (w + DistortionXCenterOffset * 0.5f)*0.5f, y + h*0.5f);
  glUniform2f(ScreenCenterUniform, x + w*0.5f, y + h*0.5f);
  glUniform2f(ScaleUniform, (w/2.0f) * scaleFactor, (h/2.0f) * scaleFactor * as);;
  glUniform2f(ScaleInUniform, (2.0f/w), (2.0f/h) / as);
  glUniform4fv(HmdWarpParamUniform, 1, DistortionK);
	
  glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_SHORT, 0);
    
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
    
  glUseProgram(0);
}

void render_distortion_lenses() {
  if(!lensParametersChanged) return;
  lensParametersChanged = false;
    
  glBindFramebuffer(GL_FRAMEBUFFER, lensFBO);
  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);
    
  render_distortion_lens(true, lensTexture);
  render_distortion_lens(false, lensTexture);
 
  glClearColor(0,0,0,1);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
