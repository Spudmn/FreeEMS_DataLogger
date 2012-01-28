################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/SD_Main_Task.c \
../src/ff.c \
../src/main.c \
../src/virtualfs.c 

OBJS += \
./src/SD_Main_Task.o \
./src/ff.o \
./src/main.o \
./src/virtualfs.o 

C_DEPS += \
./src/SD_Main_Task.d \
./src/ff.d \
./src/main.d \
./src/virtualfs.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


