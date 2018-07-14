################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../biblio_sockets.c \
../biblioteca.c \
../file_cleaner.c 

OBJS += \
./biblio_sockets.o \
./biblioteca.o \
./file_cleaner.o 

C_DEPS += \
./biblio_sockets.d \
./biblioteca.d \
./file_cleaner.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


