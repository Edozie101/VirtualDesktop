/*
 * irrlicht_plugin.h
 *
 *  Created on: Sep 22, 2012
 *      Author: parallels
 */

#ifndef IRRLICHT_PLUGIN_H_
#define IRRLICHT_PLUGIN_H_

#include "../RendererPlugin.h"
#include "../ibex.h"

int irrlicht_plugin();
void irrlicht_step(const Desktop3DLocation& loc);
void irrlicht_run_loop();
void irrlicht_move(int forward, int right, bool jump, double relativeMouseX, double relativeMouseY);

class IrrlichtRendererPlugin : public RendererPlugin {
public:
  IrrlichtRendererPlugin();
  ~IrrlichtRendererPlugin();

  Window getWindowID();
  void move(int forward_, int right_, bool jump_, double relativeMouseX_, double relativeMouseY_);
  void step(const Desktop3DLocation &loc, double timeDiff_);
};

#endif /* IRRLICHT_PLUGIN_H_ */
