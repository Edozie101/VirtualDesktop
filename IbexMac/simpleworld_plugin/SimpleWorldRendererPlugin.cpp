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


void copyMatrix(glm::mat4 &modelView, float M[4][4]) {
    for(int i = 0; i < 4; ++i) {
        for(int i2 = 0; i2 < 4; ++i2) {
            modelView[i][i2] = M[i][i2];
        }
    }
}
//////

void renderSphericalDisplay(double r, double numHorizontalLines, double numVerticalLines, double width, double height) {
    glPushMatrix();
    glRotated(90, 0, 1, 0);
    
    glPushMatrix();
    for(double i = 0; i <= numHorizontalLines; i++) {
        const double latitude0 = M_PI * (-0.5 + (i-1.0)/numHorizontalLines) *(width/360.0)-(width*M_2_PI/360/2);
        const double depth0  = sin(latitude0);
        const double depthXY0 =  cos(latitude0);
        
        const double latitude1 = M_PI * (-0.5 + i / numHorizontalLines) *(width/360.0)-(width*M_2_PI/360/2);
        const double depth1 = sin(latitude1);
        const double depthXY1 = cos(latitude1);
        
        glBegin(GL_QUAD_STRIP);
        for(double j = 0; j <= numVerticalLines; j++) {
            const double longitude = 2 * M_PI * (j-1.0)/numVerticalLines *(height/360.0) -4*(height*M_2_PI/360);
            const double x = cos(longitude);
            const double y = sin(longitude);
            
            glTexCoord2d(i/(numHorizontalLines+1.0), j/numVerticalLines);
            glNormal3f(x * depthXY0, y * depthXY0, depth0);
            glVertex3f(x * depthXY0, y * depthXY0, depth0);
            
            glTexCoord2d((i+1.0)/(numHorizontalLines+1.0), j/numVerticalLines);
            glNormal3f(x * depthXY1, y * depthXY1, depth1);
            glVertex3f(x * depthXY1, y * depthXY1, depth1);
        }
        glEnd();
    }
    glPopMatrix();
    
    glPopMatrix();
}
void renderSphericalMouse(double r, double numHorizontalLines, double numVerticalLines, double width, double height, double x, double y, double sizeX, double sizeY) {
    glPushMatrix();
    glRotated(90, 0, 1, 0);
    
    glPushMatrix();
    for(double i = 0; i <= numHorizontalLines; i++) {
        const double latitude0 = M_PI * (-0.5 + (i-1.0)/numHorizontalLines) *(width/360.0)-(width*M_2_PI/360/2);
        const double depth0  = sin(latitude0);
        const double depthXY0 =  cos(latitude0);
        
        const double latitude1 = M_PI * (-0.5 + i / numHorizontalLines) *(width/360.0)-(width*M_2_PI/360/2);
        const double depth1 = sin(latitude1);
        const double depthXY1 = cos(latitude1);
        
        glBegin(GL_QUAD_STRIP);
        for(double j = 0; j <= numVerticalLines; j++) {
            const double longitude = 2 * M_PI * (j-1.0)/numVerticalLines *(height/360.0) -4*(height*M_2_PI/360);
            const double x = cos(longitude);
            const double y = sin(longitude);
            
            glTexCoord2d(i/(numHorizontalLines+1.0), j/numVerticalLines);
            glNormal3f(x * depthXY0, y * depthXY0, depth0);
            glVertex3f(x * depthXY0, y * depthXY0, depth0);
            
            glTexCoord2d((i+1.0)/(numHorizontalLines+1.0), j/numVerticalLines);
            glNormal3f(x * depthXY1, y * depthXY1, depth1);
            glVertex3f(x * depthXY1, y * depthXY1, depth1);
        }
        glEnd();
    }
    glPopMatrix();
    
    glPopMatrix();
}

