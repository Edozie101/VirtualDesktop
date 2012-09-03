################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../glm/glm.c \
../glm/glm_util.c \
../glm/glmimg.c \
../glm/glmimg_devil.c \
../glm/glmimg_jpg.c \
../glm/glmimg_png.c \
../glm/glmimg_sdl.c \
../glm/glmimg_sim.c 

OBJS += \
./glm/glm.o \
./glm/glm_util.o \
./glm/glmimg.o \
./glm/glmimg_devil.o \
./glm/glmimg_jpg.o \
./glm/glmimg_png.o \
./glm/glmimg_sdl.o \
./glm/glmimg_sim.o 

C_DEPS += \
./glm/glm.d \
./glm/glm_util.d \
./glm/glmimg.d \
./glm/glmimg_devil.d \
./glm/glmimg_jpg.d \
./glm/glmimg_png.d \
./glm/glmimg_sdl.d \
./glm/glmimg_sim.d 


# Each subdirectory must supply rules for building sources it contributes
glm/%.o: ../glm/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DHAVE_LIBJPEG=1 -I/usr/include/nvidia-current-updates/GL/ -I/usr/include/X11 -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


