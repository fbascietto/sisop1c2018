#include <stdio.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <parsi/parser.h>
#include "../Recursos/protocolo.h"
#include "../Biblioteca/biblio_sockets.h"

#ifndef ESI_H_
#define ESI_H_

t_log_level T;
t_log_level I;
t_log_level E;
t_log* logT;
t_log* logI;
t_log* logE;

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
