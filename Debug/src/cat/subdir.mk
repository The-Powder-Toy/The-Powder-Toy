################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/cat/CommandInterface.cpp \
../src/cat/LuaScriptInterface.cpp \
../src/cat/TPTSTypes.cpp \
../src/cat/TPTScriptInterface.cpp 

OBJS += \
./src/cat/CommandInterface.o \
./src/cat/LuaScriptInterface.o \
./src/cat/TPTSTypes.o \
./src/cat/TPTScriptInterface.o 

CPP_DEPS += \
./src/cat/CommandInterface.d \
./src/cat/LuaScriptInterface.d \
./src/cat/TPTSTypes.d \
./src/cat/TPTScriptInterface.d 


# Each subdirectory must supply rules for building sources it contributes
src/cat/%.o: ../src/cat/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


