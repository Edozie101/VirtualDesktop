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

#include "oculus/Rift.h"
#undef new
#undef delete

#ifdef WIN32
#include "ibex_win_utils.h"
#else
#ifdef __APPLE__
#include "ibex_mac_utils.h"

#endif
#endif

bool lensParametersChanged = true;
static GLuint lensFBO;
static GLuint lensTexture[2];
float screenCenterX,screenCenterY;

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
static GLint a_position;
static GLint a_texCoord;
static GLint textureUniform;
static GLint lensTextureUniform;
static GLint lensTextureUniform2;

static GLint ScreenCenterUniform;
static GLint LensCenterUniform;
static GLint ScaleUniform;
static GLint ScaleInUniform;
static GLint HmdWarpParamUniform;
static GLint ChromAbParamUniform;


static GLint ScreenCenterUniform2;
static GLint LensCenterUniform2;
static GLint ScaleUniform2;
static GLint ScaleInUniform2;
static GLint HmdWarpParamUniform2;
static GLint ChromAbParamUniform2;

char RiftMonitorName[33];
long RiftDisplayId;
float EyeDistance = 0.0640000030;
float DistortionK[4] = {1.00000000, 0.22, 0.24, 0.000000000};
float DistortionChromaticAberration[4] = {0.996f, -0.004f, 1.014f, 0.0f};
static GLuint make_program(GLuint vertex_shader, GLuint fragment_shader)
{
  checkForErrors();
  std::cerr << "make_program" << std::endl;
  GLint program_ok;

  program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  checkForErrors();

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

  LensCenterUniform = glGetUniformLocation(program, "LensCenter");
  ScreenCenterUniform = glGetUniformLocation(program, "ScreenCenter");
  ScaleUniform = glGetUniformLocation(program, "Scale");
  ScaleInUniform = glGetUniformLocation(program, "ScaleIn");
  HmdWarpParamUniform = glGetUniformLocation(program, "HmdWarpParam");
  ChromAbParamUniform = glGetUniformLocation(program, "ChromAbParam");
  textureUniform = glGetUniformLocation(program, "Texture0");
    
    if(CACHED_SHADER) {
        lensTextureUniform = glGetUniformLocation(program, "lensTexture1");
        lensTextureUniform2 = glGetUniformLocation(program, "lensTexture2");
    }
    
  glUseProgram(program);
  glUniform1i(textureUniform, 0);
    if(CACHED_SHADER) {
      glUniform1i(lensTextureUniform, 1);
      glUniform1i(lensTextureUniform2, 2);
    }

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
    
  LensCenterUniform2 = glGetUniformLocation(program, "LensCenter");
  ScreenCenterUniform2 = glGetUniformLocation(program, "ScreenCenter");
  ScaleUniform2 = glGetUniformLocation(program, "Scale");
  ScaleInUniform2 = glGetUniformLocation(program, "ScaleIn");
  HmdWarpParamUniform2 = glGetUniformLocation(program, "HmdWarpParam");
    
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
    1,3,2
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
  std::cerr << "setup_buffers" << std::endl;
  glGenVertexArrays(1,&vao1);
  glGenVertexArrays(1,&vao2);
    
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
    
    
  glGenVertexArrays(1,&vaoBoth);
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
  glBindTexture(GL_TEXTURE_2D, lensTexture[0]);
  glActiveTexture(GL_TEXTURE2);
  glUniform1i(lensTextureUniform2, 1);
  glBindTexture(GL_TEXTURE_2D, lensTexture[1]);
    
  glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_SHORT, 0);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, 0);
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
    
    
    const OVR::Util::Render::DistortionConfig *pDistortion = ((left) ? leftEye : rightEye).pDistortion;
    const OVR::Util::Render::Viewport VP = ((left) ? leftEye : rightEye).VP;
    
    // MA: This is more correct but we would need higher-res texture vertically; we should adopt this
    // once we have asymmetric input texture scale.
    const float scaleFactor = 1.0f / pDistortion->Scale;
    
	const GLfloat DistortionXCenterOffset = ((left)?1.0:-1.0)*pDistortion->XCenterOffset;
    
    const float w = float(VP.w) / float(windowWidth),
    h = float(VP.h) / float(windowHeight),
    x = float(VP.x) / float(windowWidth),
    y = float(VP.y) / float(windowHeight);
    
    const float as = float(VP.w) / float(VP.h);
    
    screenCenterX = x + w*0.5f;
    screenCenterY = y + h*0.5f;
    
	glUniform2f(LensCenterUniform, x + (w + DistortionXCenterOffset * 0.5f)*0.5f, y + h*0.5f);
    glUniform2f(ScreenCenterUniform, screenCenterX, screenCenterY);
    glUniform2f(ScaleUniform, (w/2.0f) * scaleFactor, (h/2.0f) * scaleFactor * as);;
    glUniform2f(ScaleInUniform, (2.0f/w), (2.0f/h) / as);
    glUniform4fv(HmdWarpParamUniform, 1, pDistortion->K);
	glUniform4fv(ChromAbParamUniform, 1, pDistortion->ChromaticAberration);
    
    
    glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_SHORT, 0);
    //    checkForErrors();
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    
    glUseProgram(0);
}


