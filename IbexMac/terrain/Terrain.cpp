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

#include <glm/gtc/noise.hpp>

Terrain::Terrain() : data(0),vertices(0),indices(0),scaleX(30),scaleY(.2),scaleZ(30),translateX(-10),translateY(0),translateZ(-10) {
}

float Terrain::getNoiseHeight(const float &x_, const float &z_) const {
    glm::vec2 v((x_-translateX)/scaleX, (z_-translateZ)/scaleZ);
    
    return ((glm::simplex(v*f1)
             +glm::simplex(v*f2)*0.2
             +glm::simplex(v*f3)*0.1))*(maxHeight-minHeight) + minHeight;
}

void Terrain::generateNoiseTerrain(int width, int height,
                                   float scaleX_, float scaleY_, float scaleZ_,
                                   float minHeight_, float maxHeight_,
                                   float f1_, float f2_, float f3_) {
    scaleX = scaleX_;
    scaleY = scaleY_;
    scaleZ = scaleZ_;
    
    translateX = 0;
    translateY = 0;
    translateZ = 0;
    
    f1 = f1_;
    f2 = f2_;
    f3 = f3_;
    
    minHeight = minHeight_;
    maxHeight = maxHeight_;
    
    float *heights = new float[width*height];
    for(int x = 0; x < width; ++x) {
        for(int z = 0; z < height; ++z) {
            float px = x-width/2;
            float pz = z-height/2;
            
            if(px == 0 && pz == 0) {
                std::cerr << "******** " << z << " " << x << std::endl;
            }
            heights[z*height+x] = getNoiseHeight(px, pz);
        }
    }
    
    loadTerrain(heights, width, height);
    delete [] heights;
}

glm::vec2 getUV(GLfloat *vertices, GLuint vertexBufferStep, int width, int height, int x, int y) {
    
    return glm::vec2(vertices[(y*width+x)*vertexBufferStep+6], vertices[(y*width+x)*vertexBufferStep+7]);
}
glm::vec3 getPos(GLfloat *vertices, GLuint vertexBufferStep, int width, int height, int x, int y) {
    
    return glm::vec3(vertices[(y*width+x)*vertexBufferStep+0], vertices[(y*width+x)*vertexBufferStep+1],vertices[(y*width+x)*vertexBufferStep+2]);
}

