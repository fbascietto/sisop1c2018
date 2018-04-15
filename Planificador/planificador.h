#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <readline/readline.h> // Para usar readline

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_


t_log_level LogL;
t_log* logPlan;


void configureLogger();
void iniciaConsola();
void exit_gracefully(int return_nr);


#endif
