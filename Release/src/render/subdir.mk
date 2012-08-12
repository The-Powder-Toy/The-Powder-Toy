################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/render/RenderController.cpp \
../src/render/RenderModel.cpp \
../src/render/RenderView.cpp 

OBJS += \
./src/render/RenderController.o \
./src/render/RenderModel.o \
./src/render/RenderView.o 

CPP_DEPS += \
./src/render/RenderController.d \
./src/render/RenderModel.d \
./src/render/RenderView.d 


# Each subdirectory must supply rules for building sources it contributes
src/render/%.o: ../src/render/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


