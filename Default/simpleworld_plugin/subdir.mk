################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../simpleworld_plugin/SimpleWorldRendererPlugin.cpp 

OBJS += \
./simpleworld_plugin/SimpleWorldRendererPlugin.o 

CPP_DEPS += \
./simpleworld_plugin/SimpleWorldRendererPlugin.d 


# Each subdirectory must supply rules for building sources it contributes
simpleworld_plugin/%.o: ../simpleworld_plugin/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DHAVE_LIBJPEG=1 -I/usr/include/nvidia-current-updates/GL/ -I/usr/include/X11 -I/usr/include/OGRE -I/usr/include/OIS -I/usr/include/irrlicht -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


