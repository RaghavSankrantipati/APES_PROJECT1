################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/apds9301.c \
../src/main.c \
../src/message.c \
../src/tmp102.c \
../src/usrled.c 

OBJS += \
./src/apds9301.o \
./src/main.o \
./src/message.o \
./src/tmp102.o \
./src/usrled.o 

C_DEPS += \
./src/apds9301.d \
./src/main.d \
./src/message.d \
./src/tmp102.d \
./src/usrled.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	arm-linux-gnueabihf-gcc -I/usr/arm-linux-gnueabihf/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


