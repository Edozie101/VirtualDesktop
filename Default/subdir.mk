################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../RendererPlugin.cpp \
../distortions.cpp \
../ibex.cpp \
../opengl_helpers.cpp \
../opengl_setup_x11.cpp \
../utils.cpp 

OBJ_SRCS += \
../lcd_monitor.obj 

OBJS += \
./RendererPlugin.o \
./distortions.o \
./ibex.o \
./opengl_helpers.o \
./opengl_setup_x11.o \
./utils.o 

CPP_DEPS += \
./RendererPlugin.d \
./distortions.d \
./ibex.d \
./opengl_helpers.d \
./opengl_setup_x11.d \
./utils.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DHAVE_LIBJPEG=1 -I/usr/include/nvidia-current-updates/GL/ -I/usr/include/X11 -I/usr/include/OGRE -I/usr/include/OIS -I/usr/include/irrlicht -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


