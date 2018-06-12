#include "funcionesCoordinador.h"
#include <readline/readline.h>


void *esperarConexiones(void *args) {

	t_esperar_conexion *argumentos = (t_esperar_conexion*) args;

	argsPlanificador = malloc(sizeof(t_argumentos_thPlanificador));
	argsConsolaPlanificador = malloc(sizeof(t_argumentos_thPlanificador));

	printf("Esperando conexiones...\n");

	// ---------------ME QUEDO ESPERANDO UNA CONEXION NUEVA--------------

	while (1) {

		int nuevoSocket = -1;
		//char* nombre;

		nuevoSocket = esperarConexionesSocket(&argumentos->fdSocketEscucha,argumentos->socketEscucha);

		if (nuevoSocket != -1) {
			// log_trace(logCoord,"Nuevo Socket!");
			printf("Nueva Conexion Recibida - Socket N°: %d\n",	nuevoSocket);

			int cliente;
			recibirInt(nuevoSocket,&cliente);


			switch(cliente){

			case ESI:
				// printf("ESI.\n");
				log_trace(logT,"Conexion de ESI.");
				//envia al esi su nro de socket para enviarle al planificador
				enviarInt(nuevoSocket, nuevoSocket);
				pthread_t threadAtencionESI;
				t_argumentos_thESI* nuevoESI = malloc(sizeof(t_argumentos_thESI));
				nuevoESI->socketESI = nuevoSocket;
				if(pthread_create(&threadAtencionESI,NULL, atenderESI, (void*) nuevoESI)){
					log_error(logE,"Error generando thread para ESI");
				}
				/*
				if(pthread_join(threadAtencionESI, NULL)){
					log_error(logE,"Error uniendo thread para ESI");
				}*/
				break;

			case PLANIFICADOR:;
			pthread_t threadAtencionPlanificador;
			argsPlanificador->socketPlanificador = nuevoSocket;
			if(pthread_create(&threadAtencionPlanificador,NULL,atenderPlanificador, NULL)){
				log_error(logE,"Error generando thread para Planificador");
			}
			break;

			case CONSOLA_PLANIFICADOR:;
			pthread_t threadAtencionConsolaPlanificador;
			argsConsolaPlanificador->socketPlanificador = nuevoSocket;
			if(pthread_create(&threadAtencionConsolaPlanificador,NULL,atenderConsolaPlanificador, NULL)){
				log_error(logE,"Error generando thread para Planificador");
			}
			break;

			case INSTANCIA:
				crearInstancia(nuevoSocket);
				/* borrar simulaEntrada(nuevoSocket);*/
				break;

			default:
				log_error(logE,"Mensaje no identificado.");
				break;
			}
		}
	}
}

int crearInstancia(int nuevoSocket){
	t_instancia * instancia = malloc(sizeof(t_instancia));

	instancia->nombre = recibirMensajeArchivo(nuevoSocket);
	instancia->socketInstancia = nuevoSocket;
	if(enviarInt(nuevoSocket,cantidad_Entradas)<=0){
		return -1;
	}
	if(enviarInt(nuevoSocket,tamanio_Entrada)<=0){
		return -1;
	}
	instancia->claves = list_create();
	list_add(instancias,instancia);
	log_trace(logT,"Conexión de %s.", instancia->nombre);
	return 1;
}

void eliminarInstancia(t_instancia * instancia){
	bool mismo_nombre(void* parametro) {
		t_instancia* inst = (t_instancia*) parametro;
		return strcmp(inst->nombre,instancia->nombre) == 0;
	}
	list_remove_by_condition(instancias,mismo_nombre);
	free(instancia->nombre);
	list_destroy(instancia->claves);
	free(instancia);
}

int enviarEntradaInstancia(char key[LONGITUD_CLAVE], char* value, t_instancia * instancia){
	if(enviarKey(key,instancia->socketInstancia)<=0){
		log_error(logE,"no se pudo enviar entrada a la instancia %s",instancia->nombre);
		return -1;
	}
	if(enviarValue(value,instancia->socketInstancia)<=0){
		log_error(logE,"no se pudo enviar entrada a la instancia %s",instancia->nombre);
		return -1;
	}

	return 1;

}

int enviarKey(char key[LONGITUD_CLAVE], int socket ){
	int totalEnviado = 0;
	int lenEnviado = 0;
	while(totalEnviado < LONGITUD_CLAVE) {
		lenEnviado = 0;
		lenEnviado = send(socket, &key[totalEnviado], LONGITUD_CLAVE-totalEnviado, 0);
		if(lenEnviado < 0){ perror("error al enviar"); return -1;}
		totalEnviado = totalEnviado + lenEnviado;
	}
	return totalEnviado;
}

