################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/interface/Button.cpp \
../src/interface/Checkbox.cpp \
../src/interface/Component.cpp \
../src/interface/ControlFactory.cpp \
../src/interface/DropDown.cpp \
../src/interface/Engine.cpp \
../src/interface/Label.cpp \
../src/interface/Panel.cpp \
../src/interface/SaveButton.cpp \
../src/interface/Slider.cpp \
../src/interface/Spinner.cpp \
../src/interface/Textarea.cpp \
../src/interface/Textblock.cpp \
../src/interface/Textbox.cpp \
../src/interface/Window.cpp 

OBJS += \
./src/interface/Button.o \
./src/interface/Checkbox.o \
./src/interface/Component.o \
./src/interface/ControlFactory.o \
./src/interface/DropDown.o \
./src/interface/Engine.o \
./src/interface/Label.o \
./src/interface/Panel.o \
./src/interface/SaveButton.o \
./src/interface/Slider.o \
./src/interface/Spinner.o \
./src/interface/Textarea.o \
./src/interface/Textblock.o \
./src/interface/Textbox.o \
./src/interface/Window.o 

CPP_DEPS += \
./src/interface/Button.d \
./src/interface/Checkbox.d \
./src/interface/Component.d \
./src/interface/ControlFactory.d \
./src/interface/DropDown.d \
./src/interface/Engine.d \
./src/interface/Label.d \
./src/interface/Panel.d \
./src/interface/SaveButton.d \
./src/interface/Slider.d \
./src/interface/Spinner.d \
./src/interface/Textarea.d \
./src/interface/Textblock.d \
./src/interface/Textbox.d \
./src/interface/Window.d 


# Each subdirectory must supply rules for building sources it contributes
src/interface/%.o: ../src/interface/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


