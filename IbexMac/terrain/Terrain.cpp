//
//  Terrain.cpp
//  IbexMac
//
//  Created by Hesham Wahba on 11/22/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#include "Terrain.h"

#include "../simpleworld_plugin/SimpleWorldRendererPlugin.h"
#include "../simpleworld_plugin/ShadowBufferRenderer.h"
#include "../ibex_mac_utils.h"
#include "../ibex.h"

Terrain::Terrain() : data(0),vertices(0),indices(0),scaleX(30),scaleY(.2),scaleZ(30),translateX(-10),translateY(0),translateZ(-10) {
}

void Terrain::loadTerrain(unsigned char *data, int width, int height) {
    if(indices != 0) delete [] indices;
    numIndices = (width-1)*(height-1)*6;
    indices = new GLuint[numIndices];
    
    if(vertices != 0) delete [] vertices;
    numVertices = width*height*8;
    vertices = new GLfloat[numVertices];
    
    int index = 0;
    for(int y = 0; y < height; ++y) {
        for(int x = 0; x < width; ++x) {
            vertices[index] = (x-width/2)*scaleX+translateX;
            vertices[index+1] = data[y*width+x]*scaleY-20.0+translateY;
            vertices[index+2] = (y-height/2)*scaleZ+translateZ;
            
            vertices[index+6] = x;
            vertices[index+7] = y;
            
//            std::cerr << vertices[index] << ", " << vertices[index+1] << "," << vertices[index+2] << std::endl;
            index += 8;
        }
    }
    
    index = 0;
    for(int y = 0; y < height; ++y) {
        for(int x = 0; x < width; ++x) {
            glm::vec3 normal;
            if(y == height-1) {
            } else if(x == width-1) {
                
            } else {
                glm::vec3 v1(0.f,float(data[y*width+x]),0.f);
                glm::vec3 v2 = glm::vec3(1,data[y*width+x+1],0)-v1;
                glm::vec3 v3 = glm::vec3(0,data[(y+1)*width+x],1)-v1;
                normal = glm::normalize(glm::cross(v2,v3));
            }
            vertices[index+3] = normal.x;
            vertices[index+4] = normal.y;
            vertices[index+5] = normal.z;
            
            index += 8;
        }
    }
    
    index = 0;
    for(int y = 0; y < height-1; ++y) {
        for(int x = 0; x < width-1; ++x) {
            indices[index] = y*width+x;
            indices[index+1] = (y+1)*width+x;
            indices[index+2] = y*width+x+1;
            
            indices[index+3] = y*width+x+1;
            indices[index+4] = (y+1)*width+x;
            indices[index+5] = (y+1)*width+x+1;
            
//            std::cerr << "i: " << indices[index] << ", " << indices[index+1] << ", " << indices[index+2] << std::endl;
//            std::cerr << "v: " << vertices[indices[index]*8] << ", " << vertices[indices[index]*8+1] << ", " << vertices[indices[index]*8+2] << std::endl;
//            std::cerr << "v: " << vertices[indices[index+1]*8] << ", " << vertices[indices[index+1]*8+1] << ", " << vertices[indices[index+1]*8+2] << std::endl;
//            std::cerr << "v: " << vertices[indices[index+2]*8] << ", " << vertices[indices[index+2]*8+1] << ", " << vertices[indices[index+2]*8+2] << std::endl;
//            
//            std::cerr << "i: " << indices[index+3] << ", " << indices[index+4] << ", " << indices[index+5] << std::endl;
//            std::cerr << "v: " << vertices[indices[index+3]*8] << ", " << vertices[indices[index+3]*8+1] << ", " << vertices[indices[index+3]*8+2] << std::endl;
//            std::cerr << "v: " << vertices[indices[index+4]*8] << ", " << vertices[indices[index+4]*8+1] << ", " << vertices[indices[index+4]*8+2] << std::endl;
//            std::cerr << "v: " << vertices[indices[index+5]*8] << ", " << vertices[indices[index+5]*8+1] << ", " << vertices[indices[index+5]*8+2] << std::endl;
            
            index += 6;
        }
    }
    
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*numVertices, vertices, GL_STATIC_DRAW);
    checkForErrors();
    
    glGenBuffers(1, &vboGroundIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboGroundIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*numIndices, indices, GL_STATIC_DRAW);
    checkForErrors();
    
    glEnableVertexAttribArray(GroundAttribLocations[0]);
    glVertexAttribPointer(GroundAttribLocations[0], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, 0);
    glEnableVertexAttribArray(GroundAttribLocations[1]);
    glVertexAttribPointer(GroundAttribLocations[1], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, (GLvoid*) (sizeof(GLfloat) * 3));
    glEnableVertexAttribArray(GroundAttribLocations[2]);
    glVertexAttribPointer(GroundAttribLocations[2], 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, (GLvoid*) (sizeof(GLfloat) * 6));
    checkForErrors();
    
    std::cerr << "gen terrain done" << std::endl;
}

void Terrain::loadHeightmap(const char *filename, int width, int height)
{
    if(data != 0) delete [] data;
    
    FILE * file;
    
    char path[2048];
    strcpy(path, mResourcePath);
    strcat(path, filename);
    
    file = fopen( path, "rb");
    if(file == NULL) { std::cerr << "Couldn't open file: " << path << std::endl; exit(1); }
    else { std::cerr << "Loading terrain file: " << path << std::endl; }
    
    data = (unsigned char *)malloc( width * height);
    
    fread( data, width * height, 1, file );
    fclose( file );
    
    loadTerrain(data, width, height);
    
    delete [] data;
    data = 0;
}

void Terrain::renderGround(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP) {
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
    
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
}