################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/console/ConsoleController.cpp \
../src/console/ConsoleModel.cpp \
../src/console/ConsoleView.cpp 

OBJS += \
./src/console/ConsoleController.o \
./src/console/ConsoleModel.o \
./src/console/ConsoleView.o 

CPP_DEPS += \
./src/console/ConsoleController.d \
./src/console/ConsoleModel.d \
./src/console/ConsoleView.d 


# Each subdirectory must supply rules for building sources it contributes
src/console/%.o: ../src/console/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


