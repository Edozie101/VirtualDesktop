# VirtualDesktop

VirtualDesktop is a simple 3D Virtual Reality (VR) world that will allow you to
have a number of virtual monitors that contain your current actual desktop or the
ability to watch movies. The idea is for you to have a pretty virtual world to
work in regardless of where you are so that you can be flying on a plane, for example,
but still be flanked by multiple large high resolution monitors and still be effective
rather than be limited by a tiny laptop screen. In addition, through the eventual use
of plugins you can change your environment to suit your needs so you can work in a giant
castle or out in a serene field.

It currently supports the Rift on Windows and OS X and when the SDK is updated Linux
as well. If you have a Razer Hydra you can use it to look and walk around.

Networking and multi-user support, physics engine and plugin support are all coming.
Movie playing has gotten intial support on the Mac as well.

## Status

This project has been dormant for more than a year, but I've decided to take on the task
of reviving it.

I am fascinated by the idea of virtual workspaces. I think they have the potential to be
hugely transformative. I am a total novice when it comes to most of what this project
entails (3D graphics, x-platform development, `C++`, &c.), but I am very motivated to
obtain those skills. As a result, things are going to move very slowly for a while,
but they should pick up speed as I become familiar with what I'm doing.

## Launch
Default simple renderer: `./ibex`
Fancy Irrlicht renderer with Quake 3 level: `./ibex -i`
Ogre3D renderer with simple terrain and water: `./ibex -o`
Disable Side-by-Side rendering: `-m` (for mono)  

## Controls

| Action                                   | Key Combination or Motion                                                |
|------------------------------------------|--------------------------------------------------------------------------|
| Toggle Control Desktop/Move Around World | `CTRL+SHIFT+Y`, `Shift+F1` or `CTRL+SHIFT+G` (Mac), `CTRL+SHIFT+G` (Win) |
| Look                                     | Move your mouse or turn your head |
| Move Forward                             | `W` |
| Move Backwards                           | `S` |
| Walk Sideways Left (Strafe Left)         | `A` |
| Walk Sideways Right (Strafe Right)       | `D` |
| Run                                      | `SHIFT` |
| Jump                                     | `SPACE` |
| Show FPS and Info Dialog                 | `/` (Mac and Windows Only) |
| Show Help                                | `H` |
| Toggle Ground Layer                      | `G` |
| Toggle Barrel Lens Distort               | `B` |
| Toggle Display Shape (Spherical/Flat on Windows and Mac only) | `Q` |
| Adjust IPD for Rift stereo viewing       | `-`/`+` keys |
| Razer Hydra                              | Calibrate by pressing the `LT` then `RT` buttons, after that use the thumbsticks |

If using the iPhone controller app (source included) you need to connect to the
correct IP for your computer in the iPhone app.  When you hit "Broadcast" the
orientation of the phone becomes the base orientation and you will start looking
around from there.

## Requirements
### OS X
XCode 5.0.2+, OSX 10.9 (Mavericks), Intel HD3000 or better (OpenGL 3.3+) - Build and run
from XCode, check the project dependencies to find out where the bullet3d physics library
and sixense SDK need to go to build properly.  OculusSDK needs to be placed at the same
level as your checked-out git repo for ibex and it must be version 0.2.5 or later.  You'll
need to rebuild the libovr.a library with C++ RTTI and Exceptions enabled or else it won't
link properly with ibex.

The [Oculus SDK] and [sixense SDK] must be installed at the same level as the ibex repository
directory:

```
* Parent Directory:
|
|->ibex
    |...
    |->IbexMac
    |->IbexWin
|
|->sixenseSDK_linux_OSX
|
|->OculusSDK
```

For math and video support you need to install the following:

```sh
brew install --build-bottle glm assimp freetype
brew install --build-bottle x264 xvid yasm faac
brew install --build-bottle ffmpeg --use-clang
brew tap homebrew/science
brew install --build-bottle opencv --env=std
brew install --build-bottle glfw3

# complete list of packages I have installed, probably don't need them all
brew install --build-bottle assimp freetype glm jpeg boost gmp lame libtool pkg-config xz cmake lua yasm faac glfw3 imagemagick libpng mad opencv
```
bullet physics how to build:

```sh
cd ~/Downloads/bullet-2.81-rev2613
mkdir bullet-build
cd bullet-build
cmake .. -G "Unix Makefiles" -DINSTALL_LIBS=ON #-DBUILD_SHARED_LIBS=ON                                                                                                                   
make -j4
sudo make install
```

Updated cmake install method for regular build:
```sh
# at the same level as the ibex checkout directory
mkdir build
cmake -DCMAKE_INSTALL_PREFIX:PATH=`pwd` ../ibex/IbexMac
# or with Ogre
# cmake -DCMAKE_INSTALL_PREFIX:PATH=`pwd` ../ibex/IbexMac -DOGRE=1
make install
```

or

```sh
# at the same level as the ibex checkout directory
mkdir build
cmake -DCMAKE_INSTALL_PREFIX:PATH=`pwd` ../ibex/IbexMac -G Xcode
# or with Ogre
# cmake -DCMAKE_INSTALL_PREFIX:PATH=`pwd` ../ibex/IbexMac -DOGRE=1 -G Xcode
# switch to the ibex_run target and hit Command+B
```

### Windows
Visual Studio 2012 - Build and run from Visual Studio, Rift SDK must be at same level as ibex folder and named: OculusSDK.

You also need to have ffmpeg installed where VS can find it.  I recommend the builds from ([http://ffmpeg.zeranoe.com/builds](http://ffmpeg.zeranoe.com/builds/ "http://ffmpeg.zeranoe.com/builds/")).  You should download the 32-bit shared and dev builds and extract them at the same level as ibex.

The [Oculus SDK] and [sixense SDK] must also be extracted at the same level as your repository.

Your directory structure should look like:
```
* Parent Directory:
|
|->ibex
   |...
   |->IbexMac
   |->IbexWin
|->ffmpeg
   |
   |->lib
|->ffmpeg-shared
   |
   |->bin
|->OculusSDK
```

### Linux
Setting up my Ubuntu 13.04 install:

```sh
sudo apt-get install mesa-utils eclipse-cdt freeglut3-dev glew-utils libglew1.6-dev libxrender-dev libxrender1-dbg libxrender1 libxdamage-dev libxdamage1-dbg libxcomposite1-dbg libxcomposite-dev libxxf86vm-dev libxxf86vm1-dbg libxcb1-dev libxrandr-dev libxft-dev libx11-xcb-dev libxi-dev libxinerama-dev cmake
sudo apt-get install libtinyxml-dev
sudo apt-get install nvidia-current-updates-dev
sudo apt-get install libirrlicht-dev
sudo apt-get install libjpeg62-dev libboost1.53-all-dev
sudo apt-get install libudev-dev libopenal-dev

sudo add-apt-repository ppa:jon-severinsson/ffmpeg
sudo apt-get install libavcodec-dev libavdevice-dev libavfilter-dev libavformat-dev libswscale-dev libavutil-dev libx264-dev libswresample-dev # ffmpeg 0.10.7
sudo apt-get install libopencv-dev
```

* Blender
* Blender2Ogre to convert models to Ogre meshes: [Blender2Ogre]
 
To run:  

```sh
cd ~/workspace  
mkdir build  
cd build  
cmake ../ibex  
make install  
./ibex  

```

[Oculus SDK]: https://developer.oculusvr.com
[sixense SDK]: http://sixense.com/developers
[Blender2Ogre]: https://code.google.com/p/blender2ogre/
