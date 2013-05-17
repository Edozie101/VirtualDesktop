**Ibex** 3D Virtual Reality Desktop/Compositing Manager for X11/Linux, Windows and Mac OS

# About
Ibex is a simple 3D Virtual Reality (VR) world that will allow you to have a number of virtual monitors that contain your current actual desktop.  The idea is for you to have a pretty virtual world to work in regardless of where you are
so that you can be flying on a plane, for example, but still be flanked by multiple large high resolution monitors and still be effective rather than be limited by a tiny laptop screen.

It currently supports the Rift on Windows and when the SDK is updated it will support it on all platforms.

# Home Page and Downloads

Home page and binaries [http://hwahba.com/ibex](http://hwahba.com/ibex)

### Launch
Default simple renderer: ./ibex  
Fancy Irrlicht renderer with Quake 3 level: ./ibex -i  
Ogre3D renderer with simple terrain and water: ./ibex -o  
Disable Side-by-Side rendering: -m (for mono)  

### Controls
Toggle Control Desktop/Move Around World: CTRL+SHIFT+Y, Shift+F1 or CTRL+SHIFT+G (Mac), CTRL+SHIFT+G (Win)  
Look: Move Mouse  
Move Forward: W  
Move Backwards: S  
Walk Sideways Left (Strafe Left): A/Q  
Walk Sideways Right (Strafe Right): D/E   
Jump: SPACE (Irrlicht renderer only)  
Adjust IPD for Rift stereo viewing: -/+ keys  

Toggle Barrel Lens Distort: B  
Toggle Ground Layer: G  

If using the iPhone controller app (source included) you need to connect to the
correct IP for your computer in the iPhone app.  When you hit "Broadcast" the
orientation of the phone becomes the base orientation and you will start looking
around from there.

## Requirements
### Mac
XCode 4.5, OSX 10.8 (Mountain Lion) - Build and run from XCode, check the project dependencies
to find out where the bullet3d physics library and sixense SDK need to go to build properly

### Windows
Visual Studio 2012 - Build and run from Visual Studio, Rift SDK must be at same level as ibex folder and named: OculusSDK

### Linux
Setting up my Ubuntu 12.10 install:

*  sudo apt-get install mesa-utils eclipse-cdt freeglut3-dev glew-utils libglew1.6-dev libxrender-dev libxrender1-dbg libxrender1 libxdamage-dev libxdamage1-dbg libxcomposite1-dbg libxcomposite-dev libxxf86vm-dev libxxf86vm1-dbg libxcb1-dev libxrandr-dev libxft-dev libx11-xcb-dev libxi-dev cmake  
sudo apt-get install libtinyxml-dev  
sudo apt-get install nvidia-current-updates-dev  
sudo apt-get install libirrlicht-dev  

* Blender
* Blender2Ogre to convert models to Ogre meshes: https://code.google.com/p/blender2ogre/
 
To run:  
cd ~/workspace  
mkdir build  
cd build  
cmake ../ibex  
make install  
./ibex  

----

Author: Hesham Wahba (http://hwahba.com)  
Copyright Hesham Wahba 2012,2013