int enviarValue(char * value, int socket){
	return enviarMensaje(socket, value);
}

void cargar_configuracion(){

	t_config* infoConfig;

	/* para correr desde ECLIPSE
	infoConfig = config_create("../Recursos/Configuracion/coordinador.config");
	 */

	infoConfig = config_create("../../Recursos/Configuracion/coordinador.config");
	/*para correr desde CONSOLA
	*/

	if(config_has_property(infoConfig, "PUERTO_ESCUCHA")){
		coordinador_Puerto_Escucha = config_get_int_value(infoConfig, "PUERTO_ESCUCHA");
	}

	if(config_has_property(infoConfig, "ALGORITMO")){
		coordinador_Algoritmo = config_get_string_value(infoConfig, "ALGORITMO");
	}

	if(config_has_property(infoConfig, "CANTIDAD_ENTRADAS")){
		cantidad_Entradas = config_get_int_value(infoConfig, "CANTIDAD_ENTRADAS");
	}

	if(config_has_property(infoConfig, "TAMANIO_ENTRADA")){
		tamanio_Entrada = config_get_int_value(infoConfig, "TAMANIO_ENTRADA");
	}

	if(config_has_property(infoConfig, "RETARDO")){
		retardo = config_get_int_value(infoConfig, "RETARDO");
	}

}


void configureLoggers(){

	T = LOG_LEVEL_TRACE;
	I = LOG_LEVEL_INFO;
	E = LOG_LEVEL_ERROR;


	/* para correr desde ECLIPSE
	vaciarArchivo("../Recursos/Logs/Coordinador.log");
	logT = log_create("../Recursos/Logs/Coordinador.log", "Coordinador", false, T);
	logI = log_create("../Recursos/Logs/Coordinador.log", "Coordinador", false, I);
	logE = log_create("../Recursos/Logs/Coordinador.log", "Coordinador", true, E);
	 */

	/* para correr desde CONSOLA
	 */
	vaciarArchivo("../../Recursos/Logs/Coordinador.log");
	logT = log_create("../../Recursos/Logs/Coordinador.log", "Coordinador", true, T);
	logI = log_create("../../Recursos/Logs/Coordinador.log", "Coordinador", true, I);
	logE = log_create("../../Recursos/Logs/Coordinador.log", "Coordinador", true, E);
}

void destroyLoggers(){
	log_destroy(logT);
	log_destroy(logI);
	log_destroy(logE);
}

void generarlogDeOperaciones(){

		FILE *operaciones;

		int nuevo = 0;

		operaciones = fopen("operaciones.log", "r");

		if(!operaciones) //archivo no existe, crear
		{
			operaciones = fopen("operaciones.log", "w");
			nuevo = 1;
			if (operaciones == NULL) printf("Error al generar operaciones.log\n");

		} else {
			freopen("operaciones.log","a",operaciones);
		}

		if(nuevo){
			fprintf(operaciones,"%s%s", "ESI	|", "	OPERACION\n");
		}

		fclose(operaciones);
}

int logueaOperacion(char* operacion, char* key, char* value, int socket){

	pthread_mutex_lock(&mx_logOp);
	FILE *operaciones;
	operaciones = fopen("operaciones.log", "a");

	char* linea = string_new();

	/*creo linea para log*/
	string_append(&linea, "ESI");
	char* ESIindx = string_itoa(socket);
	string_append(&linea, ESIindx);
	string_append(&linea, "	|	");
	string_append(&linea, operacion);
	string_append(&linea, " ");
	string_append(&linea, key);
	string_append(&linea, " ");
	string_append(&linea, value);

	fprintf(operaciones,"%s", linea);

	free(linea);
	free(ESIindx); /*???*/

	fclose(operaciones);
	pthread_mutex_unlock(&mx_logOp);
	/* chequear para error y devolver -1*/
	return 1;
}

void* atenderESI(void *args){

	t_argumentos_thESI* nuevoESI = (t_argumentos_thESI*) args;
	log_trace(logT, "id de socket %d", nuevoESI->socketESI);
	recibirMensajeESI(nuevoESI->socketESI);
	return &(nuevoESI->socketESI);
}

