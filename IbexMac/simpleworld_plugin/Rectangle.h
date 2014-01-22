//
//  Rectangle.h
//  IbexMac
//
//  Created by Hesham Wahba on 1/22/14.
//  Copyright (c) 2014 Hesham Wahba. All rights reserved.
//

#ifndef __IbexMac__Rectangle__
#define __IbexMac__Rectangle__

#include "../GLSLShaderProgram.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace Ibex {

class Rectangle {
public:
    Rectangle(float scale_=10.0);
    
    // leftRightBoth: 0 is both, 1 is left, 2 is right
    void render(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP, GLuint texture_, const bool &randomize, const int &leftRightBoth);
    
private:
    GLuint vaoIbexDisplayFlat;
    const GLfloat IbexDisplayFlatScale;
    
    GLint IbexDisplayFlatUniformLocations[7];
    GLint IbexDisplayFlatAttribLocations[3];
    
    GLuint vboIbexDisplayFlatVertices;
    GLuint vboIbexDisplayFlatIndices;
};

}
#endif /* defined(__IbexMac__Rectangle__) */