template <class T>
void Terrain::loadTerrain(T *data, int width, int height) {
    if(indices != 0) delete [] indices;
    numIndices = (width-1)*(height-1)*6;
    indices = new GLuint[numIndices];
    
    bool useNormalMappedTextures = true;
    
    GLuint vertexBufferStep = 8;
    if(useNormalMappedTextures) {
        vertexBufferStep = 14;
    }
    if(vertices != 0) delete [] vertices;
    numVertices = width*height*vertexBufferStep;
    vertices = new GLfloat[numVertices];
    
    int index = 0;
    for(int y = 0; y < height; ++y) {
        for(int x = 0; x < width; ++x) {
            if(x-width/2 == 0 && y-height/2 == 0) {
                std::cerr << "^^^^^^^^ " << data[y*width+x]*scaleY+translateY << std::endl;
            }
            
            index = (y*width+x)*vertexBufferStep;
            vertices[index] = (x-width/2)*scaleX+translateX;
            vertices[index+1] = data[y*width+x]*scaleY+translateY;
            vertices[index+2] = (y-height/2)*scaleZ+translateZ;
            
            vertices[index+6] = x*2;
            vertices[index+7] = y*2;
            
            //            std::cerr << vertices[index] << ", " << vertices[index+1] << "," << vertices[index+2] << std::endl;
        }
    }
    
    index = 0;
    for(int y = 0; y < height-1; ++y) {
        for(int x = 0; x < width-1; ++x) {
            glm::vec3 v0 = getPos(vertices, vertexBufferStep, width, height, x,y);
            glm::vec3 v1 = getPos(vertices, vertexBufferStep,  width, height, x+1,y);
            glm::vec3 v2 = getPos(vertices, vertexBufferStep,  width, height, x,y+1);
            
            glm::vec3 normal = glm::normalize(glm::cross(v1-v0,v2-v0));
            
            int i = (y*width+x) * vertexBufferStep;
            vertices[i+3] = normal.x;
            vertices[i+4] = normal.y;
            vertices[i+5] = normal.z;
            i = (y*width+x+1) * vertexBufferStep;
            vertices[i+3] = normal.x;
            vertices[i+4] = normal.y;
            vertices[i+5] = normal.z;
            i = ((y+1)*width+x) * vertexBufferStep;
            vertices[i+3] = normal.x;
            vertices[i+4] = normal.y;
            vertices[i+5] = normal.z;
        }
    }
    if(useNormalMappedTextures) {
        index = 0;
        for(int x = 0; x < width-1; ++x) {
            for(int y = 0; y < height-1; ++y) {
                // Shortcuts for vertices
                glm::vec3 v0 = getPos(vertices, vertexBufferStep, width, height, x,y);
                glm::vec3 v1 = getPos(vertices, vertexBufferStep,  width, height, x+1,y);
                glm::vec3 v2 = getPos(vertices, vertexBufferStep,  width, height, x,y+1);
                
                // Shortcuts for UVs
                glm::vec2 uv0 = getUV(vertices, vertexBufferStep,  width, height, x,y);
                glm::vec2 uv1 = getUV(vertices, vertexBufferStep,  width, height, x+1,y);
                glm::vec2 uv2 = getUV(vertices, vertexBufferStep,  width, height, x,y+1);
                
                // Edges of the triangle : postion delta
                glm::vec3 deltaPos1 = v1-v0;
                glm::vec3 deltaPos2 = v2-v0;
                
                // UV delta
                glm::vec2 deltaUV1 = uv1-uv0;
                glm::vec2 deltaUV2 = uv2-uv0;
                
                float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
                if(!isinf(r) && !isnan(r)) {
                    //                    std::cerr << r << std::endl;
                } else {
//                    for(int i2 = 0; i2 < 3; ++i2) {
//                        // tangents
//                        vertices[index+i2*vertexBufferStep+8] = 0;
//                        vertices[index+i2*vertexBufferStep+9] = 0;
//                        vertices[index+i2*vertexBufferStep+10] = 0;
//                        
//                        // bitangents
//                        vertices[index+i2*vertexBufferStep+11] = 0;
//                        vertices[index+i2*vertexBufferStep+12] = 0;
//                        vertices[index+i2*vertexBufferStep+13] = 0;
//                    }
                    continue;
                }
                glm::vec3 tangent = glm::normalize((deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r);
                glm::vec3 bitangent = glm::normalize((deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r);
                
                glm::vec3 normal = glm::normalize(glm::cross(v1-v0, v2-v0));
                // Gram-Schmidt orthogonalize
                tangent = glm::normalize(tangent - normal * glm::dot(normal, tangent));
                
                // Calculate handedness
                if (glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f){
                    tangent = tangent * -1.0f;
                }
                
                // use same value for all 3 vertices of triangle
                // tangents
                vertices[(y*width+x)*vertexBufferStep+8] = tangent.x;
                vertices[(y*width+x)*vertexBufferStep+9] = tangent.y;
                vertices[(y*width+x)*vertexBufferStep+10] = tangent.z;
                vertices[(y*width+x+1)*vertexBufferStep+8] = tangent.x;
                vertices[(y*width+x+1)*vertexBufferStep+9] = tangent.y;
                vertices[(y*width+x+1)*vertexBufferStep+10] = tangent.z;
                vertices[((y+1)*width+x)*vertexBufferStep+8] = tangent.x;
                vertices[((y+1)*width+x)*vertexBufferStep+9] = tangent.y;
                vertices[((y+1)*width+x)*vertexBufferStep+10] = tangent.z;
                
                // bitangents
                vertices[(y*width+x)*vertexBufferStep+11] = bitangent.x;
                vertices[(y*width+x)*vertexBufferStep+12] = bitangent.y;
                vertices[(y*width+x)*vertexBufferStep+13] = bitangent.z;
                vertices[(y*width+x+1)*vertexBufferStep+11] = bitangent.x;
                vertices[(y*width+x+1)*vertexBufferStep+12] = bitangent.y;
                vertices[(y*width+x+1)*vertexBufferStep+13] = bitangent.z;
                vertices[((y+1)*width+x)*vertexBufferStep+11] = bitangent.x;
                vertices[((y+1)*width+x)*vertexBufferStep+12] = bitangent.y;
                vertices[((y+1)*width+x)*vertexBufferStep+13] = bitangent.z;
                
                // go to next triangle
                index += vertexBufferStep*3;
            }
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
            //            std::cerr << "v: " << vertices[indices[index]*vertexBufferStep] << ", " << vertices[indices[index]*vertexBufferStep+1] << ", " << vertices[indices[index]*vertexBufferStep+2] << std::endl;
            //            std::cerr << "v: " << vertices[indices[index+1]*vertexBufferStep] << ", " << vertices[indices[index+1]*vertexBufferStep+1] << ", " << vertices[indices[index+1]*vertexBufferStep+2] << std::endl;
            //            std::cerr << "v: " << vertices[indices[index+2]*vertexBufferStep] << ", " << vertices[indices[index+2]*vertexBufferStep+1] << ", " << vertices[indices[index+2]*vertexBufferStep+2] << std::endl;
            //
            //            std::cerr << "i: " << indices[index+3] << ", " << indices[index+4] << ", " << indices[index+5] << std::endl;
            //            std::cerr << "v: " << vertices[indices[index+3]*vertexBufferStep] << ", " << vertices[indices[index+3]*vertexBufferStep+1] << ", " << vertices[indices[index+3]*vertexBufferStep+2] << std::endl;
            //            std::cerr << "v: " << vertices[indices[index+4]*vertexBufferStep] << ", " << vertices[indices[index+4]*vertexBufferStep+1] << ", " << vertices[indices[index+4]*vertexBufferStep+2] << std::endl;
            //            std::cerr << "v: " << vertices[indices[index+5]*vertexBufferStep] << ", " << vertices[indices[index+5]*vertexBufferStep+1] << ", " << vertices[indices[index+5]*vertexBufferStep+2] << std::endl;
            
            index += 6;
        }
    }
    
    if(useNormalMappedTextures) {
        groundShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/ground-normalmapped.v.glsl", "/resources/shaders/ground-normalmapped.f.glsl");
    } else {
        groundShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/standard.v.glsl", "/resources/shaders/ground.f.glsl");
    }
    glUseProgram(groundShaderProgram.shader.program);
    
    
    GroundUniformLocations[0] = glGetUniformLocation(groundShaderProgram.shader.program, "MVP");
    GroundUniformLocations[1] = glGetUniformLocation(groundShaderProgram.shader.program, "V");
    GroundUniformLocations[2] = glGetUniformLocation(groundShaderProgram.shader.program, "M");
    GroundUniformLocations[3] = glGetUniformLocation(groundShaderProgram.shader.program, "textureIn");
    GroundUniformLocations[4] = glGetUniformLocation(groundShaderProgram.shader.program, "MV");
    
    GroundUniformLocations[5] = glGetUniformLocation(groundShaderProgram.shader.program, "DepthBiasMVP");
    GroundUniformLocations[6] = glGetUniformLocation(groundShaderProgram.shader.program, "shadowTexture");
    
    GroundUniformLocations[7] = glGetUniformLocation(groundShaderProgram.shader.program, "textureIn2");
    GroundUniformLocations[8] = glGetUniformLocation(groundShaderProgram.shader.program, "textureIn3");
    GroundUniformLocations[9] = glGetUniformLocation(groundShaderProgram.shader.program, "textureIn4");
    
    GroundUniformLocations[10] = glGetUniformLocation(groundShaderProgram.shader.program, "time");
    GroundUniformLocations[11] = glGetUniformLocation(groundShaderProgram.shader.program, "LightPosition_worldspace");
    GroundUniformLocations[12] = glGetUniformLocation(groundShaderProgram.shader.program, "MV3x3");
    
    if(useNormalMappedTextures) {
        GroundAttribLocations[0] = glGetAttribLocation(groundShaderProgram.shader.program, "vertexPosition_modelspace");
        GroundAttribLocations[1] = glGetAttribLocation(groundShaderProgram.shader.program, "vertexNormal_modelspace");
        GroundAttribLocations[2] = glGetAttribLocation(groundShaderProgram.shader.program, "vertexUV");
        GroundAttribLocations[3] = glGetAttribLocation(groundShaderProgram.shader.program, "vertexTangent_modelspace");
        GroundAttribLocations[4] = glGetAttribLocation(groundShaderProgram.shader.program, "vertexBitangent_modelspace");
    } else {
        GroundAttribLocations[0] = glGetAttribLocation(groundShaderProgram.shader.program, "vertexPosition_modelspace");
        GroundAttribLocations[1] = glGetAttribLocation(groundShaderProgram.shader.program, "vertexNormal_modelspace");
        GroundAttribLocations[2] = glGetAttribLocation(groundShaderProgram.shader.program, "vertexUV");
    }
    
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
    
    if(useNormalMappedTextures) {
        glEnableVertexAttribArray(GroundAttribLocations[0]);
        glVertexAttribPointer(GroundAttribLocations[0], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*vertexBufferStep, 0);
        glEnableVertexAttribArray(GroundAttribLocations[1]);
        glVertexAttribPointer(GroundAttribLocations[1], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*vertexBufferStep, (GLvoid*) (sizeof(GLfloat) * 3));
        glEnableVertexAttribArray(GroundAttribLocations[2]);
        glVertexAttribPointer(GroundAttribLocations[2], 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*vertexBufferStep, (GLvoid*) (sizeof(GLfloat) * 6));
        glEnableVertexAttribArray(GroundAttribLocations[3]);
        glVertexAttribPointer(GroundAttribLocations[3], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*vertexBufferStep, (GLvoid*) (sizeof(GLfloat) * 8));
        glEnableVertexAttribArray(GroundAttribLocations[4]);
        glVertexAttribPointer(GroundAttribLocations[4], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*vertexBufferStep, (GLvoid*) (sizeof(GLfloat) * 11));
    } else {
        glEnableVertexAttribArray(GroundAttribLocations[0]);
        glVertexAttribPointer(GroundAttribLocations[0], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*vertexBufferStep, 0);
        glEnableVertexAttribArray(GroundAttribLocations[1]);
        glVertexAttribPointer(GroundAttribLocations[1], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*vertexBufferStep, (GLvoid*) (sizeof(GLfloat) * 3));
        glEnableVertexAttribArray(GroundAttribLocations[2]);
        glVertexAttribPointer(GroundAttribLocations[2], 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*vertexBufferStep, (GLvoid*) (sizeof(GLfloat) * 6));
    }
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

void Terrain::renderGround(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP, const double &time) {
    //    checkForErrors();
    //    std::cerr << "Loading ground texture" << std::endl;
#ifdef _WIN32
    static const GLuint groundTexture = loadTexture("\\resources\\humus-skybox\\negy.jpg");
    //        orientation = getRiftOrientation();
#else
#ifdef __APPLE__
    static const GLuint groundTexture = loadTexture("/resources/textures/grass1/grass1-diffuse.png");
    static const GLuint groundTexture1 = loadNormalTexture("/resources/textures/grass1/grass1-normal.png");
    static const GLuint groundTexture2 = loadTexture("/resources/textures/brown1/brown1-diffuse.png");
    static const GLuint groundTexture3 = loadNormalTexture("/resources/textures/brown1/brown1-normal.png");
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
        if(GroundUniformLocations[4] > -1) glUniformMatrix4fv(GroundUniformLocations[4], 1, GL_FALSE, &(V*M)[0][0]);
        
        if(GroundUniformLocations[5] > -1) {
            glUniformMatrix4fv(GroundUniformLocations[5], 1, GL_FALSE, &depthMVP[0][0]);
        }
        
        if(GroundUniformLocations[6] > -1) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, shadowMapDepthTextureId);
            glUniform1i(GroundUniformLocations[6], 1);
        }
        
        if(GroundUniformLocations[3] > -1) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, groundTexture);
            glUniform1i(GroundUniformLocations[3], 0);
        }
        if(GroundUniformLocations[7] > -1) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, groundTexture1);
            glUniform1i(GroundUniformLocations[7], 2);
        }
        
        if(GroundUniformLocations[8] > -1) {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, groundTexture2);
            glUniform1i(GroundUniformLocations[8], 3);
        }
        if(GroundUniformLocations[9] > -1) {
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, groundTexture3);
            glUniform1i(GroundUniformLocations[9], 4);
        }
        
        if(GroundUniformLocations[10] > -1) glUniform1f(GroundUniformLocations[10], time);
        if(GroundUniformLocations[11] > -1) glUniform3f(GroundUniformLocations[11], lightInvDir.x, lightInvDir.y, lightInvDir.z);
        
        glm::mat3 MV3x3 = glm::mat3(V*M);
        if(GroundUniformLocations[12] > -1) glUniformMatrix3fv(GroundUniformLocations[12], 1, GL_FALSE, &MV3x3[0][0]);
    }
    
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
}