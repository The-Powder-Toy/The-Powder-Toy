################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/dialogues/ConfirmPrompt.cpp \
../src/dialogues/ErrorMessage.cpp \
../src/dialogues/TextPrompt.cpp 

OBJS += \
./src/dialogues/ConfirmPrompt.o \
./src/dialogues/ErrorMessage.o \
./src/dialogues/TextPrompt.o 

CPP_DEPS += \
./src/dialogues/ConfirmPrompt.d \
./src/dialogues/ErrorMessage.d \
./src/dialogues/TextPrompt.d 


# Each subdirectory must supply rules for building sources it contributes
src/dialogues/%.o: ../src/dialogues/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


