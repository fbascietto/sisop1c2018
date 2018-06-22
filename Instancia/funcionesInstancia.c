/*
 * funcionesInstancia.c
 *
 *  Created on: Apr 27, 2018
 *      Author: utnso
 */
#include "funcionesInstancia.h"



int  almacenarEntrada(char key[LONGITUD_CLAVE], int entradaInicial, int largoValue){
	t_entrada * entrada;
	if(obtenenerEntrada(key,&entrada)){
		entrada = malloc(sizeof(t_entrada));
		strcpy(entrada->key,key);
		list_add(tablaEntradas,entrada);
	}
	entrada->ultimaRef = operacionNumero;
	entrada->entry = entradaInicial; /* numero de entrada */
	entrada->size = largoValue;  /* largo de value */

	return 1;

}

bool obtenerEntrada(char key[LONGITUD_CLAVE],t_entrada ** entrada){

	bool* retorno = false;
	bool* findByKey(void* parametro) {
		t_entrada* entrada = (t_entrada*) parametro;
		if(strcmp(entrada->key,key) == 0){
			*retorno = true;
		}
		return retorno;
	}
	*entrada =(t_entrada *) list_find(tablaEntradas,findByKey);
	return *retorno;
}

void eliminarEntrada(char * key){
	bool* findByKey(void* parametro) {
		t_entrada* entrada = (t_entrada*) parametro;
		return (strcmp(entrada->key,key)==0);
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


int escribirEntrada(FILE* archivoDatos, char * escribir){

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

	close(data);
	return strlen(escribir);

}

int recibirValue(int socketConn, char** value){

		*value = recibirMensajeArchivo(socketConn);

		if(strcmp(*value,"-1") == 0){
			return -1;
		}

		return 1;


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

int obtenerCantidadEntradasLibres(){
	bool estaVacia(void* entrada){
		t_entrada* entradaInst = (void*) entrada;
		return entradaInst->key == NULL;
	}
	return list_count_satisfying(tablaEntradas, estaVacia);
}

int obtenerCantidadEntradasOcupadas(){
	bool estaOcupada(void* entrada){
		t_entrada* entradaInst = (void*) entrada;
		return entradaInst->key != NULL;
	}
	return list_count_satisfying(tablaEntradas, estaOcupada);
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

	calcularSiguienteEntrada();
	almacenarEntrada(key, numEntradaActual, lenValue);

	for(int i=0;i<entradasAOcupar;i++){
		char* segmento;
		segmento = malloc(tamanioEntrada);
		segmento = strncpy(segmento,value+(i*tamanioEntrada),tamanioEntrada);
		escribirEntrada(file, segmento);

	}


	return entradasAOcupar;

}
/******** FIN OPERACION SET **********/


/******** OPERACION STORE **********/
int ejecutarStore(int coordinador_socket, FILE* archivoDatos){
		char key[LONGITUD_CLAVE];
		if(recibirKey(coordinador_socket,key)<=0){
			log_trace(logE, "error al recibir clave para persistir");
			return -1;
		}else{
			if(persistir_clave(key, archivoDatos)<=0){
				log_trace(logE, "error al persistir clave");
				return -1;
			}
		}
		return 1;
}


int persistir_clave(char key[LONGITUD_CLAVE], FILE* archivoDatos){


	char* path_final = string_new();

	string_append(&path_final, punto_Montaje);
	string_append(&path_final, key);

	FILE* keyStore = fopen(path_final,"w+");
	if (keyStore == NULL){
			log_error(logE, "Fallo al generar el STORE de la key %s.", key);
			exit(EXIT_FAILURE);
	}


	t_entrada* entradaElegida;

	if(!obtenerEntrada(key,&entradaElegida)){
		log_error(logE,"No se encontro la clave %s",key);
	}

	entradaElegida->ultimaRef = operacionNumero;

	char* value = malloc(entradaElegida->size);

	leer_entrada(entradaElegida, archivoDatos, value);

	fprintf(keyStore,"%s", value);

	fclose(keyStore);
	free(value);
	free(path_final);
	return 1;
}

void leer_entrada(t_entrada* entrada, FILE* archivoDatos, char* value){

	int data = open(archivoDatos,O_RDWR);
	struct stat fileStat;
	if (fstat(data, &fileStat) < 0){
		log_error(logE,"Error fstat --> %s");
		exit(EXIT_FAILURE);
	}

	unsigned char* map = (unsigned char*) mmap(NULL, qEntradas * tamanioEntrada , PROT_READ | PROT_WRITE, MAP_SHARED, data, sizeof(unsigned char)*entrada->entry*tamanioEntrada);

	if (map == MAP_FAILED){
		close(data);
		log_error(logE,"Error en el mapeo del archivo.dat.\n");
		exit(EXIT_FAILURE);
	   }

	int bytesAleer = entrada->size;

	int bytes_totales_leidos = 0;
	int bytes_leidos = 0;

	while(bytes_totales_leidos < bytesAleer){


		for (;bytes_leidos<bytesAleer && bytes_totales_leidos< bytesAleer;bytes_totales_leidos++){
			value[bytes_leidos] = map[bytes_totales_leidos];
			bytes_leidos++;
		}
		bytes_leidos=0;


	}

	log_trace(logT,"Se leyó con exito el value de la clave %s.", entrada->key);
	munmap(map,fileStat.st_size);

}

/********* FIN OPERACION STORE *********/

void configureLoggers(char* instName){

	T = LOG_LEVEL_TRACE;
	I = LOG_LEVEL_INFO;
	E = LOG_LEVEL_ERROR;

	char* logPath = string_new();

	/* para correr desde ECLIPSE */
	string_append(&logPath,"../Recursos/Logs/");


	/* para correr desde CONSOLA
	string_append(&logPath,"../../Recursos/Logs/");
*/
	string_append(&logPath,instName);
	string_append(&logPath,".log");


	//vaciarArchivo(logPath);
	logT = log_create(logPath,"Instancia", true, T);
	logI = log_create(logPath, "Instancia", true, I);
	logE = log_create(logPath, "Instancia", true, E);


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

int calculoCircular(int lenValue){

	int size = list_size(tablaEntradas);
	int entradasOcupadas = 0;

	void calcularEntradasOcupadas(void* parametro) {
		t_entrada* entrada = (t_entrada*) parametro;

		entradasOcupadas += (entrada->size / tamanioEntrada);

		if(entrada->size%tamanioEntrada){
			entradasOcupadas++;
		}
	}

	list_iterate(tablaEntradas,calcularEntradasOcupadas);

	if(entradasOcupadas==qEntradas){
		/* empieza a reemplazar entradas, modificación en la lista buscando espacio en el que entre el nuevo value,
		 * se tiene que considerar agregar el parámetro del tamaño del nuevo value */
		for(int i=0;i<size;i++){
			t_entrada* ent = list_get(tablaEntradas,i);
			if(lenValue < ent->size){
				entradasOcupadas = ent->entry;
				break;
			}
		}

		if(entradasOcupadas == qEntradas){return -1; /*TODO que hacer en este caso?*/}
	}

	return entradasOcupadas;
}

void cargar_configuracion(){

	t_config* infoConfig;

	/* SI SE CORRE DESDE ECLIPSE */
	infoConfig = config_create("../Recursos/Configuracion/instancia.config");


	/* SI SE CORRE DESDE CONSOLA
	infoConfig = config_create("../../Recursos/Configuracion/instancia.config");
*/
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

