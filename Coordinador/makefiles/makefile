################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Coordinador

dependents:
	-cd /home/utnso/workspace/Biblioteca/Debug && $(MAKE) clean all

# Tool invocations
Coordinador: $(OBJS) $(USER_OBJS) /home/utnso/workspace/Biblioteca/Debug/libBiblioteca.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/workspace/Biblioteca/Debug" -L/home/utnso/github/tp-2018-1c-Destino-Rusia/Biblioteca/Debug -o "Coordinador" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) Coordinador
	-@echo ' '

.PHONY: all clean dependents
/home/utnso/workspace/Biblioteca/Debug/libBiblioteca.so:

-include ../makefile.targets
