################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../ibex.cpp 

OBJ_SRCS += \
../lcd_monitor.obj 

OBJS += \
./ibex.o 

CPP_DEPS += \
./ibex.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DHAVE_LIBJPEG=1 -I/usr/include/nvidia-current-updates/GL/ -I/usr/include/X11 -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


