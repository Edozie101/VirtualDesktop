//
//  Rectangle.cpp
//  IbexMac
//
//  Created by Hesham Wahba on 1/22/14.
//  Copyright (c) 2014 Hesham Wahba. All rights reserved.
//

#include "Rectangle.h"

#include "../GLSLShaderProgram.h"
#include "ShadowBufferRenderer.h"
#include "SimpleWorldRendererPlugin.h"

#include "GeometricUtils.h"

Ibex::Rectangle::Rectangle(float scale_) : vaoIbexDisplayFlat(0),
IbexDisplayFlatScale(scale_),
vboIbexDisplayFlatVertices(0),
vboIbexDisplayFlatIndices(0),
p0(-1.0f*IbexDisplayFlatScale, -1.0f*IbexDisplayFlatScale, 0.0f, 1.0f),
p1( 1.0f*IbexDisplayFlatScale, -1.0f*IbexDisplayFlatScale, 0.0f, 1.0f),
p2( 1.0f*IbexDisplayFlatScale,  1.0f*IbexDisplayFlatScale, 0.0f, 1.0f),
p3(-1.0f*IbexDisplayFlatScale,  1.0f*IbexDisplayFlatScale, 0.0f, 1.0f)
{
    memset(IbexDisplayFlatUniformLocations,0,sizeof(IbexDisplayFlatUniformLocations));
    memset(IbexDisplayFlatAttribLocations,0,sizeof(IbexDisplayFlatAttribLocations));
    
    GLfloat IbexDisplayFlatVertices[] = {
        -1.0, -1.0, 0.0, 0, 0, -1, 0, 0,
         1.0, -1.0, 0.0, 0, 0, -1, 1, 0,
         1.0,  1.0, 0.0, 0, 0, -1, 1, 1,
        -1.0,  1.0, 0.0, 0, 0, -1, 0, 1,
        
        // left
        -0.5, -1.0, 0.0, 0, 0, -1, 0,          0,
         0.5, -1.0, 0.0, 0, 0, -1, 0.49999999, 0,
         0.5,  1.0, 0.0, 0, 0, -1, 0.49999999, 1,
        -0.5,  1.0, 0.0, 0, 0, -1, 0,          1,
        
        // right
        -0.5, -1.0, 0.0, 0, 0, -1, 0.5, 0,
         0.5, -1.0, 0.0, 0, 0, -1, 1,   0,
         0.5,  1.0, 0.0, 0, 0, -1, 1,   1,
        -0.5,  1.0, 0.0, 0, 0, -1, 0.5, 1,
    };
    
    GLushort IbexDisplayFlatIndices[] = {
        0, 1, 2,
        0, 2, 3,
        
        // left
        0+4, 1+4, 2+4,
        0+4, 2+4, 3+4,
        
        // right
        0+8, 1+8, 2+8,
        0+8, 2+8, 3+8,
    };
    
    for(int i = 0; i < sizeof(IbexDisplayFlatVertices)/sizeof(GLfloat); ++i) {
        if(i%8 < 3)
            IbexDisplayFlatVertices[i] *= IbexDisplayFlatScale;
        if(i%8 == 1)
            IbexDisplayFlatVertices[i] *= height/width;
    }
    
    if(standardShaderProgram.shader.program == 0) standardShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/emissive.v.glsl", "/resources/shaders/emissive.f.glsl");
    glUseProgram(standardShaderProgram.shader.program);
    
    
    IbexDisplayFlatUniformLocations[0] = glGetUniformLocation(standardShaderProgram.shader.program, "MVP");
    IbexDisplayFlatUniformLocations[1] = glGetUniformLocation(standardShaderProgram.shader.program, "V");
    IbexDisplayFlatUniformLocations[2] = glGetUniformLocation(standardShaderProgram.shader.program, "M");
    IbexDisplayFlatUniformLocations[3] = glGetUniformLocation(standardShaderProgram.shader.program, "textureIn");
    IbexDisplayFlatUniformLocations[4] = glGetUniformLocation(standardShaderProgram.shader.program, "MV");
    IbexDisplayFlatUniformLocations[5] = glGetUniformLocation(standardShaderProgram.shader.program, "inFade");
    IbexDisplayFlatUniformLocations[6] = glGetUniformLocation(standardShaderProgram.shader.program, "offset");
    
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
    glEnableVertexAttribArray(IbexDisplayFlatAttribLocations[2]);
    glVertexAttribPointer(IbexDisplayFlatAttribLocations[2], 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, (GLvoid*) (sizeof(GLfloat) * 6));
    
    
    glGenBuffers(1, &vboIbexDisplayFlatIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIbexDisplayFlatIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IbexDisplayFlatIndices), IbexDisplayFlatIndices, GL_STATIC_DRAW);
}
void Ibex::Rectangle::render(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP, GLuint texture_, const bool &randomize, const int &leftRightBoth) const
{
    if(shadowPass) {
        glUseProgram(shadowProgram.shader.program);
        glUniformMatrix4fv(ShadowUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
    } else {
        glUseProgram(standardShaderProgram.shader.program);
        glUniformMatrix4fv(IbexDisplayFlatUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(IbexDisplayFlatUniformLocations[1], 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(IbexDisplayFlatUniformLocations[2], 1, GL_FALSE, &M[0][0]);
        glUniformMatrix4fv(IbexDisplayFlatUniformLocations[4], 1, GL_FALSE, &(V*M)[0][0]);
        
        if(IbexDisplayFlatUniformLocations[5] >= 0) glUniform1f(IbexDisplayFlatUniformLocations[5], 1.0);
        if(IbexDisplayFlatUniformLocations[6] >= 0) {
            if(randomize) {
                const float offsetU = float(rand()%1280)/1280.0f;
                const float offsetV = float(rand()%720)/720.0f;
                glUniform2f(IbexDisplayFlatUniformLocations[6], offsetU, offsetV);
            } else {
                glUniform2f(IbexDisplayFlatUniformLocations[6], 0,0);
            }
        }
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_);
        glUniform1i(IbexDisplayFlatUniformLocations[3], 0);
    }
    
    glBindVertexArray(vaoIbexDisplayFlat);
    switch(leftRightBoth) {
        case 0:
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            break;
        case 1:
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)(sizeof(GLushort)*6));
            break;
        case 2:
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)(sizeof(GLushort)*12));
            break;
    }
}

