################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/tags/TagsController.cpp \
../src/tags/TagsModel.cpp \
../src/tags/TagsView.cpp 

OBJS += \
./src/tags/TagsController.o \
./src/tags/TagsModel.o \
./src/tags/TagsView.o 

CPP_DEPS += \
./src/tags/TagsController.d \
./src/tags/TagsModel.d \
./src/tags/TagsView.d 


# Each subdirectory must supply rules for building sources it contributes
src/tags/%.o: ../src/tags/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


