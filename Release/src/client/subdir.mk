################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/client/Client.cpp \
../src/client/HTTP.cpp \
../src/client/MD5.cpp 

OBJS += \
./src/client/Client.o \
./src/client/HTTP.o \
./src/client/MD5.o 

CPP_DEPS += \
./src/client/Client.d \
./src/client/HTTP.d \
./src/client/MD5.d 


# Each subdirectory must supply rules for building sources it contributes
src/client/%.o: ../src/client/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


