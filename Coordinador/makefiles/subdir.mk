################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../algoritmos.c \
../coordinador.c \
../funcionesCoordinador.c 

OBJS += \
./algoritmos.o \
./coordinador.o \
./funcionesCoordinador.o 

C_DEPS += \
./algoritmos.d \
./coordinador.d \
./funcionesCoordinador.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/Biblioteca" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


