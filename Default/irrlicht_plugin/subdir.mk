################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../irrlicht_plugin/irrlicht_plugin.cpp 

OBJS += \
./irrlicht_plugin/irrlicht_plugin.o 

CPP_DEPS += \
./irrlicht_plugin/irrlicht_plugin.d 


# Each subdirectory must supply rules for building sources it contributes
irrlicht_plugin/%.o: ../irrlicht_plugin/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DHAVE_LIBJPEG=1 -I/usr/include/nvidia-current-updates/GL/ -I/usr/include/X11 -I/usr/include/OGRE -I/usr/include/OIS -I/usr/include/irrlicht -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


