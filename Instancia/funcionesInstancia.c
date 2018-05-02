/*
 * funcionesInstancia.c
 *
 *  Created on: Apr 27, 2018
 *      Author: utnso
 */
#include "funcionesInstancia.h"


int  almacenarEntrada(char key[LONGITUD_CLAVE], FILE* archivoDatos, void * value){

	t_entrada * entrada = malloc(sizeof(t_entrada));

	strcpy(entrada->key,key);
	entrada->entry = escribirEntrada(entrada,archivoDatos, value);/* numero de entrada */
	entrada->size = strlen(value); /* largo de value */

	list_add(tablaEntradas,entrada);

	return 1;
}

void eliminarEntrada(char * key){
	bool* findByKey(void* parametro) {
		t_entrada* entrada = (t_entrada*) parametro;
		return (strcmp(entrada->key,key));
	}

	t_entrada * entrada =(t_entrada *) list_remove_by_condition(tablaEntradas,findByKey);
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


int escribirEntrada(t_entrada * entrada, FILE* archivoDatos, char * escribir){

	unsigned char* map;

	int numEntrada = 0; /* ToDo: depende del algoritmo la asignación de la entrada a utilizar*/

	int data = fileno(archivoDatos);

	map = (unsigned char*) mmap(NULL, qEntradas * tamanioEntrada , PROT_READ | PROT_WRITE, MAP_SHARED, data, sizeof(unsigned char)*numEntrada*tamanioEntrada);

	if (map == MAP_FAILED){
		close(data);
		log_error(logE,"Error en el mapeo de instancia.dat.\n");
		exit(EXIT_FAILURE);
	   }

	int i = 0;

	for (;i<strlen(escribir);i++){
			map[i]=escribir[i];
	}

	int entradasOcupadas = strlen(escribir)/tamanioEntrada;

	if (strlen(escribir) % tamanioEntrada > 0){
		log_trace(logT,"Se escribió con exito sobre la entrada %d y con un total de %d entradas.", numEntrada, entradasOcupadas + 1);
	} else {
		log_trace(logT,"Se escribió con exito sobre la entrada %d y con un total de %d entradas.", numEntrada, entradasOcupadas);

	}
	munmap(map,qEntradas * tamanioEntrada);

	//free(bloqueArchivo);
	close(data);
	return strlen(escribir);

}

int recibirValue(int socketConn, char* bloqueArchivo){

		int largoMensaje = 0;
		int bytesRecibidos = 0;
		int recibido = 0;

		recibirInt(socketConn,&largoMensaje);


			bloqueArchivo = malloc((size_t)largoMensaje);

		while(recibido<largoMensaje){
		int bytesAleer = 0;
		recibirInt(socketConn,&bytesAleer);
			while(bytesRecibidos<bytesAleer){
						bytesRecibidos += recv(socketConn,bloqueArchivo,(size_t)bytesAleer-bytesRecibidos,NULL);

			}
			recibido += bytesRecibidos;
			bytesRecibidos = 0;

		}
		if(recibido <= 0){
			log_error(logT,"no se recibio nada del socket %d",socketConn);
		}
		return recibido;


}

int recibirKey(int socket, char key [LONGITUD_CLAVE]){
	int size = LONGITUD_CLAVE;
		char * mensaje = malloc(size);

			int largoLeido = 0, llegoTodo = 0, totalLeido = 0, tamanio = size;
		while(!llegoTodo){
				largoLeido = recv(socket, mensaje + totalLeido, size, 0);

				//toda esta fumada es para cuando algun cliente se desconecta.
				if(largoLeido == -1){
					printf("Socket dice: Cliente en socket N° %d se desconecto\n", socket);
					log_error(logT,"no se recibio nada del socket %d",socket);
				}

				totalLeido += largoLeido;
				size -= largoLeido;
				if(size <= 0) llegoTodo = 1;
			}
		strcpy(key,mensaje);
		free(mensaje);
		return totalLeido;
}

int recibirEntrada(int socket, FILE * file){

	char key [LONGITUD_CLAVE];
	char * value;

	if(recibirKey(socket,key)<=0){
		return -1;
	}
	if(recibirValue(socket,value)<=0){
		return -1;
	}

	return almacenarEntrada(key,file, value);

}


void configureLoggers(char* instName){

	T = LOG_LEVEL_TRACE;
	I = LOG_LEVEL_INFO;
	E = LOG_LEVEL_ERROR;

	char* logPath = string_new();
	string_append(logPath,"../Logs/");
	string_append(logPath,instName);
	string_append(logPath,".log");

	logT = log_create("../Logs/ESI.log","ESI", false, T);
	logI = log_create("../Logs/ESI.log", "ESI", false, I);
	logE = log_create("../Logs/ESI.log", "ESI", true, E);

	/* 	free(logPath); */
}

void destroyLoggers(){
	log_destroy(logT);
	log_destroy(logI);
	log_destroy(logE);
}


void cargar_configuracion(){

	t_config* infoConfig;

	infoConfig = config_create("../Configuracion/instancia.config");

	if(config_has_property(infoConfig, "IP_COORDINADOR")){
		coordinador_IP = config_get_string_value(infoConfig, "IP_COORDINADOR");
	}

	if(config_has_property(infoConfig, "PUERTO_COORDINADOR")){
		coordinador_Puerto = config_get_int_value(infoConfig, "PUERTO_COORDINADOR");
	}

	if(config_has_property(infoConfig, "ALGORITMO")){
		reemplazo_Algoritmo = config_get_string_value(infoConfig, "ALGORITMO");
	}

	if(config_has_property(infoConfig, "PUNTO_MONTAJE")){
		punto_Montaje = config_get_string_value(infoConfig, "PUNTO_MONTAJE");
	}

	if(config_has_property(infoConfig, "NOMBRE")){
		nombre_Instancia = config_get_string_value(infoConfig, "NOMBRE");
	}

	if(config_has_property(infoConfig, "INTERVALO_DUMP")){
		intervalo_dump = config_get_int_value(infoConfig, "INTERVALO_DUMP");
	}

}
