################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ssave/SSaveController.cpp \
../src/ssave/SSaveModel.cpp \
../src/ssave/SSaveView.cpp 

OBJS += \
./src/ssave/SSaveController.o \
./src/ssave/SSaveModel.o \
./src/ssave/SSaveView.o 

CPP_DEPS += \
./src/ssave/SSaveController.d \
./src/ssave/SSaveModel.d \
./src/ssave/SSaveView.d 


# Each subdirectory must supply rules for building sources it contributes
src/ssave/%.o: ../src/ssave/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