void renderCylindricalDisplay(double r, double numHorizontalLines, double numVerticalLines, double width, double height) {
    glPushMatrix();
    //    glRotated(90, 1, 0, 0);
    
    glPushMatrix();
    for(double i = 0; i <= numHorizontalLines; i++) {
        const double latitude0 = 2*M_PI * (-0.5 + (i-1.0)/numHorizontalLines) *(width/360.0);
        const double depth0  = sin(latitude0);
        const double depthXY0 =  cos(latitude0);
        
        const double latitude1 = 2*M_PI * (-0.5 + i / numHorizontalLines) *(width/360.0);
        const double depth1 = sin(latitude1);
        const double depthXY1 = cos(latitude1);
        
        glBegin(GL_QUAD_STRIP);
        for(double j = 0; j <= numVerticalLines; j++) {
            const double longitude = 2 * M_PI * (j-1.0)/numVerticalLines *(height/360.0);
            const double x = cos(longitude);
            const double y = sin(longitude);
            
            glTexCoord2d(1-i/(numHorizontalLines+1.0), j/numVerticalLines);
            glNormal3f(x * depthXY0, depthXY0, y * depthXY0);
            glVertex3f(depth0*r, (j/numVerticalLines)*2*r-r, depthXY0*r);
            
            glTexCoord2d(1-(i+1.0)/(numHorizontalLines+1.0), j/numVerticalLines);
            glNormal3f(x * depthXY1, y * depthXY1, depthXY1);
            glVertex3f(depth1*r, (j/numVerticalLines)*2*r-r,depthXY1*r);
        }
        glEnd();
    }
    glPopMatrix();
    
    glPopMatrix();
}

SimpleWorldRendererPlugin::SimpleWorldRendererPlugin() {
}
SimpleWorldRendererPlugin::~SimpleWorldRendererPlugin() {
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
    _skybox[0] = loadTexture("/resources/humus-skybox/negz.jpg");
    _skybox[1] = loadTexture("/resources/humus-skybox/posx.jpg");
    _skybox[2] = loadTexture("/resources/humus-skybox/posz.jpg");
    _skybox[3] = loadTexture("/resources/humus-skybox/negx.jpg");
    _skybox[4] = loadTexture("/resources/humus-skybox/posy.jpg");
    _skybox[5] = loadTexture("/resources/humus-skybox/negy.jpg");
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

void SimpleWorldRendererPlugin::renderIbexDisplayFlat(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M)
{
    checkForErrors();
//    std::cerr << "start IbexDisplayFlat" << std::endl;
    
    static GLuint vaoIbexDisplayFlat = 0;
    static const GLfloat IbexDisplayFlatScale = 10;
    
    static GLuint IbexDisplayFlatUniformLocations[5] = { 0, 0, 0, 0, 0};
    static GLuint IbexDisplayFlatAttribLocations[3] = { 0, 0, 0 };
    
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
        
        standardShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/standard.v.glsl", "/resources/shaders/standard.f.glsl");
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
        glEnableVertexAttribArray(IbexDisplayFlatAttribLocations[1]);
        glVertexAttribPointer(IbexDisplayFlatAttribLocations[1], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, (GLvoid*) (sizeof(GLfloat) * 3));
        glEnableVertexAttribArray(IbexDisplayFlatAttribLocations[2]);
        glVertexAttribPointer(IbexDisplayFlatAttribLocations[2], 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, (GLvoid*) (sizeof(GLfloat) * 6));
        
        
        glGenBuffers(1, &vboIbexDisplayFlatIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIbexDisplayFlatIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IbexDisplayFlatIndices), IbexDisplayFlatIndices, GL_STATIC_DRAW);
    }
    
    glUseProgram(standardShaderProgram.shader.program);
    glUniformMatrix4fv(IbexDisplayFlatUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(IbexDisplayFlatUniformLocations[1], 1, GL_FALSE, &V[0][0]);
    glUniformMatrix4fv(IbexDisplayFlatUniformLocations[2], 1, GL_FALSE, &M[0][0]);
    glUniformMatrix4fv(IbexDisplayFlatUniformLocations[4], 1, GL_FALSE, &(M*V)[0][0]);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, desktopTexture);
    glUniform1i(IbexDisplayFlatUniformLocations[3], 0);
    
    glBindVertexArray(vaoIbexDisplayFlat);
    glDrawElements(GL_TRIANGLES, sizeof(IbexDisplayFlatIndices)/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glUseProgram(0);
    
    if(!checkForErrors()) {
        std::cerr << "IbexDisplayFlat failed, exiting" << std::endl;
        exit(0);
    }
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
    const bool useCubemap = false;
    
    checkForErrors();
//    std::cerr << "start skybox" << std::endl;
    
    static GLuint vaoSkybox = 0;
    static const GLfloat skyboxScale = 1000;
    
    static GLuint SkyBoxUniformLocations[3] = {0, 0, 0};
    static GLuint SkyBoxAttribLocations[2] = {0, 0};
    
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
        0, 1, 2,
        1, 2, 3,
        
        4, 5, 6,
        5, 6, 7,
        
        8, 9, 10,
        9, 10, 11,
        
        12, 13, 14,
        13, 14, 15,
        
        16, 17, 18,
        17, 18, 19,
        
        20, 21, 22,
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
        0, 1, 2,
        0, 2, 3,
        
        3, 2, 6,
        3, 6, 7,
        
        7, 6, 5,
        7, 5, 4,
        
        4, 5, 1,
        4, 1, 0,
        
        0, 3, 7,
        0, 7, 4,
        
        1, 2, 6,
        1, 6, 5
    };
    static GLuint vboSkyboxIndices = 0;
    
    static bool first = true;
    if(first) {
        first = false;
        
        loadSkybox();
        
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
            "/resources/humus-skybox/posx.jpg",
            "/resources/humus-skybox/negx.jpg",
            "/resources/humus-skybox/posy.jpg",
            "/resources/humus-skybox/negy.jpg",
            "/resources/humus-skybox/posz.jpg",
            "/resources/humus-skybox/negz.jpg"
        };
        _skycube = loadCubemapTextures(paths);
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
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glBindVertexArray(0);
    }
    
    glUseProgram(0);
    
    if(!checkForErrors()) {
        std::cerr << "skybox failed, exiting" << std::endl;
        exit(0);
    }
