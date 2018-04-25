#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <sys/socket.h>

typedef struct {
	int socketEscucha;
	fd_set fdSocketEscucha;
} t_esperar_conexion;


#endif /* ESTRUCTURAS_H_ */
