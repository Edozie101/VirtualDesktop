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

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class SimpleWorldRendererPlugin : public RendererPlugin {
public:
    SimpleWorldRendererPlugin();
    ~SimpleWorldRendererPlugin();
    
    void loadSkybox();
    void renderSkybox(const glm::mat4 &modelView, const glm::mat4 &proj);
    void renderGround(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M);
    void renderIbexDisplayFlat(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M);
    
    void init();
    void step(const Desktop3DLocation &loc_, double timeDiff_);
    bool needsSwapBuffers();
    
    Window getWindowID();
private:
    GLuint _skybox[6];
    GLuint _skycube;
};

#endif /* SIMPLEWORLDRENDERERPLUGIN_H_ */
