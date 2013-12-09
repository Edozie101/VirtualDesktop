/*
 * SimpleWorldRendererPlugin.cpp
 *
 *  Created on: Sep 25, 2012
 *      Author: Hesham Wahba
 */

//#include <bullet/btBulletDynamicsCommon.h>

#include "../filesystem/Filesystem.h"

#include <iostream>
#include <math.h>
#include <stdio.h>

#include "../distortions.h"
#include "../opengl_helpers.h"
#include "../iphone_orientation_plugin/iphone_orientation_listener.h"

#ifdef _WIN32
#include "../ibex_win_utils.h"
#else
#ifdef __APPLE__
#include "ibex_mac_utils.h"
#else
#define HAVE_LIBJPEG 1
#include <jpeglib.h>
#include "../glm/glm.h"
#endif
#endif

#include "SimpleWorldRendererPlugin.h"

#include "oculus/Rift.h"

#include "../GLSLShaderProgram.h"
#include "ShadowBufferRenderer.h"

#include "Model.h"

glm::vec3 lightInvDir;

void copyMatrix(glm::mat4 &modelView, float M[4][4]) {
    for(int i = 0; i < 4; ++i) {
        for(int i2 = 0; i2 < 4; ++i2) {
            modelView[i][i2] = M[i][i2];
        }
    }
}
//////

void renderSphericalDisplay(double r, double numHorizontalLines, double numVerticalLines, double width, double height) {
    //    glPushMatrix();
    //    glRotated(90, 0, 1, 0);
    //
    //    glPushMatrix();
    //    for(double i = 0; i <= numHorizontalLines; i++) {
    //        const double latitude0 = M_PI * (-0.5 + (i-1.0)/numHorizontalLines) *(width/360.0)-(width*M_2_PI/360/2);
    //        const double depth0  = sin(latitude0);
    //        const double depthXY0 =  cos(latitude0);
    //
    //        const double latitude1 = M_PI * (-0.5 + i / numHorizontalLines) *(width/360.0)-(width*M_2_PI/360/2);
    //        const double depth1 = sin(latitude1);
    //        const double depthXY1 = cos(latitude1);
    //
    //        glBegin(GL_QUAD_STRIP);
    //        for(double j = 0; j <= numVerticalLines; j++) {
    //            const double longitude = 2 * M_PI * (j-1.0)/numVerticalLines *(height/360.0) -4*(height*M_2_PI/360);
    //            const double x = cos(longitude);
    //            const double y = sin(longitude);
    //
    //            glTexCoord2d(i/(numHorizontalLines+1.0), j/numVerticalLines);
    //            glNormal3f(x * depthXY0, y * depthXY0, depth0);
    //            glVertex3f(x * depthXY0, y * depthXY0, depth0);
    //
    //            glTexCoord2d((i+1.0)/(numHorizontalLines+1.0), j/numVerticalLines);
    //            glNormal3f(x * depthXY1, y * depthXY1, depth1);
    //            glVertex3f(x * depthXY1, y * depthXY1, depth1);
    //        }
    //        glEnd();
    //    }
    //    glPopMatrix();
    //
    //    glPopMatrix();
}

void renderCylindricalDisplay(double r, double numHorizontalLines, double numVerticalLines, double width, double height) {
    //    glPushMatrix();
    //    //    glRotated(90, 1, 0, 0);
    //
    //    glPushMatrix();
    //    for(double i = 0; i <= numHorizontalLines; i++) {
    //        const double latitude0 = 2*M_PI * (-0.5 + (i-1.0)/numHorizontalLines) *(width/360.0);
    //        const double depth0  = sin(latitude0);
    //        const double depthXY0 =  cos(latitude0);
    //
    //        const double latitude1 = 2*M_PI * (-0.5 + i / numHorizontalLines) *(width/360.0);
    //        const double depth1 = sin(latitude1);
    //        const double depthXY1 = cos(latitude1);
    //
    //        glBegin(GL_QUAD_STRIP);
    //        for(double j = 0; j <= numVerticalLines; j++) {
    //            const double longitude = 2 * M_PI * (j-1.0)/numVerticalLines *(height/360.0);
    //            const double x = cos(longitude);
    //            const double y = sin(longitude);
    //
    //            glTexCoord2d(1-i/(numHorizontalLines+1.0), j/numVerticalLines);
    //            glNormal3f(x * depthXY0, depthXY0, y * depthXY0);
    //            glVertex3f(depth0*r, (j/numVerticalLines)*2*r-r, depthXY0*r);
    //
    //            glTexCoord2d(1-(i+1.0)/(numHorizontalLines+1.0), j/numVerticalLines);
    //            glNormal3f(x * depthXY1, y * depthXY1, depthXY1);
    //            glVertex3f(depth1*r, (j/numVerticalLines)*2*r-r,depthXY1*r);
    //        }
    //        glEnd();
    //    }
    //    glPopMatrix();
    //
    //    glPopMatrix();
}

SimpleWorldRendererPlugin::SimpleWorldRendererPlugin() : _bringUpIbexDisplay(false) {
    reset();
}
SimpleWorldRendererPlugin::~SimpleWorldRendererPlugin() {
}

void SimpleWorldRendererPlugin::bringUpIbexDisplay() {
    _bringUpIbexDisplay = true;
}
// ---------------------------------------------------------------------------
// Function: loadSkybox
// Design:   Belongs to OpenGL component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void updateResourcePath(char *path_) {
    char path[1024];
    
    strcpy(path, mResourcePath);
    strcat(path, path_);
    strcpy(path_, path);
}

GLSLShaderProgram skyboxShaderProgram;
GLSLShaderProgram groundShaderProgram;
GLSLShaderProgram standardShaderProgram;
GLSLShaderProgram shadowProgram;
GLSLShaderProgram waterShaderProgram;

