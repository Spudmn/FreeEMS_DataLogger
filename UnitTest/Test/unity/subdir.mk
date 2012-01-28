################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Test/unity/cmock.c \
../Test/unity/unity.c \
../Test/unity/unity_fixture.c 

OBJS += \
./Test/unity/cmock.o \
./Test/unity/unity.o \
./Test/unity/unity_fixture.o 

C_DEPS += \
./Test/unity/cmock.d \
./Test/unity/unity.d \
./Test/unity/unity_fixture.d 


# Each subdirectory must supply rules for building sources it contributes
Test/unity/%.o: ../Test/unity/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


