################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/game/GameController.cpp \
../src/game/GameModel.cpp \
../src/game/GameView.cpp \
../src/game/ToolButton.cpp 

OBJS += \
./src/game/GameController.o \
./src/game/GameModel.o \
./src/game/GameView.o \
./src/game/ToolButton.o 

CPP_DEPS += \
./src/game/GameController.d \
./src/game/GameModel.d \
./src/game/GameView.d \
./src/game/ToolButton.d 


# Each subdirectory must supply rules for building sources it contributes
src/game/%.o: ../src/game/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