void SimpleWorldRendererPlugin::loadSkybox()
{
#ifdef _WIN32
    _skybox[0] = loadTexture("\\resources\\humus-skybox\\negz.jpg");
    _skybox[1] = loadTexture("\\resources\\humus-skybox\\posx.jpg");
    _skybox[2] = loadTexture("\\resources\\humus-skybox\\posz.jpg");
    _skybox[3] = loadTexture("\\resources\\humus-skybox\\negx.jpg");
    _skybox[4] = loadTexture("\\resources\\humus-skybox\\posy.jpg");
    _skybox[5] = loadTexture("\\resources\\humus-skybox\\negy.jpg");
#else
#ifdef __APPLE__
    _skybox[0] = loadTexture("/resources/humus-skybox/smaller/negz.jpg");
    _skybox[1] = loadTexture("/resources/humus-skybox/smaller/posx.jpg");
    _skybox[2] = loadTexture("/resources/humus-skybox/smaller/posz.jpg");
    _skybox[3] = loadTexture("/resources/humus-skybox/smaller/negx.jpg");
    _skybox[4] = loadTexture("/resources/humus-skybox/smaller/posy.jpg");
    _skybox[5] = loadTexture("/resources/humus-skybox/smaller/negy.jpg");
#else
    float sizeX = 2048;
    float sizeY = 2048;
    
    _skybox[0] = glmLoadTexture("./resources/humus-skybox/negz.jpg", GL_TRUE, GL_FALSE,
                                GL_TRUE, GL_FALSE, &sizeX, &sizeY);
    _skybox[1] = glmLoadTexture("./resources/humus-skybox/posx.jpg", GL_TRUE, GL_FALSE,
                                GL_TRUE, GL_FALSE, &sizeX, &sizeY);
    _skybox[2] = glmLoadTexture("./resources/humus-skybox/posz.jpg", GL_TRUE, GL_FALSE,
                                GL_TRUE, GL_FALSE, &sizeX, &sizeY);
    _skybox[3] = glmLoadTexture("./resources/humus-skybox/negx.jpg", GL_TRUE, GL_FALSE,
                                GL_TRUE, GL_FALSE, &sizeX, &sizeY);
    _skybox[4] = glmLoadTexture("./resources/humus-skybox/posy.jpg", GL_TRUE, GL_FALSE,
                                GL_TRUE, GL_FALSE, &sizeX, &sizeY);
    _skybox[5] = glmLoadTexture("./resources/humus-skybox/negy.jpg", GL_TRUE, GL_FALSE,
                                GL_TRUE, GL_FALSE, &sizeX, &sizeY);
#endif
#endif
    for(int i = 0; i < 6; ++i) {
        glBindTexture(GL_TEXTURE_2D, _skybox[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    std::cout << _skybox[5] << std::endl;
}

/////////////////////
void SimpleWorldRendererPlugin::renderVideoDisplayFlat(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, const glm::mat4 &depthMVP)
{
    //    ySize = videoHeight/videoWidth/2.0;
    //    glBindTexture(GL_TEXTURE_2D, videoTexture[i2]);
    //    glUseProgram(standardShaderProgram.shader.program);
    //    glUniformMatrix4fv(IbexDisplayFlatUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
    //    glUniformMatrix4fv(IbexDisplayFlatUniformLocations[1], 1, GL_FALSE, &V[0][0]);
    //    glUniformMatrix4fv(IbexDisplayFlatUniformLocations[2], 1, GL_FALSE, &M[0][0]);
    //    glUniformMatrix4fv(IbexDisplayFlatUniformLocations[4], 1, GL_FALSE, &(M*V)[0][0]);
    //
    //    glActiveTexture(GL_TEXTURE0);
    //    glBindTexture(GL_TEXTURE_2D, desktopTexture);
    //    glUniform1i(IbexDisplayFlatUniformLocations[3], 0);
    //
    //    glBindVertexArray(vaoIbexDisplayFlat);
    //    glDrawElements(GL_TRIANGLES, sizeof(IbexDisplayFlatIndices)/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
    //    glBindVertexArray(0);
    //
    //    glBindTexture(GL_TEXTURE_2D, 0);
    //
    //    glUseProgram(0);
    //
    //    if(!checkForErrors()) {
    //        std::cerr << "IbexVideoFlat failed, exiting" << std::endl;
    //        exit(0);
    //    }
    //    //    std::cerr << "Done IbexVideoFlat" << std::endl;
}

GLint ShadowUniformLocations[1];
GLint ShadowAttribLocations[3];
void loadShadowProgram() {
    shadowProgram.loadShaderProgram(mResourcePath, "/resources/shaders/depth.v.glsl", "/resources/shaders/depth.f.glsl");
    
    ShadowUniformLocations[0] = glGetUniformLocation(shadowProgram.shader.program, "MVP");
    
    ShadowAttribLocations[0] = glGetAttribLocation(shadowProgram.shader.program, "vertexPosition_modelspace");
    ShadowAttribLocations[1] = glGetAttribLocation(shadowProgram.shader.program, "vertexNormal_modelspace");
    ShadowAttribLocations[2] = glGetAttribLocation(shadowProgram.shader.program, "vertexUV");
}
void SimpleWorldRendererPlugin::renderIbexDisplayFlat(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP)
{
    //    checkForErrors();
    //    std::cerr << "start IbexDisplayFlat" << std::endl;
    
    static GLuint vaoIbexDisplayFlat = 0;
    static const GLfloat IbexDisplayFlatScale = 10;
    
    static GLint IbexDisplayFlatUniformLocations[5] = { 0, 0, 0, 0, 0};
    static GLint IbexDisplayFlatAttribLocations[3] = { 0, 0, 0 };
    
    static GLfloat IbexDisplayFlatVertices[] = {
        -1.0,  -1, 0.0, 0, 0, -1, 0, 0,
        1.0, -1.0, 0.0, 0, 0, -1, 1, 0,
        1.0, 1.0, 0.0, 0, 0, -1, 1, 1,
        -1.0, 1.0, 0.0, 0, 0, -1, 0, 1,
    };
    static GLuint vboIbexDisplayFlatVertices = 0;
    
    static GLushort IbexDisplayFlatIndices[] = {
        0, 1, 2,
        0, 2, 3
    };
    static GLuint vboIbexDisplayFlatIndices = 0;
    
    static bool first = true;
    if(first) {
        first = false;
        
        for(int i = 0; i < sizeof(IbexDisplayFlatVertices)/sizeof(GLfloat); ++i) {
            if(i%8 < 3)
                IbexDisplayFlatVertices[i] *= IbexDisplayFlatScale;
            if(i%8 == 1)
                IbexDisplayFlatVertices[i] *= height/width;
        }
        
        standardShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/emissive.v.glsl", "/resources/shaders/emissive.f.glsl");
        glUseProgram(standardShaderProgram.shader.program);
        
        
        IbexDisplayFlatUniformLocations[0] = glGetUniformLocation(standardShaderProgram.shader.program, "MVP");
        IbexDisplayFlatUniformLocations[1] = glGetUniformLocation(standardShaderProgram.shader.program, "V");
        IbexDisplayFlatUniformLocations[2] = glGetUniformLocation(standardShaderProgram.shader.program, "M");
        IbexDisplayFlatUniformLocations[3] = glGetUniformLocation(standardShaderProgram.shader.program, "textureIn");
        IbexDisplayFlatUniformLocations[4] = glGetUniformLocation(standardShaderProgram.shader.program, "MV");
        
        IbexDisplayFlatAttribLocations[0] = glGetAttribLocation(standardShaderProgram.shader.program, "vertexPosition_modelspace");
        IbexDisplayFlatAttribLocations[1] = glGetAttribLocation(standardShaderProgram.shader.program, "vertexNormal_modelspace");
        IbexDisplayFlatAttribLocations[2] = glGetAttribLocation(standardShaderProgram.shader.program, "vertexUV");
        
        glUseProgram(0);
        
        std::cerr << "setup_buffers" << std::endl;
        checkForErrors();
        glGenVertexArrays(1,&vaoIbexDisplayFlat);
        
        checkForErrors();
        std::cerr << "gen vaoIbexDisplayFlat done" << std::endl;
        
        glBindVertexArray(vaoIbexDisplayFlat);
        glGenBuffers(1, &vboIbexDisplayFlatVertices);
        glBindBuffer(GL_ARRAY_BUFFER, vboIbexDisplayFlatVertices);
        glBufferData(GL_ARRAY_BUFFER, sizeof(IbexDisplayFlatVertices), IbexDisplayFlatVertices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(IbexDisplayFlatAttribLocations[0]);
        glVertexAttribPointer(IbexDisplayFlatAttribLocations[0], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, 0);
        //        glEnableVertexAttribArray(IbexDisplayFlatAttribLocations[1]);
        //        glVertexAttribPointer(IbexDisplayFlatAttribLocations[1], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, (GLvoid*) (sizeof(GLfloat) * 3));
        glEnableVertexAttribArray(IbexDisplayFlatAttribLocations[2]);
        glVertexAttribPointer(IbexDisplayFlatAttribLocations[2], 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, (GLvoid*) (sizeof(GLfloat) * 6));
        
        
        glGenBuffers(1, &vboIbexDisplayFlatIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIbexDisplayFlatIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IbexDisplayFlatIndices), IbexDisplayFlatIndices, GL_STATIC_DRAW);
    }
    
    if(shadowPass) {
        glUseProgram(shadowProgram.shader.program);
        glUniformMatrix4fv(ShadowUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
    } else {
        glUseProgram(standardShaderProgram.shader.program);
        glUniformMatrix4fv(IbexDisplayFlatUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(IbexDisplayFlatUniformLocations[1], 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(IbexDisplayFlatUniformLocations[2], 1, GL_FALSE, &M[0][0]);
        glUniformMatrix4fv(IbexDisplayFlatUniformLocations[4], 1, GL_FALSE, &(M*V)[0][0]);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, desktopTexture);
        glUniform1i(IbexDisplayFlatUniformLocations[3], 0);
    }
    
    glBindVertexArray(vaoIbexDisplayFlat);
    glDrawElements(GL_TRIANGLES, sizeof(IbexDisplayFlatIndices)/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
    //    glBindVertexArray(0);
    //
    //    glBindTexture(GL_TEXTURE_2D, 0);
    //
    //    glUseProgram(0);
    //
    //    if(!checkForErrors()) {
    //        std::cerr << "IbexDisplayFlat failed, exiting" << std::endl;
    //        exit(0);
    //    }
    //    std::cerr << "Done IbexDisplayFlat" << std::endl;
}

// ---------------------------------------------------------------------------
// Function: renderSkybox
// Design:   Belongs to OpenGL component
// Purpose:
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
void SimpleWorldRendererPlugin::renderSkybox(const glm::mat4 &modelView, const glm::mat4 &proj)
{
    const bool useCubemap = true;//false;
    
    //    checkForErrors();
    //    std::cerr << "start skybox" << std::endl;
    
    static GLuint vaoSkybox = 0;
    static const GLfloat skyboxScale = 100;
    
    static GLint SkyBoxUniformLocations[3] = {0, 0, 0};
    static GLint SkyBoxAttribLocations[2] = {0, 0};
    
    static GLfloat cubeVertices[] = {
        // Back face
        1,-1,-1,0,0,
        -1,-1,-1,1,0,
        1,1,-1,0,1,
        -1,1,-1,1,1,
        
        // Right face
        1,-1,1, 0,0,
        1,-1,-1, 1,0,
        1,1,1,0,1,
        1,1,-1,1,1,
        
        // Front face
        -1,-1,1, 0,0,
        1,-1,1, 1,0,
        -1,1,1, 0,1,
        1,1,1, 1,1,
        
        
        
        // Left face
        -1,-1,-1,0,0,
        -1,-1,1,1,0,
        -1,1,-1,0,1,
        -1,1,1,1,1,
        
        // Top face
        -1,1,1,0,0,
        1,1,1,1,0,
        -1,1,-1,0,1,
        1,1,-1,1,1,
        
        // Bottom face
        -1,-1,-1,0,0,
        1,-1,-1,1,0,
        -1,-1,1,0,1,
        1,-1,1,1,1,
    };
    
    static const GLushort cubeIndices[] = {
        0, 2, 1,
        1, 2, 3,
        
        4, 6, 5,
        5, 6, 7,
        
        8, 10, 9,
        9, 10, 11,
        
        12, 14, 13,
        13, 14, 15,
        
        16, 18, 17,
        17, 18, 19,
        
        20, 22, 21,
        21, 22, 23
    };
    
    static GLfloat skyboxVertices[] = {
        -1.0,  1.0,  1.0,
        -1.0, -1.0,  1.0,
        1.0, -1.0,  1.0,
        1.0,  1.0,  1.0,
        -1.0,  1.0, -1.0,
        -1.0, -1.0, -1.0,
        1.0, -1.0, -1.0,
        1.0,  1.0, -1.0
    };
    static GLuint vboSkyboxVertices = 0;
    
    static GLushort skyboxIndices[] = {
        0, 2, 1,
        0, 3, 2,
        
        3, 6, 2,
        3, 7, 6,
        
        7, 5, 6,
        7, 4, 5,
        
        4, 1, 5,
        4, 0, 1,
        
        0, 7, 3,
        0, 4, 7,
        
        1, 2, 6,
        1, 6, 5
    };
    static GLuint vboSkyboxIndices = 0;
    
    static bool first = true;
    if(first) {
        first = false;
        
        for(int i = 0; i < sizeof(skyboxVertices)/sizeof(GLfloat); ++i) {
            skyboxVertices[i] *= skyboxScale;
        }
        for(int i = 0; i < sizeof(cubeVertices)/sizeof(GLfloat); ++i) {
            if(i%5 < 3) {
                cubeVertices[i] *= skyboxScale;
            }
        }
        
        _skycube = 0;
        if(useCubemap) {
            skyboxShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/skybox.v.glsl", "/resources/shaders/skybox.f.glsl");
        } else {
            skyboxShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/skybox-cube.v.glsl", "/resources/shaders/skybox-cube.f.glsl");
        }
        glUseProgram(skyboxShaderProgram.shader.program);
        
        
        SkyBoxUniformLocations[0] = glGetUniformLocation(skyboxShaderProgram.shader.program, "modelViewMatrix");
        SkyBoxUniformLocations[1] = glGetUniformLocation(skyboxShaderProgram.shader.program, "projectionMatrix");
        if(useCubemap) {
            SkyBoxUniformLocations[2] = glGetUniformLocation(skyboxShaderProgram.shader.program, "cubemap");
        } else {
            SkyBoxUniformLocations[2] = glGetUniformLocation(skyboxShaderProgram.shader.program, "textureIn");
        }
        
        SkyBoxAttribLocations[0] = glGetAttribLocation(skyboxShaderProgram.shader.program, "vertexPosition_modelspace");
        if(!useCubemap) {
            SkyBoxAttribLocations[1] = glGetAttribLocation(skyboxShaderProgram.shader.program, "vertexUV");
        }
        
        glUseProgram(0);
        
        std::cerr << "setup_buffers" << std::endl;
        checkForErrors();
        glGenVertexArrays(1,&vaoSkybox);
        
        checkForErrors();
        std::cerr << "gen vaoSkybox done" << std::endl;
        
        glBindVertexArray(vaoSkybox);
        glGenBuffers(1, &vboSkyboxVertices);
        glBindBuffer(GL_ARRAY_BUFFER, vboSkyboxVertices);
        if(useCubemap) {
            glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
        } else {
            glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
        }
        
        glGenBuffers(1, &vboSkyboxIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSkyboxIndices);
        if(useCubemap) {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), skyboxIndices, GL_STATIC_DRAW);
        } else {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
        }
        
        if(useCubemap) {
            glEnableVertexAttribArray(SkyBoxAttribLocations[0]);
            glVertexAttribPointer(SkyBoxAttribLocations[0], 3, GL_FLOAT, GL_FALSE, 0, 0);
        } else {
            glEnableVertexAttribArray(SkyBoxAttribLocations[0]);
            glVertexAttribPointer(SkyBoxAttribLocations[0], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*5, 0);
            glEnableVertexAttribArray(SkyBoxAttribLocations[1]);
            glVertexAttribPointer(SkyBoxAttribLocations[1], 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*5, (void*)(sizeof(GLfloat)*3));
        }
        
        const char *paths[6] = {
            "/resources/humus-skybox/smaller/posx.jpg",
            "/resources/humus-skybox/smaller/negx.jpg",
            "/resources/humus-skybox/smaller/posy.jpg",
            "/resources/humus-skybox/smaller/negy.jpg",
            "/resources/humus-skybox/smaller/posz.jpg",
            "/resources/humus-skybox/smaller/negz.jpg"
        };
        _skycube = loadCubemapTextures(paths);
        if(!useCubemap) {
            loadSkybox();
        }
    }
    
    glUseProgram(skyboxShaderProgram.shader.program);
    glUniformMatrix4fv(SkyBoxUniformLocations[0], 1, GL_FALSE, &modelView[0][0]);
    glUniformMatrix4fv(SkyBoxUniformLocations[1], 1, GL_FALSE, &proj[0][0]);
    
    glActiveTexture(GL_TEXTURE0);
    if(useCubemap) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, _skycube);
        glUniform1i(SkyBoxUniformLocations[2], 0);
        
        glBindVertexArray(vaoSkybox);
        glDrawElements(GL_TRIANGLES, sizeof(skyboxIndices)/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);
        
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    } else {
        glBindVertexArray(vaoSkybox);
        
        glUniform1i(SkyBoxUniformLocations[2], 0);
        for(int i = 0; i < 6; ++i) {
            glBindTexture(GL_TEXTURE_2D, _skybox[i]);
            
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)(i*6*sizeof(GLushort)));//&cubeIndices[i*6]);
        }
        //        glBindTexture(GL_TEXTURE_2D, 0);
        //
        //        glBindVertexArray(0);
    }
    //
    //    glUseProgram(0);
    //
    //    if(!checkForErrors()) {
    //        std::cerr << "skybox failed, exiting" << std::endl;
    //        exit(0);
    //    }
    //    std::cerr << "Done skybox" << std::endl;
}

