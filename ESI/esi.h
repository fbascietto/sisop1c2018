#include <stdio.h>
#include <commons/config.h>
#include <commons/string.h>
#include "../Biblioteca/protocolo.h"

#ifndef ESI_H_
#define ESI_H_

char* ruta_script_ejecuciones;

char* coordinador_IP;
int coordinador_puerto;
int coordinador_socket;

char* planificador_IP;
int planificador_puerto;
int planificador_socket;


t_config* cargar_configuracion();
void leerScript(char*);


#endif /* ESI_H_ */
