/*
 * SimpleWorldRendererPlugin.h
 *
 *  Created on: Sep 25, 2012
 *      Author: Hesham Wahba
 */

#ifndef SIMPLEWORLDRENDERERPLUGIN_H_
#define SIMPLEWORLDRENDERERPLUGIN_H_

#include "../RendererPlugin.h"
#include "../ibex.h"

#include "../GLSLShaderProgram.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "../terrain/Terrain.h"

#include "Model.h"

extern GLSLShaderProgram skyboxShaderProgram;
extern GLSLShaderProgram groundShaderProgram;
extern GLSLShaderProgram standardShaderProgram;
extern GLSLShaderProgram shadowProgram;
extern GLSLShaderProgram waterShaderProgram;

extern GLint ShadowUniformLocations[1];
extern GLint ShadowAttribLocations[3];

extern glm::vec3 lightInvDir;

class SimpleWorldRendererPlugin : public RendererPlugin {
public:
    SimpleWorldRendererPlugin();
    ~SimpleWorldRendererPlugin();
    
    GLfloat getPlayerHeightAtPosition(GLfloat x, GLfloat z);
    
    void loadSkybox();
    void renderSkybox(const glm::mat4 &modelView, const glm::mat4 &proj);
    void renderGround(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP, const double &time);
    void renderVideoDisplayFlat(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, const glm::mat4 &depthMVP);
    void renderIbexDisplayFlat(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP);
    void renderWater(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP, const double &time);
    
    void init();
    void reset();
    void step(const Desktop3DLocation &loc_, double timeDiff_, const double &time_);
    bool needsSwapBuffers();
    
    void bringUpIbexDisplay();
    
    Window getWindowID();
private:
    void render(const glm::mat4 &proj_,  const glm::mat4 &orthoProj, const glm::mat4 &view_, const glm::mat4 &playerCamera_, const glm::mat4 &playerRotation_, const glm::vec3 &playerPosition_, bool shadowPass, const glm::mat4 &depthBiasMVP, const double &time);
    
    GLuint _skybox[6];
    GLuint _skycube;
    
    Terrain terrain;
    
    bool _bringUpIbexDisplay;
    glm::mat4 ibexDisplayModelTransform;
    
    Model treeModel;
};

#endif /* SIMPLEWORLDRENDERERPLUGIN_H_ */
