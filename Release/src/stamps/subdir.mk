################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/stamps/StampsController.cpp \
../src/stamps/StampsModel.cpp \
../src/stamps/StampsView.cpp 

OBJS += \
./src/stamps/StampsController.o \
./src/stamps/StampsModel.o \
./src/stamps/StampsView.o 

CPP_DEPS += \
./src/stamps/StampsController.d \
./src/stamps/StampsModel.d \
./src/stamps/StampsView.d 


# Each subdirectory must supply rules for building sources it contributes
src/stamps/%.o: ../src/stamps/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


