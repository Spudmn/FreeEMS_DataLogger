################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Test/MissingTeeth-Crank-8minus1_Test.c \
../Test/MissingTeethTestRunner.c \
../Test/Unit_Test_Main.c 

OBJS += \
./Test/MissingTeeth-Crank-8minus1_Test.o \
./Test/MissingTeethTestRunner.o \
./Test/Unit_Test_Main.o 

C_DEPS += \
./Test/MissingTeeth-Crank-8minus1_Test.d \
./Test/MissingTeethTestRunner.d \
./Test/Unit_Test_Main.d 


# Each subdirectory must supply rules for building sources it contributes
Test/%.o: ../Test/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


