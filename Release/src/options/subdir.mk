################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/options/OptionsController.cpp \
../src/options/OptionsModel.cpp \
../src/options/OptionsView.cpp 

OBJS += \
./src/options/OptionsController.o \
./src/options/OptionsModel.o \
./src/options/OptionsView.o 

CPP_DEPS += \
./src/options/OptionsController.d \
./src/options/OptionsModel.d \
./src/options/OptionsView.d 


# Each subdirectory must supply rules for building sources it contributes
src/options/%.o: ../src/options/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