void recibirMensajeESI(int socket){

	int mensaje;
	char* clave;
	char* valor;
	t_instancia* instancia;
	int retorno;



	while(1){


		recibirInt(socket, &mensaje);

//		log_trace(logT, "recibi mensaje del esi %d, pidio un %d", socket, mensaje);
		switch(mensaje){

		case GET_KEY:;
			clave = recibirMensajeArchivo(socket);
			logueaOperacion("GET",clave,"",socket);
			enviarInt(argsPlanificador->socketPlanificador, GET_KEY);
			enviarMensaje(argsPlanificador->socketPlanificador, clave);

			int codigo;
			recibirInt(argsPlanificador->socketPlanificador, &codigo);

			switch(codigo){
				case CLAVE_OTORGADA:;
				ejecutarOperacionGET(clave);
				enviarInt(socket, EJECUCION_OK);
				break;

				case CLAVE_BLOQUEADA:;
				//TODO que deberia hacer el coordinador?
				enviarInt(socket, EN_ESPERA);
				break;
			}
			//todo: validar mensajes de errores hacia ESI de operacion GET
			break;

		case SET_KEY:
			clave = recibirMensajeArchivo(socket);
			valor = recibirMensajeArchivo(socket);
			logueaOperacion("SET",clave,valor,socket);

			retorno = buscarInstanciaContenedora(clave, instancia);
			if(retorno == -1){
				enviarInt(socket, CLAVE_INEXISTENTE);
				break;
			}
			enviarInt(argsPlanificador->socketPlanificador, SET_KEY);
			enviarMensaje(argsPlanificador->socketPlanificador, clave);
			recibirInt(argsPlanificador->socketPlanificador, &codigo);
			switch(codigo){
			case CLAVE_RESERVADA:;
			ejecutar_operacion_set(clave, valor, instancia);
			enviarInt(socket, EJECUCION_OK);
			break;
			case CLAVE_NO_RESERVADA:;
			enviarInt(socket, EJECUCION_INVALIDA);
			break;
			case CLAVE_INEXISTENTE:;
			enviarInt(socket, EJECUCION_INVALIDA);
			break;
			}
			break;

		case STORE_KEY:
			clave = recibirMensajeArchivo(socket);
			retorno = buscarInstanciaContenedora(clave, instancia);
			logueaOperacion("STORE",clave,"",socket);
			if(retorno == -1){
				enviarInt(socket, CLAVE_INEXISTENTE);
				break;
			}
			enviarInt(argsPlanificador->socketPlanificador, STORE_KEY);
			enviarMensaje(argsPlanificador->socketPlanificador, clave);
			recibirInt(argsPlanificador->socketPlanificador, &codigo);
			switch(codigo){
			case CLAVE_LIBERADA:;
			ejecutar_operacion_store(clave, instancia);
			enviarInt(socket, EJECUCION_OK);
			break;
			case CLAVE_NO_RESERVADA:;
			enviarInt(socket, EJECUCION_INVALIDA);
			break;
			case CLAVE_INEXISTENTE:;
			enviarInt(socket, EJECUCION_INVALIDA);
			break;
			}
			break;

		default:
			log_error(logE,"error de codigo, recibi %d", mensaje);
			break;

		}
	}

}

void atenderPlanificador(void *args){
	while(1){

	//	recibirMensajePlanificador(argsPlanificador->socketPlanificador);
		//TODO
		sleep(5);
	}

}

void atenderConsolaPlanificador(void *args){

	while(1){

		//recibirMensajeConsolaPlanificador(argsConsolaPlanificador->socketPlanificador);
		//TODO
		sleep(5);
	}

}

void recibirMensajeConsolaPlanificador(int socket){
	int mensaje;

	recibirInt(socket, &mensaje);

	t_instancia* instancia;
	char* clave;
	int retorno;

	switch(mensaje){

	case DONDE_ESTA_LA_CLAVE:

		clave = recibirMensajeArchivo(socket);
		retorno = simularBuscarInstanciaContenedora(clave, instancia);
		if(retorno > 0){
			enviarInt(socket, CLAVE_ENCONTRADA);
		}else{
			enviarInt(socket, CLAVE_NO_ENCONTRADA);
		}

		enviarMensaje(socket, instancia->nombre);
		break;

	default:

		log_error(logE, "No reconozco ese mensaje %d\n", mensaje);

	}
}

void recibirMensajePlanificador(int socket){

	int mensaje;

	recibirInt(socket, &mensaje);

	//TODO: Implementar los mensajes necesarios
	switch(mensaje){

	default:

		log_error(logE, "No reconozco ese mensaje\n");

	}

}

/*********** OPERACION GET **************/
int ejecutarOperacionGET(char key[LONGITUD_CLAVE]){
	int pos = -1;
	t_instancia * instancia;
	pos = buscarInstanciaContenedora(key, instancia);
	if(pos<0){
		log_trace(logT,"No se encontro la clave %s en ninguna instancia", key);
		if(elegirInstancia(instancia)<0){
			log_trace(logE,"No se pudo guardar la clave %s en ninguna instancia", key);
			return -1;
		}
	}

	return bloquearKey(key);
}


