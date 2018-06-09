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
	entrada->entry = escribirEntrada(entrada,archivoDatos, value); /* numero de entrada */
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

	if (status < 0 && (errno != EEXIST)){
		/*informar error en la creación de la carpeta y salir de manera ordenada*/
		log_error(logE, "Fallo al generar el archivo .dat de la instancia.");
		exit(EXIT_FAILURE);
	}

	string_append(&nuevoArchivo, path);
	string_append(&nuevoArchivo, filename);
	string_append(&nuevoArchivo, ".dat");

	FILE* instanciaDat = fopen(nuevoArchivo,"w+");
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

	int data = fileno(archivoDatos);

	map = (unsigned char*) mmap(NULL, qEntradas * tamanioEntrada , PROT_READ | PROT_WRITE, MAP_SHARED, data, sizeof(unsigned char)*numEntradaActual*tamanioEntrada);

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
		log_trace(logT,"Se escribió con exito sobre la entrada %d y con un total de %d entradas.", numEntradaActual, entradasOcupadas + 1);

	} else {
		log_trace(logT,"Se escribió con exito sobre la entrada %d y con un total de %d entradas.", numEntradaActual, entradasOcupadas);
	}
	munmap(map,qEntradas * tamanioEntrada);



	//free(bloqueArchivo);
	close(data);
	return strlen(escribir);

}

int recibirValue(int socketConn, char** value){

		int largoMensaje = 0;
		int bytesRecibidos = 0;
		int recibido = 0;

		recibirInt(socketConn,&largoMensaje);

		*value = malloc((size_t)largoMensaje);

		while(recibido<largoMensaje){
			while(bytesRecibidos<largoMensaje){
						bytesRecibidos += recv(socketConn,*value,(size_t)largoMensaje-bytesRecibidos,NULL);

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




/********** OPERACION SET ************/
int recibirEntrada(int socket, FILE * file){

	char key [LONGITUD_CLAVE];
	char * value;
	//value = string_new();
	if(recibirKey(socket,key)<=0){
		return -1;
	}
	if(recibirValue(socket,&value)<=0){
		return -1;
	}

	int lenValue = strlen(value);
	int entradasAOcupar;

	if(lenValue % tamanioEntrada){
		entradasAOcupar = (lenValue / tamanioEntrada) +1;
	} else {
		entradasAOcupar = (lenValue / tamanioEntrada);
	}

	for(int i=0;i<entradasAOcupar;i++){
		char* segmento;
		segmento = malloc(tamanioEntrada);
		segmento = strncpy(segmento,value+(i*tamanioEntrada),tamanioEntrada);
		almacenarEntrada(key,file, segmento);
		numEntradaActual = calculoCircular();
	}

	return lenValue;

}
/******** FIN OPERACION SET **********/


/******** OPERACION STORE **********/
int ejecutarStore(int coordinador_socket){
		char key[LONGITUD_CLAVE];
		if(recibirKey(coordinador_socket,key)<=0){
			log_trace(logE, "error al recibir clave para persistir");
			return -1;
		}else{
			if(persistir_clave(key)<=0){
				log_trace(logE, "error al persisir clave");
				return -1;
			}
		}
		return 1;
}


int persistir_clave(char key[LONGITUD_CLAVE]){
	//TODO persisir tipo archivos onda el dump??
	return 1;
}

/********* FIN OPERACION STORE *********/

void configureLoggers(char* instName){

	T = LOG_LEVEL_TRACE;
	I = LOG_LEVEL_INFO;
	E = LOG_LEVEL_ERROR;

	char* logPath = string_new();

	/* para correr desde ECLIPSE
	string_append(&logPath,"../Recursos/Logs/");
	 */

	/* para correr desde CONSOLA*/
	string_append(&logPath,"../../Recursos/Logs/");

	string_append(&logPath,instName);
	string_append(&logPath,".log");


	vaciarArchivo(logPath);
	logT = log_create(logPath,"Instacia", true, T);
	logI = log_create(logPath, "Instacia", true, I);
	logE = log_create(logPath, "Instacia", true, E);


	 	free(logPath);
}

void destroyLoggers(){
	log_destroy(logT);
	log_destroy(logI);
	log_destroy(logE);
}

int algoritmoR(char* algoritmo){
	int value;

	if(!strncmp(algoritmo, "CIRCULAR", 8)){
		value = CIRCULAR;
	} else if (!strncmp(algoritmo, "LRU", 3)){
		value = LRU;
	} else if (!strncmp(algoritmo, "BSU", 3)){
		value = BSU;
	} else {value = -1;}

	return value;
}

int calculoCircular(){
	int size = list_size(tablaEntradas);
	return size;
}

void cargar_configuracion(){

	t_config* infoConfig;

	/* SI SE CORRE DESDE ECLIPSE
	infoConfig = config_create("../Recursos/Configuracion/instancia.config");
	 */

	/* SI SE CORRE DESDE CONSOLA*/
	infoConfig = config_create("../../Recursos/Configuracion/instancia.config");

	if(config_has_property(infoConfig, "IP_COORDINADOR")){
		coordinador_IP = config_get_string_value(infoConfig, "IP_COORDINADOR");
	}

	if(config_has_property(infoConfig, "PUERTO_COORDINADOR")){
		coordinador_Puerto = config_get_int_value(infoConfig, "PUERTO_COORDINADOR");
	}

	if(config_has_property(infoConfig, "ALGORITMO")){
		reemplazo_Algoritmo = algoritmoR(config_get_string_value(infoConfig, "ALGORITMO"));
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

