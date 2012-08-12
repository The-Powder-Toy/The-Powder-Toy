################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/cajun/elements.cpp \
../src/cajun/reader.cpp \
../src/cajun/writer.cpp 

OBJS += \
./src/cajun/elements.o \
./src/cajun/reader.o \
./src/cajun/writer.o 

CPP_DEPS += \
./src/cajun/elements.d \
./src/cajun/reader.d \
./src/cajun/writer.d 


# Each subdirectory must supply rules for building sources it contributes
src/cajun/%.o: ../src/cajun/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


