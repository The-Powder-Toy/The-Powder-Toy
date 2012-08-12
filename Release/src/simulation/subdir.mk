################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/simulation/Air.cpp \
../src/simulation/Gravity.cpp \
../src/simulation/SaveLoader.cpp \
../src/simulation/SaveRenderer.cpp \
../src/simulation/Simulation.cpp \
../src/simulation/SimulationData.cpp \
../src/simulation/Solids.cpp 

OBJS += \
./src/simulation/Air.o \
./src/simulation/Gravity.o \
./src/simulation/SaveLoader.o \
./src/simulation/SaveRenderer.o \
./src/simulation/Simulation.o \
./src/simulation/SimulationData.o \
./src/simulation/Solids.o 

CPP_DEPS += \
./src/simulation/Air.d \
./src/simulation/Gravity.d \
./src/simulation/SaveLoader.d \
./src/simulation/SaveRenderer.d \
./src/simulation/Simulation.d \
./src/simulation/SimulationData.d \
./src/simulation/Solids.d 


# Each subdirectory must supply rules for building sources it contributes
src/simulation/%.o: ../src/simulation/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