void SimpleWorldRendererPlugin::renderWater(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP, const double &time)
{
    //    checkForErrors();
    //    std::cerr << "Loading Water texture" << std::endl;
    //#ifdef _WIN32
    //    static const GLuint WaterTexture = loadTexture("\\resources\\humus-skybox\\posy.jpg");
    //    //        orientation = getRiftOrientation();
    //#else
    //#ifdef __APPLE__
    //    static const GLuint WaterTexture = loadTexture("/resources/humus-skybox/smaller/posy.jpg");
    //#else
    //    static float sizeX = 64;
    //    static float sizeY = 64;
    //    static const GLuint WaterTexture = glmLoadTexture("./resources/humus-skybox/posy.jpg", GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE, &sizeX, &sizeY);
    //#endif
    //    // gluInvertMatrix(get_orientation(), orientation);
    //#endif
    
    //    checkForErrors();
    //    std::cerr << "start Water" << std::endl;
    
    static GLuint vaoWater = 0;
    static const GLfloat WaterScale = 2000;
    
    static GLint WaterUniformLocations[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    static GLint WaterAttribLocations[3] = {0, 0, 0};
    
    static GLfloat WaterVertices[] = {
        -1.0,  -10.0, -1.0, 0, 1, 0, 0, 0,
        1.0, -10.0, -1.0, 0, 1, 0, 1, 0,
        1.0, -10.0, 1.0, 0, 1, 0, 1, 1,
        -1.0, -10.0, 1.0, 0, 1, 0, 0, 1
    };
    static GLuint vboWaterVertices = 0;
    
    static GLushort WaterIndices[] = {
        0, 2, 1,
        0, 3, 2
    };
    static GLuint vboWaterIndices = 0;
    
    static bool first = true;
    if(first) {
        first = false;
        
        for(int i = 0; i < sizeof(WaterVertices)/sizeof(GLfloat); ++i) {
            if(i%8 != 1 && (i%8 < 3 || i%8 > 5)) {
                WaterVertices[i] *= WaterScale;
            }
        }
        
        //waterShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/water.v.glsl", "/resources/shaders/water.f.glsl");
        waterShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/water-reflect.v.glsl", "/resources/shaders/water-reflect.f.glsl");
        glUseProgram(waterShaderProgram.shader.program);
        
        
        WaterUniformLocations[0] = glGetUniformLocation(waterShaderProgram.shader.program, "MVP");
        WaterUniformLocations[1] = glGetUniformLocation(waterShaderProgram.shader.program, "V");
        WaterUniformLocations[2] = glGetUniformLocation(waterShaderProgram.shader.program, "M");
        WaterUniformLocations[3] = glGetUniformLocation(waterShaderProgram.shader.program, "textureIn");
        WaterUniformLocations[4] = glGetUniformLocation(waterShaderProgram.shader.program, "MV");
        
        WaterUniformLocations[5] = glGetUniformLocation(waterShaderProgram.shader.program, "DepthBiasMVP");
        WaterUniformLocations[6] = glGetUniformLocation(waterShaderProgram.shader.program, "shadowTexture");
        WaterUniformLocations[7] = glGetUniformLocation(waterShaderProgram.shader.program, "time");
        WaterUniformLocations[8] = glGetUniformLocation(waterShaderProgram.shader.program, "eyePosition_modelspace");
        
        WaterAttribLocations[0] = glGetAttribLocation(waterShaderProgram.shader.program, "vertexPosition_modelspace");
        WaterAttribLocations[1] = glGetAttribLocation(waterShaderProgram.shader.program, "vertexNormal_modelspace");
        WaterAttribLocations[2] = glGetAttribLocation(waterShaderProgram.shader.program, "vertexUV");
        
        glUseProgram(0);
        
        std::cerr << "setup_buffers" << std::endl;
        checkForErrors();
        glGenVertexArrays(1,&vaoWater);
        
        checkForErrors();
        std::cerr << "gen vaoWater done" << std::endl;
        
        glBindVertexArray(vaoWater);
        glGenBuffers(1, &vboWaterVertices);
        glBindBuffer(GL_ARRAY_BUFFER, vboWaterVertices);
        glBufferData(GL_ARRAY_BUFFER, sizeof(WaterVertices), WaterVertices, GL_STATIC_DRAW);
        
        glGenBuffers(1, &vboWaterIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboWaterIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(WaterIndices), WaterIndices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(WaterAttribLocations[0]);
        glVertexAttribPointer(WaterAttribLocations[0], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, 0);
        //        glEnableVertexAttribArray(WaterAttribLocations[1]);
        //        glVertexAttribPointer(WaterAttribLocations[1], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, (GLvoid*) (sizeof(GLfloat) * 3));
        //        glEnableVertexAttribArray(WaterAttribLocations[2]);
        //        glVertexAttribPointer(WaterAttribLocations[2], 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, (GLvoid*) (sizeof(GLfloat) * 6));
    }
    
    glBindVertexArray(vaoWater);
    if(shadowPass) {
        glUseProgram(shadowProgram.shader.program);
        glUniformMatrix4fv(ShadowUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
    } else {
        glUseProgram(waterShaderProgram.shader.program);
        if(WaterUniformLocations[0] > -1) glUniformMatrix4fv(WaterUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
        if(WaterUniformLocations[1] > -1) glUniformMatrix4fv(WaterUniformLocations[1], 1, GL_FALSE, &V[0][0]);
        if(WaterUniformLocations[2] > -1) glUniformMatrix4fv(WaterUniformLocations[2], 1, GL_FALSE, &M[0][0]);
        if(WaterUniformLocations[4] > -1) glUniformMatrix4fv(WaterUniformLocations[4], 1, GL_FALSE, &(M*V)[0][0]);
        if(WaterUniformLocations[7] > -1) glUniform1f(WaterUniformLocations[7], (float)time);//*8);
        //        if(WaterUniformLocations[8] > -1) glUniform3f(WaterUniformLocations[8], ;
        
        if(WaterUniformLocations[5] > -1) {
            glUniformMatrix4fv(WaterUniformLocations[5], 1, GL_FALSE, &depthMVP[0][0]);
        }
        
        if(WaterUniformLocations[3] > -1) {
            glActiveTexture(GL_TEXTURE0);
            //            glBindTexture(GL_TEXTURE_2D, WaterTexture);
            glBindTexture(GL_TEXTURE_CUBE_MAP, _skycube);
            glUniform1i(WaterUniformLocations[3], 0);
        }
        if(WaterUniformLocations[6] > -1) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, shadowMapDepthTextureId);
            glUniform1i(WaterUniformLocations[6], 1);
        }
    }
    
    glDrawElements(GL_TRIANGLES, sizeof(WaterIndices)/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
    //    glBindVertexArray(0);
    //
    //    if(WaterUniformLocations[3] > -1) {
    //        glActiveTexture(GL_TEXTURE0);
    //        glBindTexture(GL_TEXTURE_2D, 0);
    //    }
    //    if(WaterUniformLocations[6] > -1) {
    //        glActiveTexture(GL_TEXTURE1);
    //        glBindTexture(GL_TEXTURE_2D, 0);
    //    }
    //
    //    glUseProgram(0);
    //
    //    if(!checkForErrors()) {
    //        std::cerr << "Water water, exiting" << std::endl;
    //        exit(0);
    //    }
    //    std::cerr << "Done Water" << std::endl;
}

void SimpleWorldRendererPlugin::renderGround(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP, const double &time)
{
    //    checkForErrors();
    //    std::cerr << "Loading ground texture" << std::endl;
#ifdef _WIN32
    static const GLuint groundTexture = loadTexture("\\resources\\humus-skybox\\negy.jpg");
    //        orientation = getRiftOrientation();
#else
#ifdef __APPLE__
    static const GLuint groundTexture = loadTexture("/resources/humus-skybox/smaller/negy.jpg");
#else
    static float sizeX = 64;
    static float sizeY = 64;
    static const GLuint groundTexture = glmLoadTexture("./resources/humus-skybox/negy.jpg", GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE, &sizeX, &sizeY);
#endif
    // gluInvertMatrix(get_orientation(), orientation);
#endif
    
    //    checkForErrors();
    //    std::cerr << "start Ground" << std::endl;
    
    static GLuint vaoGround = 0;
    static const GLfloat GroundScale = 1000;
    
    static GLint GroundUniformLocations[7] = { 0, 0, 0, 0, 0, 0, 0};
    static GLint GroundAttribLocations[3] = { 0, 0, 0 };
    
    static GLfloat GroundVertices[] = {
        -1.0,  -10.0, -1.0, 0, 1, 0, 0, 0,
        1.0, -10.0, -1.0, 0, 1, 0, 1, 0,
        1.0, -10.0, 1.0, 0, 1, 0, 1, 1,
        -1.0, -10.0, 1.0, 0, 1, 0, 0, 1
    };
    static GLuint vboGroundVertices = 0;
    
    static GLushort GroundIndices[] = {
        0, 2, 1,
        0, 3, 2
    };
    static GLuint vboGroundIndices = 0;
    
    static bool first = true;
    if(first) {
        first = false;
        
        for(int i = 0; i < sizeof(GroundVertices)/sizeof(GLfloat); ++i) {
            if(i%8 != 1 && (i%8 < 3 || i%8 > 5)) {
                GroundVertices[i] *= GroundScale;
            }
        }
        
        //        groundShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/ground.v.glsl", "/resources/shaders/ground.f.glsl");
        groundShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/standard.v.glsl", "/resources/shaders/standard.f.glsl");
        glUseProgram(groundShaderProgram.shader.program);
        
        
        GroundUniformLocations[0] = glGetUniformLocation(groundShaderProgram.shader.program, "MVP");
        GroundUniformLocations[1] = glGetUniformLocation(groundShaderProgram.shader.program, "V");
        GroundUniformLocations[2] = glGetUniformLocation(groundShaderProgram.shader.program, "M");
        GroundUniformLocations[3] = glGetUniformLocation(groundShaderProgram.shader.program, "textureIn");
        GroundUniformLocations[4] = glGetUniformLocation(groundShaderProgram.shader.program, "MV");
        
        GroundUniformLocations[5] = glGetUniformLocation(groundShaderProgram.shader.program, "DepthBiasMVP");
        GroundUniformLocations[6] = glGetUniformLocation(groundShaderProgram.shader.program, "shadowTexture");
        
        GroundAttribLocations[0] = glGetAttribLocation(groundShaderProgram.shader.program, "vertexPosition_modelspace");
        GroundAttribLocations[1] = glGetAttribLocation(groundShaderProgram.shader.program, "vertexNormal_modelspace");
        GroundAttribLocations[2] = glGetAttribLocation(groundShaderProgram.shader.program, "vertexUV");
        
        glUseProgram(0);
        
        std::cerr << "setup_buffers" << std::endl;
        checkForErrors();
        glGenVertexArrays(1,&vaoGround);
        
        checkForErrors();
        std::cerr << "gen vaoGround done" << std::endl;
        
        glBindVertexArray(vaoGround);
        glGenBuffers(1, &vboGroundVertices);
        glBindBuffer(GL_ARRAY_BUFFER, vboGroundVertices);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GroundVertices), GroundVertices, GL_STATIC_DRAW);
        
        glGenBuffers(1, &vboGroundIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboGroundIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GroundIndices), GroundIndices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(GroundAttribLocations[0]);
        glVertexAttribPointer(GroundAttribLocations[0], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, 0);
        glEnableVertexAttribArray(GroundAttribLocations[1]);
        glVertexAttribPointer(GroundAttribLocations[1], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, (GLvoid*) (sizeof(GLfloat) * 3));
        glEnableVertexAttribArray(GroundAttribLocations[2]);
        glVertexAttribPointer(GroundAttribLocations[2], 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, (GLvoid*) (sizeof(GLfloat) * 6));
    }
    
    glBindVertexArray(vaoGround);
    if(shadowPass) {
        glUseProgram(shadowProgram.shader.program);
        glUniformMatrix4fv(ShadowUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
    } else {
        glUseProgram(groundShaderProgram.shader.program);
        if(GroundUniformLocations[0] > -1) glUniformMatrix4fv(GroundUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
        if(GroundUniformLocations[1] > -1) glUniformMatrix4fv(GroundUniformLocations[1], 1, GL_FALSE, &V[0][0]);
        if(GroundUniformLocations[2] > -1) glUniformMatrix4fv(GroundUniformLocations[2], 1, GL_FALSE, &M[0][0]);
        if(GroundUniformLocations[4] > -1) glUniformMatrix4fv(GroundUniformLocations[4], 1, GL_FALSE, &(M*V)[0][0]);
        
        if(GroundUniformLocations[5] > -1) {
            glUniformMatrix4fv(GroundUniformLocations[5], 1, GL_FALSE, &depthMVP[0][0]);
        }
        
        if(GroundUniformLocations[3] > -1) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, groundTexture);
            glUniform1i(GroundUniformLocations[3], 0);
        }
        if(GroundUniformLocations[6] > -1) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, shadowMapDepthTextureId);
            glUniform1i(GroundUniformLocations[6], 1);
        }
    }
    
    glDrawElements(GL_TRIANGLES, sizeof(GroundIndices)/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
    //    glBindVertexArray(0);
    //
    //    if(GroundUniformLocations[3] > -1) {
    //        glActiveTexture(GL_TEXTURE0);
    //        glBindTexture(GL_TEXTURE_2D, 0);
    //    }
    //    if(GroundUniformLocations[6] > -1) {
    //        glActiveTexture(GL_TEXTURE1);
    //        glBindTexture(GL_TEXTURE_2D, 0);
    //    }
    //
    //    glUseProgram(0);
    //
    //    if(!checkForErrors()) {
    //        std::cerr << "Ground failed, exiting" << std::endl;
    //        exit(0);
    //    }
    //    std::cerr << "Done Ground" << std::endl;
}

