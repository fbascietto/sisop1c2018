/*
 * funcionesInstancia.c
 *
 *  Created on: Apr 27, 2018
 *      Author: utnso
 */
#include "funcionesInstancia.h"


int  almacenarEntrada(char key[40], FILE* archivoDatos, int socketConn){

	t_entrada * entrada = malloc(sizeof(t_entrada));

	strcpy(entrada->key,key);
	entrada->entry = list_size(entradas);
	entrada->size = escribirEntrada(entrada,archivoDatos, socketConn);

	list_add(entradas,entrada);

	return 1;
}

void eliminarEntrada(char * key){
	bool* findByKey(void* parametro) {
		t_entrada* entrada = (t_entrada*) parametro;
		return (strcmp(entrada->key,key));
	}

	t_entrada * entrada =(t_entrada *) list_remove_by_condition(entradas,findByKey);
	free(entrada);
}

FILE* inicializarPuntoMontaje(char * path, char * filename){
	/*creo carpeta de Montaje*/
	int status = mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	char* nuevoArchivo = string_new();

	if (status < 0){
		/*informar error en la creación de la carpeta y salir de manera ordenada*/
		log_error(logE, "Fallo al generar el archivo .dat de la instancia.");
		exit(EXIT_FAILURE);
	}

	string_append(&nuevoArchivo, path);
	string_append(&nuevoArchivo, filename);
	string_append(&nuevoArchivo, ".dat");

	FILE* instanciaDat = fopen(nuevoArchivo,"rw");
	if (instanciaDat == NULL){
			log_error(logE, "Fallo al generar el archivo .dat de la instancia.");
			exit(EXIT_FAILURE);
	}

	int fd = fileno(instanciaDat);
	fallocate(fd,0,0,qEntradas*tamanioEntrada); /* se alloca la memoria a mapear */

	return instanciaDat;

}


int escribirEntrada(t_entrada * entrada, FILE* archivoDatos, int socketConn){

	unsigned char* map;
	//map = malloc(sizeof(unsigned char)*(1024*1024));
	int numEntrada;

	char * bloqueArchivo;
	bloqueArchivo = malloc((size_t)tamanioEntrada);

	int data = fileno(archivoDatos);

	map = (unsigned char*) mmap(NULL, qEntradas * tamanioEntrada , PROT_READ | PROT_WRITE, MAP_SHARED, data, sizeof(unsigned char)*qEntradas*tamanioEntrada);

	if (map == MAP_FAILED){
		close(data);
		log_error(logE,"Error en el mapeo de instacia.dat.\n");
		exit(EXIT_FAILURE);
	   }


	int i = 0;
	int j = 0;
	int largoMensaje = 0;
	int bytesRecibidos = 0;
	int recibido = 0;

	recibirInt(socketConn,&largoMensaje);

	while(recibido<largoMensaje){
	int bytesAleer = 0;
	recibirInt(socketConn,&bytesAleer);
		while(bytesRecibidos<bytesAleer){
					bytesRecibidos += recv(socketConn,bloqueArchivo,(size_t)bytesAleer-bytesRecibidos,NULL);
					for (;j<strlen(bloqueArchivo);j++){
							map[i]=bloqueArchivo[j];
							i++;
					}
					j=0;
				}
				recibido += bytesRecibidos;
				bytesRecibidos = 0;


			}

	log_trace(logT,"Se escribió con exito sobre entrada %d.", numEntrada);
	munmap(map,qEntradas * tamanioEntrada);

	free(bloqueArchivo);
	close(data);
	return recibido;
}
