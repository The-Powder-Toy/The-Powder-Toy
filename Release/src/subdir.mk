################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Console.cpp \
../src/Graphics.cpp \
../src/Misc.cpp \
../src/PowderToy.cpp \
../src/Renderer.cpp 

OBJS += \
./src/Console.o \
./src/Graphics.o \
./src/Misc.o \
./src/PowderToy.o \
./src/Renderer.o 

CPP_DEPS += \
./src/Console.d \
./src/Graphics.d \
./src/Misc.d \
./src/PowderToy.d \
./src/Renderer.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