int init_distortion_shader()
{
    char path[2048];
    strcpy(path, mResourcePath);
    strcat(path, "/resources/shaders/distortions.v.glsl");
    std::cerr << "loading shader: " << path << std::endl;
    distortion_shader.vertex_shader = make_shader(
                                                  GL_VERTEX_SHADER,
                                                  path
                                                  );
    if (distortion_shader.vertex_shader == 0)
        return 0;
    
    strcpy(path, mResourcePath);
    
    if(CACHED_SHADER) {
        strcat(path, "/resources/shaders/distortions.f.glsl");
    } else {
        strcat(path, "/resources/shaders/distortions_full.f.glsl");
    }
    std::cerr << "loading shader: " << path << std::endl;
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
    checkForErrors();
    std::cerr << "init_distortion_shader: success" << std::endl;
    
    return 1;
}

//--------------
bool setup_lens_fbo() {
    glGenFramebuffers(1, &lensFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, lensFBO);
    
    for(int i = 0; i < 2; ++i) {
        glGenTextures(1, &(lensTexture[i]));
        glBindTexture(GL_TEXTURE_2D, lensTexture[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height,0, GL_RGBA, GL_FLOAT, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, (i==0)?GL_COLOR_ATTACHMENT0:GL_COLOR_ATTACHMENT1, lensTexture[i], 0);
        if (!checkForErrors()) {
            std::cerr << "Stage 1 - Problem generating lens FBO" << std::endl;
            exit(EXIT_FAILURE);
        }
        
        std::cout << "Generating lens FBO: " << i << std::endl;
        std::cout << "FBO: " << i << " -- " << textureWidth << "x" << textureHeight << std::endl;
    }
    
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
  std::cerr << "loading shader: " << path << std::endl;
  distortion_shader_cache.vertex_shader = make_shader(
						      GL_VERTEX_SHADER,
						      path
						      );
  if (distortion_shader_cache.vertex_shader == 0)
    return 0;
    
  strcpy(path, mResourcePath);
  strcat(path, "/resources/shaders/distortions_cache_full.f.glsl");
  std::cerr << "loading shader: " << path << std::endl;
  distortion_shader_cache.fragment_shader = make_shader(
							GL_FRAGMENT_SHADER,
							path
							);
  if (distortion_shader_cache.fragment_shader == 0)
    return 0;
    
    if(CACHED_SHADER) {
      distortion_shader_cache.program = make_program_lens(
                                  distortion_shader_cache.vertex_shader,
                                  distortion_shader_cache.fragment_shader
                                  );
      if (distortion_shader_cache.program == 0)
        return 0;
    }
    
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

void render_distortion_lens(const bool left, const GLuint textureId[2])
{
    glUseProgram(distortion_shader_cache.program);
    
    glBindVertexArray(vaoBoth);
    
    const OVR::Util::Render::DistortionConfig *pDistortion = ((left) ? leftEye : rightEye).pDistortion;
    const OVR::Util::Render::Viewport VP = ((left) ? leftEye : rightEye).VP;
    
    // MA: This is more correct but we would need higher-res texture vertically; we should adopt this
    // once we have asymmetric input texture scale.
    const float scaleFactor = 1.0f / pDistortion->Scale;
    
	const GLfloat DistortionXCenterOffset = ((left)?1.0:-1.0)*pDistortion->XCenterOffset;
    
    const float w = float(VP.w) / float(windowWidth),
    h = float(VP.h) / float(windowHeight),
    x = float(VP.x) / float(windowWidth),
    y = float(VP.y) / float(windowHeight);
    
    const float as = float(VP.w) / float(VP.h);
    
    screenCenterX = x + w*0.5f;
    screenCenterY = y + h*0.5f;
    
  glUniform2f(LensCenterUniform2, x + (w + DistortionXCenterOffset * 0.5f)*0.5f, y + h*0.5f);
  glUniform2f(ScreenCenterUniform2, screenCenterX, screenCenterY);
  glUniform2f(ScaleUniform2, (w/2.0f) * scaleFactor, (h/2.0f) * scaleFactor * as);;
  glUniform2f(ScaleInUniform2, (2.0f/w), (2.0f/h) / as);
  glUniform4fv(HmdWarpParamUniform2, 1, DistortionK);
  glUniform4fv(ChromAbParamUniform2, 1, pDistortion->ChromaticAberration);
    
    GLenum buffers[] = { GL_COLOR_ATTACHMENT0 , GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, buffers);
	
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
