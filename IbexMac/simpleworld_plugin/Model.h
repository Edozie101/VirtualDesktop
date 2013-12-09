//
//  Model.h
//  IbexMac
//
//  Created by Hesham Wahba on 11/28/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#ifndef __IbexMac__Model__
#define __IbexMac__Model__

#include "../opengl_helpers.h"
#include "../ibex_mac_utils.h"

#include "../GLSLShaderProgram.h"

#include <assimp/Importer.hpp>
#include <assimp/PostProcess.h>
#include <assimp/Scene.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <math.h>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <iostream>

#include <boost/filesystem.hpp>

class Model {
public:
    GLuint setupShaders();
    bool Import3DFromFile( const std::string& pFile);
    int LoadGLTextures(const aiScene* scene, const std::string &basePath);
    void genVAOsAndUniformBuffer(const aiScene *sc);
    void recursive_render(const aiScene *sc, const aiNode* nd,  const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP);
    void renderScene(GLuint program, const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP);
    
    
private:
    Assimp::Importer importer;
    const aiScene *scene;
    
    std::map<std::string,GLuint> textureIdMap;
    
    //// Can't send color down as a pointer to aiColor4D because AI colors are ABGR.
    //void Color4f(const aiColor4D *color)
    //{
    //	glColor4f(color->r, color->g, color->b, color->a);
    //}
    
    void set_float4(float f[4], float a, float b, float c, float d)
    {
        f[0] = a;
        f[1] = b;
        f[2] = c;
        f[3] = d;
    }
    
    void color4_to_float4(const aiColor4D *c, float f[4])
    {
        f[0] = c->r;
        f[1] = c->g;
        f[2] = c->b;
        f[3] = c->a;
    }
    
    
    
    // Information to render each assimp node
    struct MyMesh{
        
        GLuint vao;
        GLuint texIndex;
        GLuint uniformBlockIndex;
        int numFaces;
    };
    
    std::vector<struct MyMesh> myMeshes;
    
    // This is for a shader uniform block
    struct MyMaterial{
        
        float diffuse[4];
        float ambient[4];
        float specular[4];
        float emissive[4];
        float shininess;
        int texCount;
    };
    
    // Model Matrix (part of the OpenGL Model View Matrix)
    float modelMatrix[16];
    
    // For push and pop matrix
    std::vector<float *> matrixStack;
    
    // Vertex Attribute Locations
    GLuint vertexLoc=0, normalLoc=1, texCoordLoc=2;
    
    // Uniform Bindings Points
    GLuint matricesUniLoc = 1, materialUniLoc = 2;
    
    // The sampler uniform for textured models
    // we are assuming a single texture so this will
    //always be texture unit 0
    GLuint texUnit = 0;
    
    // Uniform Buffer for Matrices
    // this buffer will contain 3 matrices: projection, view and model
    // each matrix is a float array with 16 components
    GLuint matricesUniBuffer;
#define MatricesUniBufferSize sizeof(float) * 16 * 3
#define ProjMatrixOffset 0
#define ViewMatrixOffset sizeof(float) * 16
#define ModelMatrixOffset sizeof(float) * 16 * 2
#define MatrixSize sizeof(float) * 16
    
    
    GLSLShaderProgram modelShaderProgram;
    GLint ModelUniformLocations[6] = { 0, 0, 0, 0, 0, 0};
    GLint ModelAttribLocations[3] = { 0, 0, 0 };
};

#endif /* defined(__IbexMac__Model__) */
