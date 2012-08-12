################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include src/tasks/subdir.mk
-include src/tags/subdir.mk
-include src/stamps/subdir.mk
-include src/ssave/subdir.mk
-include src/simulation/subdir.mk
-include src/search/subdir.mk
-include src/render/subdir.mk
-include src/preview/subdir.mk
-include src/options/subdir.mk
-include src/login/subdir.mk
-include src/interface/subdir.mk
-include src/game/subdir.mk
-include src/elements/subdir.mk
-include src/dialogues/subdir.mk
-include src/console/subdir.mk
-include src/client/subdir.mk
-include src/cat/subdir.mk
-include src/cajun/subdir.mk
-include src/subdir.mk
-include build/obj/powder/tasks/subdir.mk
-include build/obj/powder/tags/subdir.mk
-include build/obj/powder/stamps/subdir.mk
-include build/obj/powder/ssave/subdir.mk
-include build/obj/powder/simulation/subdir.mk
-include build/obj/powder/search/subdir.mk
-include build/obj/powder/render/subdir.mk
-include build/obj/powder/preview/subdir.mk
-include build/obj/powder/options/subdir.mk
-include build/obj/powder/login/subdir.mk
-include build/obj/powder/interface/subdir.mk
-include build/obj/powder/game/subdir.mk
-include build/obj/powder/elements/subdir.mk
-include build/obj/powder/dialogues/subdir.mk
-include build/obj/powder/console/subdir.mk
-include build/obj/powder/client/subdir.mk
-include build/obj/powder/cat/subdir.mk
-include build/obj/powder/cajun/subdir.mk
-include build/obj/powder/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: PowderToypp

# Tool invocations
PowderToypp: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++  -o"PowderToypp" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C++_DEPS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) PowderToypp
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
