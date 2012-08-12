################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/login/LoginController.cpp \
../src/login/LoginModel.cpp \
../src/login/LoginView.cpp 

OBJS += \
./src/login/LoginController.o \
./src/login/LoginModel.o \
./src/login/LoginView.o 

CPP_DEPS += \
./src/login/LoginController.d \
./src/login/LoginModel.d \
./src/login/LoginView.d 


# Each subdirectory must supply rules for building sources it contributes
src/login/%.o: ../src/login/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