//    std::cerr << "Done skybox" << std::endl;
}

void SimpleWorldRendererPlugin::renderGround(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M)
{
    checkForErrors();
//    std::cerr << "Loading ground texture" << std::endl;
#ifdef _WIN32
    static const GLuint groundTexture = loadTexture("\\resources\\humus-skybox\\negy.jpg");
    //        orientation = getRiftOrientation();
#else
#ifdef __APPLE__
    static const GLuint groundTexture = loadTexture("/resources/humus-skybox/negy.jpg");
#else
    static float sizeX = 64;
    static float sizeY = 64;
    static const GLuint groundTexture = glmLoadTexture("./resources/humus-skybox/negy.jpg", GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE, &sizeX, &sizeY);
#endif
    // gluInvertMatrix(get_orientation(), orientation);
#endif
    
    checkForErrors();
//    std::cerr << "start Ground" << std::endl;
    
    static GLuint vaoGround = 0;
    static const GLfloat GroundScale = 1000;
    
    static GLuint GroundUniformLocations[5] = { 0, 0, 0, 0, 0};
    static GLuint GroundAttribLocations[3] = { 0, 0, 0 };
    
    static GLfloat GroundVertices[] = {
        -1.0,  -10.0, -1.0, 0, 1, 0, 0, 0,
        1.0, -10.0, -1.0, 0, 1, 0, 1, 0,
        1.0, -10.0, 1.0, 0, 1, 0, 1, 1,
        -1.0, -10.0, 1.0, 0, 1, 0, 0, 1
    };
    static GLuint vboGroundVertices = 0;
    
    static GLushort GroundIndices[] = {
        0, 1, 2,
        0, 2, 3
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
    
    glUseProgram(groundShaderProgram.shader.program);
    glUniformMatrix4fv(GroundUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(GroundUniformLocations[1], 1, GL_FALSE, &V[0][0]);
    glUniformMatrix4fv(GroundUniformLocations[2], 1, GL_FALSE, &M[0][0]);
    glUniformMatrix4fv(GroundUniformLocations[4], 1, GL_FALSE, &(M*V)[0][0]);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, groundTexture);
    glUniform1i(GroundUniformLocations[3], 0);
    
    glBindVertexArray(vaoGround);
    glDrawElements(GL_TRIANGLES, sizeof(GroundIndices)/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glUseProgram(0);
    
    if(!checkForErrors()) {
        std::cerr << "Ground failed, exiting" << std::endl;
        exit(0);
    }
//    std::cerr << "Done Ground" << std::endl;
}

void SimpleWorldRendererPlugin::init() {
}

void SimpleWorldRendererPlugin::step(const Desktop3DLocation &loc, double timeDiff_) {
    if (USE_FBO) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbos[0]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
		if (!checkForErrors()) {
			std::cerr << "GL ISSUE -- SimpleWorldRendererPlugin::step" << std::endl;
			//exit(EXIT_FAILURE);
		}
    }
    
#ifdef _WIN32
    static const GLuint groundTexture = loadTexture("\\resources\\humus-skybox\\negy.jpg");
    //        orientation = getRiftOrientation();
#else
#ifdef __APPLE__
    static const GLuint groundTexture = loadTexture("/resources/humus-skybox/negy.jpg");
#else
    static float sizeX = 64;
    static float sizeY = 64;
    static const GLuint groundTexture = glmLoadTexture("./resources/humus-skybox/negy.jpg", GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE, &sizeX, &sizeY);
#endif
    // gluInvertMatrix(get_orientation(), orientation);
#endif
    
    for (int i2 = 0; i2 < 2; ++i2) {
        const bool left = (i2 == 0);
        const OVR::Util::Render::StereoEyeParams &stereo = (left) ? leftEye : rightEye;
        glViewport(stereo.VP.x*renderScale,stereo.VP.y*renderScale,stereo.VP.w*renderScale,stereo.VP.h*renderScale);
        glEnable(GL_SCISSOR_TEST);
        glScissor(stereo.VP.x*renderScale,stereo.VP.y*renderScale,stereo.VP.w*renderScale,stereo.VP.h*renderScale);
        
        if (!USE_FBO && i2 > 0) {
            break;
        }
        
        //            // Setup the frustum for the left eye
        //            glMatrixMode(GL_PROJECTION);
        //            glLoadMatrixf((const float *)stereo.Projection.Transposed().M);
        //
        //            static double *orientation;
        //            if(i2 == 0) orientation = getRiftOrientation();
        //            glMultMatrixd(orientation);
        //
        //
        //            glMatrixMode(GL_MODELVIEW);
        //            glLoadMatrixf((const float*)stereo.ViewAdjust.M);
        
        glm::mat4 modelView;
        glm::mat4 view;
        glm::mat4 model;
        glm::mat4 proj;
        copyMatrix(view,stereo.ViewAdjust.Transposed().M);
        copyMatrix(proj, (getRiftOrientationNative()*stereo.Projection.Transposed()).M);
        
        
        glm::mat4 playerRotation(glm::rotate(glm::mat4(1.0f), (float)loc.getXRotation(), glm::vec3(1, 0, 0)));
        playerRotation = glm::rotate(playerRotation, (float)loc.getYRotation(), glm::vec3(0, 1, 0));
        glm::mat4 playerCamera(glm::translate(playerRotation, glm::vec3((float)loc.getXPosition(), (float)loc.getYPosition(), (float)loc.getZPosition())));
        
        glDepthMask(GL_FALSE);
        renderSkybox(view*playerRotation, proj);
        
        glDepthMask(GL_TRUE);
        view *= playerCamera;
        
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -10.0f));
        renderIbexDisplayFlat(proj*view*model, view, model);
        
        if(showGround) {
            model = glm::mat4();
            renderGround(proj*view*model, view, model);
        }
        
        if(false) {
            //double orientation[16];
            //gluInvertMatrix(get_orientation(), orientation);
            
            //            glPushMatrix();
            {
                //glMultMatrixd(orientation);
                glRotated(loc.getXRotation(), 1, 0, 0);
                glRotated(loc.getYRotation(), 0, 1, 0);
                //  glTranslated(0, -1.5, 0);
                glTranslated(0, -0.5, 0);
                
                //                renderSkybox();
                glColor4f(1,1,1,1);
                
                glPushMatrix();
                {
                    glTranslated(loc.getXPosition(),
                                 loc.getYPosition(),
                                 loc.getZPosition());
                    
                    if(showGround) {
                        static const int gridSize = 25;
                        static const int textureRepeat = 2*gridSize;
                        
                        glBindTexture(GL_TEXTURE_2D, groundTexture);
                        glBegin(GL_TRIANGLE_STRIP);
                        glTexCoord2d(0, 0);
                        glVertex3f(-gridSize, 0, -gridSize);
                        
                        glTexCoord2d(textureRepeat, 0);
                        glVertex3f(gridSize, 0, -gridSize);
                        
                        glTexCoord2d(0, textureRepeat);
                        glVertex3f(-gridSize, 0, gridSize);
                        
                        glTexCoord2d(textureRepeat, textureRepeat);
                        glVertex3f(gridSize, 0, gridSize);
                        glEnd();
                        //glBindTexture(GL_TEXTURE_2D, 0);
                    }
                    
                    if (renderToTexture) {
                        double ySize = ((double)height / (double)width) / 2.0;
                        glTranslated(0, 0.5, 0);
                        const double monitorOriginZ = -0.5;
                        glBindTexture(GL_TEXTURE_2D, desktopTexture);
                        glColor4f(1,1,1,1);
                        
                        switch(displayShape) {
                            case SphericalDisplay:
                                renderSphericalDisplay(0.5, 100, 100,180,135);//25, 25, 180, 90);
                                break;
                            case CylindricalDisplay:
                                renderCylindricalDisplay(0.5, 100,100,180,180);//25, 25, 180, 90);
                                break;
                            case FlatDisplay:
                            default:
                                glBegin(GL_TRIANGLE_STRIP);
                                glTexCoord2d(0, 0);
                                glVertex3f(-0.5, -ySize, monitorOriginZ);
                                
                                glTexCoord2d(1, 0);
                                glVertex3f(0.5, -ySize, monitorOriginZ);
                                
                                glTexCoord2d(0, 1);
                                glVertex3f(-0.5, ySize, monitorOriginZ);
                                
                                glTexCoord2d(1, 1);
                                glVertex3f(0.5, ySize, monitorOriginZ);
                                glEnd();
                        }
                        
                        ySize = videoHeight/videoWidth/2.0;
                        glBindTexture(GL_TEXTURE_2D, videoTexture[i2]);
                        glColor4f(1,1,1,1);
                        glBegin(GL_TRIANGLE_STRIP);
                        glTexCoord2d(0, 1);
                        glVertex3f(1.5-0.5, -ySize, monitorOriginZ);
                        
                        glTexCoord2d(1, 1);
                        glVertex3f(1.5+0.5, -ySize, monitorOriginZ);
                        
                        glTexCoord2d(0, 0);
                        glVertex3f(1.5-0.5, ySize, monitorOriginZ);
                        
                        glTexCoord2d(1, 0);
                        glVertex3f(1.5+0.5, ySize, monitorOriginZ);
                        glEnd();
                        
                        
                        glEnable(GL_BLEND);
                        
                        if(mouseBlendAlternate) {
                            glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
                        } else {
                            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                        }
                        
                        
                        ySize = ((double)height / (double)width) / 2.0;
                        glBindTexture( GL_TEXTURE_2D, cursor );
                        glBegin(GL_TRIANGLE_STRIP);
                        {
                            //					const double cursorSize = 32.0; // was 20.0, not sure which is best on each OS
                            double x0, x1, y0, y1, z;
                            x0 = -0.5+(cursorPosX/physicalWidth);
                            x1 = -0.5+((cursorPosX+cursorSize)/physicalWidth);
                            y0 = -ySize+ySize*2*(cursorPosY+0.)/physicalHeight;
                            y1 = -ySize+ySize*2*(cursorPosY-cursorSize)/physicalHeight;
                            
#ifdef WIN32
                            z = monitorOriginZ+0.00001;
#else
                            z = monitorOriginZ+0.0000001;
#endif
                            
                            glTexCoord2d(0, 0);
                            glVertex3f(  x0,  y0, z);
                            
                            glTexCoord2d(1, 0);
                            glVertex3f( x1, y0, z);
                            
                            glTexCoord2d(0, -1);
                            glVertex3f(  x0, y1 ,z);
                            
                            glTexCoord2d(1, -1);
                            glVertex3f(  x1,  y1, z);
                            
                            glTexCoord2d(1, -1);
                            glVertex3f( x1, y1, z);
                            
                            glTexCoord2d(0, -1);
                            glVertex3f(  x0, y1,z);
                        }
                        glEnd();
                        
                        if(mouseBlendAlternate) { // restore mode
                            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                        }
                    } else {
                        renderDesktopToTexture();
                    }
                }
                glPopMatrix();
                
                if(showDialog) {
                    window.render();
                }
            }
            //            glPopMatrix();
        }
    }
    glDisable(GL_SCISSOR_TEST);
    glFlush();
    
    glViewport(0,0, windowWidth, windowHeight);
    if (USE_FBO) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if(!CACHED_SHADER) {
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        
        //        if (ortho) {
        //            glMatrixMode(GL_PROJECTION);
        //            glOrtho(0, 1, 0, 1, -1, 1);
        //            glMatrixMode(GL_MODELVIEW);
        //            glLoadIdentity();
        //        }
        
        if(barrelDistort) {
            if(CACHED_SHADER) {
                if(lensParametersChanged) {
                    render_distortion_lenses();
                }
                
                render_both_frames(textures[0]);
            } else {
                render_distorted_frame(true, textures[0]);
                render_distorted_frame(false, textures[0]);
            }
        } else {
            if(!SBS) {
                glViewport(0,0, windowWidth, windowHeight);
                glBindTexture(GL_TEXTURE_2D, textures[0]);
                
                glColor4f(1, 1, 1, 1);
                glBegin(GL_TRIANGLE_STRIP);
                glTexCoord2d(0, bottom);
                glVertex3f(0, 0, 0);
                
                glTexCoord2d(0.5, bottom);
                glVertex3f(1, 0, 0);
                
                glTexCoord2d(0, top);
                glVertex3f(0, 1, 0);
                
                glTexCoord2d(0.5, top);
                glVertex3f(1, 1, 0);
                glEnd();
                
            } else {
                for (int i = 0; i < 2; ++i) {
                    if (ortho) {
                        double originX = (i == 0) ? 0 : 0.5;
                        glBindTexture(GL_TEXTURE_2D, textures[i]);
                        
                        glColor4f(1, 1, 1, 1);
                        glBegin(GL_TRIANGLE_STRIP);
                        glTexCoord2d(0, bottom);
                        glVertex3f(originX, 0, 0);
                        
                        glTexCoord2d(1, bottom);
                        glVertex3f(originX + 0.5, 0, 0);
                        
                        glTexCoord2d(0, top);
                        glVertex3f(originX, 1, 0);
                        
                        glTexCoord2d(1, top);
                        glVertex3f(originX + 0.5, 1, 0);
                        glEnd();
                    } else {
                        glBindTexture(GL_TEXTURE_2D, textures[i]);
                        glPushMatrix();
                        glTranslated((i < 1) ? -0.98 : 0, -0.5, -2.4);
                        glColor4f(1, 1, 1, 1);
                        glBegin(GL_TRIANGLE_STRIP);
                        glTexCoord2d(0, bottom);
                        glVertex3f(0, 0, 0);
                        
                        glTexCoord2d(1, bottom);
                        glVertex3f(1,0,0);
                        
                        glTexCoord2d(0, top);
                        glVertex3f(0,1,0);
                        
                        glTexCoord2d(1, top);
                        glVertex3f(1,1,0);
                        glEnd();
                        glPopMatrix();
                    }
                }
            }
        }
    }
}

Window SimpleWorldRendererPlugin::getWindowID() {
    return ::window;
}

bool SimpleWorldRendererPlugin::needsSwapBuffers() {
    return doubleBuffered;
}
