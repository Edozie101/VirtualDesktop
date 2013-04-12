/*
 * ibex.h
 *
 *  Created on: Sep 22, 2012
 *      Author: Hesham Wahba
 */

#ifndef IBEX_H_
#define IBEX_H_

//#include "opengl_setup_x11.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <time.h>

#include "OVR.h"
using namespace OVR;

extern Ptr<DeviceManager>	pManager;
extern Ptr<HMDDevice>		pHMD;
extern Ptr<SensorDevice>	pSensor;
extern SensorFusion		FusionResult;
extern HMDInfo			Info;
extern bool			InfoLoaded;

typedef unsigned int Display;
typedef bool Bool;

extern int dpy;
extern int display;
extern int window;

extern GLfloat physicalWidth,physicalHeight;
extern GLfloat windowWidth, windowHeight;
extern GLfloat width,height;
extern GLfloat textureWidth,textureHeight;

extern double IOD;

extern GLfloat top, bottom;
extern GLuint desktopTexture;
extern bool mouseBlendAlternate;
extern GLuint cursor;

extern GLfloat cursorPosX;
extern GLfloat cursorPosY;

extern GLuint fbos[2];
extern GLuint textures[2];

extern bool resetPosition;
extern bool showGround;
extern bool barrelDistort;
extern bool ortho;
extern bool renderToTexture;
extern bool USE_FBO;
extern bool OGRE3D;
extern bool IRRLICHT;
extern bool SBS;

extern bool controlDesktop;

extern double relativeMouseX;
extern double relativeMouseY;
extern double walkForward;
extern double strafeRight;

// ---------------------------------------------------------------------------
// Class:    Desktop3DLocation
// Design:   Belongs to ?
// Purpose:  Owns information on the global position of desktop in 3D
// Updated:  Sep 10, 2012
// ---------------------------------------------------------------------------
class Desktop3DLocation
{
public:
  // Prevent unforeseen copying
  explicit Desktop3DLocation()
    : WALK_SPEED(0.2),//05),
      m_xRotation(0.0), m_yRotation(0.0), m_zRotation(0.0),
      m_xPosition(0.0), m_yPosition(0.0), m_zPosition(0.0) {};
  // Class not intended for inheritence
  ~Desktop3DLocation() {};

  // Resets the state
  inline void resetState()
  {
    m_xRotation = 0.0;
    m_yRotation = 0.0;
    m_zRotation = 0.0;
    m_xPosition = 0.0;
    m_yPosition = 0.0;
    m_zPosition = 0.0;
  }

  // Get methods for position and rotation
  inline double getXRotation() const { return m_xRotation; };
  inline double getYRotation() const { return m_yRotation; };
  inline double getZRotation() const { return m_zRotation; };

  inline double getXPosition() const { return m_xPosition; };
  inline double getYPosition() const { return m_yPosition; };
  inline double getZPosition() const { return m_zPosition; };

  // Set methods for position and rotation
  inline void setXRotation(const double xRotation) { m_xRotation = xRotation; }
  inline void setYRotation(const double yRotation) { m_yRotation = yRotation; }
  inline void setZRotation(const double zRotation) { m_zRotation = zRotation; }

  inline void setXPosition(const double xPosition) { m_xPosition = xPosition; }
  inline void setYPosition(const double yPosition) { m_yPosition = yPosition; }
  inline void setZPosition(const double zPosition) { m_zPosition = zPosition; }

  // Modify location of the desktop in 3D
  inline void walk(double forward, double right, double seconds)
  {
    const double walkSpeedSec = WALK_SPEED * seconds;

    m_xPosition -= sin(m_yRotation / 90.0 * M_PI_2) * walkSpeedSec * forward;
    m_zPosition += cos(m_yRotation / 90.0 * M_PI_2) * walkSpeedSec * forward;

    m_xPosition -= cos(-m_yRotation / 90.0 * M_PI_2) * walkSpeedSec * right;
    m_zPosition += sin(-m_yRotation / 90.0 * M_PI_2) * walkSpeedSec * right;
  }

  void setWalkSpeed(double WALK_SPEED_) {
    WALK_SPEED = WALK_SPEED_;
  }

private:
  // Prevent compiler from generating copy semantics by default
  Desktop3DLocation(const Desktop3DLocation& loc);
  Desktop3DLocation& operator=(const Desktop3DLocation& loc);

  // Local constants
  double WALK_SPEED;

  // State of the class: location and orientation in 3D
  double m_xRotation;
  double m_yRotation;
  double m_zRotation;
  double m_xPosition;
  double m_yPosition;
  double m_zPosition;
};

void getCursorTexture();

bool didInitOpenGL();

void renderSkybox();
void renderDesktopToTexture();

void resizeGL(unsigned int width, unsigned int height);

class Ibex {
public:
    // Instance of the class that tracks position/orientation of desktop
    Desktop3DLocation desktop3DLocation;

#ifndef _WIN32
    struct timespec ts_start;
#endif
    
    Ibex(int argc, char ** argv);
    void render(double timeDiff);
};

#endif /* IBEX_H_ */
