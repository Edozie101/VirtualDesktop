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

class SimpleWorldRendererPlugin : public RendererPlugin {
public:
  SimpleWorldRendererPlugin();
  ~SimpleWorldRendererPlugin();

  void loadSkybox();
  void renderSkybox();

  void init();
  void step(const Desktop3DLocation &loc_, double timeDiff_);
  bool needsSwapBuffers();

  Window getWindowID();
private:
  GLuint _skybox[6];
};

#endif /* SIMPLEWORLDRENDERERPLUGIN_H_ */
