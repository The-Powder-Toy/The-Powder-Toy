################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/preview/PreviewController.cpp \
../src/preview/PreviewModel.cpp \
../src/preview/PreviewView.cpp 

OBJS += \
./src/preview/PreviewController.o \
./src/preview/PreviewModel.o \
./src/preview/PreviewView.o 

CPP_DEPS += \
./src/preview/PreviewController.d \
./src/preview/PreviewModel.d \
./src/preview/PreviewView.d 


# Each subdirectory must supply rules for building sources it contributes
src/preview/%.o: ../src/preview/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


