################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../ogre3d_plugin/BaseApplication.cpp \
../ogre3d_plugin/DotSceneLoader.cpp \
../ogre3d_plugin/TutorialApplication.cpp 

OBJS += \
./ogre3d_plugin/BaseApplication.o \
./ogre3d_plugin/DotSceneLoader.o \
./ogre3d_plugin/TutorialApplication.o 

CPP_DEPS += \
./ogre3d_plugin/BaseApplication.d \
./ogre3d_plugin/DotSceneLoader.d \
./ogre3d_plugin/TutorialApplication.d 


# Each subdirectory must supply rules for building sources it contributes
ogre3d_plugin/%.o: ../ogre3d_plugin/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DHAVE_LIBJPEG=1 -I/usr/include/nvidia-current-updates/GL/ -I/usr/include/X11 -I/usr/include/OGRE -I/usr/include/OIS -I/usr/include/irrlicht -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