int bloquearKey(char key[LONGITUD_CLAVE]){

	list_add(claves_bloqueadas, key);
	log_trace(logT, "clave %s bloqueada", key);
	return 1;
}

int elegirInstancia(t_instancia * instancia){

	if(strcmp(coordinador_Algoritmo,"EL")==0){
		if(proxima_posicion_instancia  >= list_size(instancias) ){
			proxima_posicion_instancia = 0;
		}
		instancia = list_get(instancias,proxima_posicion_instancia);
		log_trace(logT,"se eligio la instancia %s para el guardado de clave", instancia->nombre);
		return proxima_posicion_instancia++;
	}else
		if(strcmp(coordinador_Algoritmo,"LSU")==0){
			//TODO
		}else
			if(strcmp(coordinador_Algoritmo,"KE")==0){
				//TODO
			}


	return -1;
}

int buscarInstanciaContenedora(char key[LONGITUD_CLAVE], t_instancia * instancia){
	int pos = -1;
	int encontro = 0;
	bool contieneClave(void* parametro) {
		t_instancia* inst = (t_instancia*) parametro;
		encontro = contieneClaveInstancia(inst,key);
		++pos;
		return encontro;
	}

	instancia = (t_instancia *)list_find(instancias,contieneClave);
	if(encontro){
		return pos;
	}else{
		return -1;
	}

}

int simularBuscarInstanciaContenedora(char key[LONGITUD_CLAVE], t_instancia* instancia){

	/*todo: busca la clave en las instancias. Si no la encuentra, simula guardarla. Siempre almacena
	 * el valor de la instancia donde esta la clave o donde estaria la clave en la instancia que recibe
	 * la funcion como parametro y retorna 1 en el caso que haya encontrado la clave, 0 en el caso de que
	 * haya simulado la distribucion.
	 */

	return 0;

}


int contieneClaveInstancia(t_instancia * instancia, char key[LONGITUD_CLAVE]){

	bool* contieneClave(void* parametro) {
		char* clave = (char*) parametro;
		return (strcmp(clave,key)==0);
	}

	t_list* list = list_filter(instancia->claves,contieneClave);
	int size = list_size(list);
	free(list);
	return size;
}
/**************** FIN OPERACION GET ***************/


/***************** OPERACION SET ******************/
int ejecutar_operacion_set(char key[LONGITUD_CLAVE], char * value, t_instancia * instancia){
	int socket = instancia->socketInstancia;
	if(enviarInt(socket,ENVIO_ENTRADA)<=0){
		log_trace(logE,"error de comunicacion con la instancia %s al enviar la clave %s",instancia->nombre, key);
		return -1;
	}
	if(enviarKey(key, socket)<=0){
		log_trace(logE,"error al enviar la clave %s a la instancia %s",key,instancia->nombre);
		return -1;

	}
	if(enviarValue(value,socket)){
		log_trace(logE,"error al enviar el valor %s de la clave %s a la instancia %s",value,key,instancia->nombre);
		return -1;
	}
	return 1;
}
/*************** FIN OPERACION SET ****************/

/*************** OPERACION STORE *****************/
int ejecutar_operacion_store(char key[LONGITUD_CLAVE], t_instancia * instancia){
	int socket = instancia->socketInstancia;
	if(enviarInt(socket,STORE_ENTRADA)<=0){
		liberar_clave(key);
		log_trace(logE,"error al ejecutar STORE con instancia %s de la clave %s",instancia->nombre, key);
		return -1;
	}else{
		if(enviarKey(key,socket)<=0){
			liberar_clave(key);
			log_trace(logE,"error al ejecutar STORE con instancia %s de la clave %s",instancia->nombre, key);
			return -1;
		}
	}
	liberar_clave(key);
	return 1;
}

void liberar_clave(char key[LONGITUD_CLAVE]){

	bool* igualClave(void* parametro) {
		char* clave = (char*)parametro;
		return (strcmp(clave,key)==0);
	}

	list_remove(claves_bloqueadas,igualClave);
	log_trace(logT, "se libero la clave %s", key);
}
/*************** FIN OPERACION STORE *****************/

/**************** FUNCIONES PRUEBA ************************/
void simulaEntrada(int socket){
	char* linea;
	linea = readline("ComandoESI:" );
	char** parametros;
	parametros = string_split(linea, " ");

	char key[LONGITUD_CLAVE];
	strcpy(key,parametros[0]);
	/*
	 * parametro 0 = key
	 * parametro 1 = value
	 *
	 * */
	enviarKey(key,socket);
	enviarValue(parametros[1],socket);
}
