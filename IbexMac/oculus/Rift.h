/*
 * x11.h
 *
 *  Created on: Jul 27, 2013
 *      Author: Hesham Wahba
 */

#ifndef Rift_H_
#define Rift_H_

#include "OVR.h"

extern OVR::Ptr<OVR::DeviceManager> pManager;
extern OVR::Ptr<OVR::HMDDevice> pHMD;
extern OVR::Ptr<OVR::SensorDevice> pSensor;
extern OVR::SensorFusion *pFusionResult;
extern OVR::HMDInfo Info;
extern bool InfoLoaded;
extern bool riftConnected;
extern float renderScale;
extern bool renderScaleChanged;

extern OVR::Util::Render::StereoEyeParams  leftEye;
extern OVR::Util::Render::StereoEyeParams  rightEye;

extern int riftX;
extern int riftY;
extern int riftResolutionX;
extern int riftResolutionY;

void initRift();
void cleanUpRift();

double *getRiftOrientation();
const OVR::Matrix4f getRiftOrientationNative();

void setRenderScale(float renderScale_);
void toggleRenderScale();

#endif /* Rift_H_ */
