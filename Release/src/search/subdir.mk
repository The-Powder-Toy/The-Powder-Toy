################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/search/Save.cpp \
../src/search/SearchController.cpp \
../src/search/SearchModel.cpp \
../src/search/SearchView.cpp \
../src/search/Thumbnail.cpp 

OBJS += \
./src/search/Save.o \
./src/search/SearchController.o \
./src/search/SearchModel.o \
./src/search/SearchView.o \
./src/search/Thumbnail.o 

CPP_DEPS += \
./src/search/Save.d \
./src/search/SearchController.d \
./src/search/SearchModel.d \
./src/search/SearchView.d \
./src/search/Thumbnail.d 


# Each subdirectory must supply rules for building sources it contributes
src/search/%.o: ../src/search/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