void SimpleWorldRendererPlugin::init() {
    reset();
}

void SimpleWorldRendererPlugin::reset() {
    ibexDisplayModelTransform = glm::mat4(glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -10.0f)));
}
void SimpleWorldRendererPlugin::render(const glm::mat4 &proj_, const glm::mat4 &view_, const glm::mat4 &playerCamera_, const glm::mat4 &playerRotation_, const glm::vec3 &playerPosition_, bool shadowPass, const glm::mat4 &depthBiasMVP, const double &time) {
    glm::mat4 view(view_);
    glm::mat4 model;
    
    if(!shadowPass) {
        glDepthMask(GL_FALSE);
        renderSkybox(view*playerRotation_, proj_);
        glDepthMask(GL_TRUE);
    }
    view *= playerCamera_;
    
    glm::mat4 PV(proj_*view);
    
    //    static int i = 0;
    //    i = ++i%(int)(360./0.5);
    //    model = model*glm::rotate(0.5f*i, 0.f, 1.f, 0.f);
    
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    model = glm::translate(ibexDisplayModelTransform, glm::vec3(0,-getPlayerHeightAtPosition(-ibexDisplayModelTransform[3][0], -ibexDisplayModelTransform[3][2]-10),0));
    renderIbexDisplayFlat(PV*model, view, model, shadowPass, depthBiasMVP*model);
    //    renderVideoDisplayFlat(PV*model, view, model, depthBiasMVP*model);
    glEnable(GL_CULL_FACE);
//    glCullFace(GL_BACK);
    
    if(shadowPass) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
//        glCullFace(GL_FRONT_AND_BACK);
//    glDisable(GL_CULL_FACE);
    }
    if(showGround) {
        //model = glm::mat4();
        //renderGround(PV*model, view, model, shadowPass, depthBiasMVP*model, time);

        glm::mat4 treeMat;// = glm::scale(glm::mat4(), 10.0f, 10.0f, 10.0f);
        glDisable(GL_CULL_FACE);
        srand(1982);
        for(int i = 0; i < 10; ++i) {
            
            int x = rand()%6000-3000;
            int z = rand()%6000-3000;
            float height = getPlayerHeightAtPosition(x, z);
            if(height >= -20) continue;
            model = glm::translate(treeMat, glm::vec3(-x,-height-20.0f,-z));
            model = glm::scale(model, 40.0f, 40.0f, 40.0f);
            treeModel.renderScene(standardShaderProgram.shader.program, PV*model, view, model, shadowPass, depthBiasMVP*model);
            //                std::cerr << x << " " << z << std::endl;
        }
        glEnable(GL_CULL_FACE);
        
        if(!shadowPass) {
            model = glm::mat4();
            terrain.renderGround(PV*model, view, model, shadowPass, depthBiasMVP*model, time);
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            model = glm::translate(model, -glm::vec3(playerPosition_.x, 0, playerPosition_.z));
            renderWater(PV*model, view, model, shadowPass, depthBiasMVP*model, time);
            glDisable(GL_BLEND);
        }
    }
    if(shadowPass) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
}