bool Ibex::Rectangle::lineIntersects(const glm::vec4 &l0, const glm::vec4 &l1, const glm::mat4 &V, const glm::mat4 &M) const {
    const glm::mat4 VM = V*M;
    const glm::vec4 p0 = VM*this->p0;
    const glm::vec4 p1 = VM*this->p1;
    const glm::vec4 p2 = VM*this->p2;
    const glm::vec4 p3 = VM*this->p3;
    
    Ray r;
    r.P0 = l0;
    r.P1 = l1;
    
    Triangle t1; t1.V0 = p0; t1.V1 = p1; t1.V2 = p2;
    Triangle t2; t2.V0 = p0; t2.V1 = p2; t2.V2 = p3;
    
    Point i;
    if(intersect3D_RayTriangle(r, t1, &i) > 0 || intersect3D_RayTriangle(r, t2, &i) > 0) {
//        std::cerr << "+++ " << p0.x << " " << p0.y << " " << p0.z << std::endl;
//        std::cerr << "+++ " << p1.x << " " << p1.y << " " << p1.z << std::endl;
//        std::cerr << "+++ " << p2.x << " " << p2.y << " " << p2.z << std::endl;
//        std::cerr << "+++ " << p3.x << " " << p3.y << " " << p3.z << std::endl;
//        
//        std::cerr << l0.x << " " << l0.y << " " << l0.z << std::endl;
//        std::cerr << l1.x << " " << l1.y << " " << l1.z << std::endl;
        return true;
    } else {    
//        std::cerr << "--- " << p0.x << " " << p0.y << " " << p0.z << std::endl;
//        std::cerr << "--- " << p1.x << " " << p1.y << " " << p1.z << std::endl;
//        std::cerr << "--- " << p2.x << " " << p2.y << " " << p2.z << std::endl;
//        std::cerr << "--- " << p3.x << " " << p3.y << " " << p3.z << std::endl;
//        
//        std::cerr << l0.x << " " << l0.y << " " << l0.z << std::endl;
//        std::cerr << l1.x << " " << l1.y << " " << l1.z << std::endl;
    }

    
    return false;
}
