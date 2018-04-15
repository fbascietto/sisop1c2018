

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#ifndef BIBLIO_SOCKETS_H_
#define BIBLIO_SOCKETS_H_


#define TAMBUFFER 1024
#define MAX_CLIENTES 10

int enviarMensajeArchivo(int socketDestino, char* mensaje);
char *recibirMensajeArchivo(int socketDestino);
int escuchar(int puerto);
int conectarseA(char *ip, int puerto);
int aceptarConexion(int socketEscucha);
int esperarConexionesSocket(fd_set *master, int socketEscucha);
int enviarInt(int socketDestino, int num);
int recibirInt(int socketDestino, int* i);

#endif /* BIBLIO_SOCKETS_H_ */