GLfloat SimpleWorldRendererPlugin::getPlayerHeightAtPosition(GLfloat x, GLfloat z) {
    const GLfloat playerHeight = 10.;
    GLfloat y = -terrain.getNoiseHeight((-x)/50., (-z)/50.)-playerHeight;
    if(y > -playerHeight) y = -playerHeight;
    return y-playerHeight;
}
void SimpleWorldRendererPlugin::step(const Desktop3DLocation &loc, double timeDiff_, const double &time_) {
    static const bool ENABLE_SHADOWMAPPING = true;
    
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    static bool first = true;
    if(first) {
        first = false;
        
        //Import3DFromFile("/Users/hesh/Downloads/59269_low_poly_foliege_blend_by_EugeneKiver/tree.dae");
        //Import3DFromFile("/Users/hesh/Downloads/59269_low_poly_foliege_blend_by_EugeneKiver/tree2.dae");
        treeModel.Import3DFromFile("/Users/hesh/Downloads/59269_low_poly_foliege_blend_by_EugeneKiver/tree3.dae");
        //Import3DFromFile("/Users/hesh/Downloads/palmtree3DS/palmtree.3ds");
        
        glClearColor(0, 0, 0, 1);
        
        if(ENABLE_SHADOWMAPPING) {
            loadShadowProgram();
            generateShadowFBO();
        }
        
        //terrain.loadHeightmap("/resources/terrain.raw", 1024, 1024);
        //terrain.loadHeightmap("/resources/terrain-128.raw", 128,128);
        terrain.generateNoiseTerrain(128,128,
                                     50.0, 1.0, 50.0,//50.,1.,50.,
                                     -20.0,500.0,
                                     //0.7/256., 1.4/256., 3./256.);
                                     0.7, 1.4, 3.);
    }
    
    glm::mat4 modelView;
    glm::mat4 view;
    glm::mat4 proj;
    //    copyMatrix(view, lightsourceMatrix);
    //    copyMatrix(proj, (getRiftOrientationNative()*stereo.Projection.Transposed()).M);
    
    
    glm::mat4 playerRotation(glm::rotate(glm::mat4(1.0f), (float)loc.getXRotation(), glm::vec3(1, 0, 0)));
    playerRotation = glm::rotate(playerRotation, (float)loc.getYRotation(), glm::vec3(0, 1, 0));
    glm::vec3 playerPosition((float)loc.getXPosition(), loc.getYPosition(), loc.getZPosition());
    playerPosition.y = getPlayerHeightAtPosition(playerPosition.x, playerPosition.z)-playerPosition.y;
    glm::mat4 playerCamera(glm::translate(playerRotation,
                                          playerPosition));
    
    
    if(_bringUpIbexDisplay) {
        _bringUpIbexDisplay = false;
        ibexDisplayModelTransform = glm::translate(glm::mat4(), glm::vec3(-playerPosition.x, 0, -playerPosition.z-10));//glm::inverse(playerCamera), glm::vec3(0,0,-10));
    }
    
    
    //    static glm::mat4 lightView = glm::lookAt(glm::vec3(0.f,0.f,0.f), glm::vec3(4.f,4.f,4.f), glm::vec3(0,1,0));
    //    static glm::mat4 lightProj = glm::ortho(-100.f, 100.f, -100.f, 100.f, -100.f, 100.f);
    
    lightInvDir = glm::vec3(0.0f,300.0f,250.0f);//playerPosition.x, playerPosition.y, playerPosition.z);//0,1000,1000);///0.5f,2,2);
    
    // Compute the MVP matrix from the light's point of view
    glm::mat4 lightProj = glm::ortho<float>(-4000,4000,-4000,4000,-4000,4000);//-10,10,-10,10,-10,20);
    //glm::mat4 lightView = glm::lookAt(lightInvDir, glm::vec3(playerPosition.x-1,playerPosition.y-1,playerPosition.z-1), glm::vec3(0,1,0));//glm::vec3(-1,-1,-1), glm::vec3(0,1,0));
    glm::mat4 lightView = glm::lookAt(lightInvDir, glm::vec3(0.0f,0.0f, 0.0f), glm::vec3(0.0f,1.0f,0.0f));//glm::vec3(-1,-1,-1), glm::vec3(0,1,0));
    glm::mat4 depthMVP = lightProj*lightView;
    //    // spotlight
    //    glm::vec3 lightPos(5, 20, 20);
    //    lightProj = glm::perspective<float>(45.0f, 1.0f, 2.0f, 50.0f);
    //    lightView = glm::lookAt(lightPos, lightPos-lightInvDir, glm::vec3(0,1,0));
    //    depthMVP = lightProj*lightView;
    
//    lightInvDir = glm::vec3(4,4,4);///0.5f,2,2);
//    
//    // Compute the MVP matrix from the light's point of view
//    lightProj = glm::ortho<float>(-30,30,-30,30,-100,3000);//-10,10,-10,10,-10,20);
//    lightView = glm::lookAt(lightInvDir, glm::vec3(-1,-1,-1), glm::vec3(0,1,0));
//    depthMVP = lightProj*lightView;

    
    static glm::mat4 biasMatrix(
                                0.5, 0.0, 0.0, 0.0,
                                0.0, 0.5, 0.0, 0.0,
                                0.0, 0.0, 0.5, 0.0,
                                0.5, 0.5, 0.5, 1.0
                                );
    glm::mat4 depthBiasMVP = biasMatrix*depthMVP;
    if(ENABLE_SHADOWMAPPING) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glFrontFace(GL_CCW);
        
        // render shadowmap
        bindShadowFBO();
        render(lightProj, lightView, glm::mat4(), glm::mat4(), glm::vec3(), true, depthBiasMVP, time_);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, fbos[0]);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glViewport(0, 0, textureWidth,textureHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (!checkForErrors()) {
        std::cerr << "GL ISSUE -- SimpleWorldRendererPlugin::step" << std::endl;
        //exit(EXIT_FAILURE);
    }
    glEnable(GL_SCISSOR_TEST);
    for (int i2 = 0; i2 < 2; ++i2) {
        const bool left = (i2 == 0);
        const OVR::Util::Render::StereoEyeParams &stereo = (left) ? leftEye : rightEye;
        glViewport(stereo.VP.x*renderScale,stereo.VP.y*renderScale,stereo.VP.w*renderScale,stereo.VP.h*renderScale);
        glScissor(stereo.VP.x*renderScale,stereo.VP.y*renderScale,stereo.VP.w*renderScale,stereo.VP.h*renderScale);
        
        copyMatrix(view,stereo.ViewAdjust.Transposed().M);
        copyMatrix(proj, (getRiftOrientationNative()*stereo.Projection.Transposed()).M);
        
        if(useLightPerspective) {
            proj = lightProj;
            view = lightView;
        }
        
        // render normally
        render(proj, view, playerCamera, playerRotation, playerPosition, false, depthBiasMVP, time_);
    }
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_FRAMEBUFFER_SRGB);
    glFlush();
    
    // Barrel distort-only for now
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0,0, windowWidth, windowHeight);
    if(CACHED_SHADER) {
        if(lensParametersChanged) {
            render_distortion_lenses();
        }
        
        render_both_frames(textures[0]);
    } else {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        render_distorted_frame(true, textures[0]);
        render_distorted_frame(false, textures[0]);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
    }
}

Window SimpleWorldRendererPlugin::getWindowID() {
    return ::window;
}

bool SimpleWorldRendererPlugin::needsSwapBuffers() {
    return doubleBuffered;
}
