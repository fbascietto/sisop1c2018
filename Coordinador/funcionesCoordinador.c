#include "funcionesCoordinador.h"
#include <readline/readline.h>
#include <limits.h>


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
	t_instancia * instancia;
	char* nombreInstancia = recibirMensajeArchivo(nuevoSocket);

	if(existeInstancia(nombreInstancia,instancia)){
		instancia->socketInstancia = nuevoSocket;
		free(nombreInstancia);
		return 1;
	}

	instancia= malloc(sizeof(t_instancia));

	instancia->nombre = nombreInstancia;
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

bool existeInstancia(char* nombreInstancia, t_instancia * instancia){
	bool encontro = false;
	void mismo_nombre(void* parametro) {
			t_instancia* inst = (t_instancia*) parametro;
			if( strcmp(inst->nombre,nombreInstancia) == 0){
				instancia = inst;
				encontro = true;
			}
	}

	list_iterate(instancias,mismo_nombre);
	return encontro;

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


	/*para correr desde CONSOLA
	 */
	infoConfig = config_create("../../Recursos/Configuracion/coordinador.config");

	if(config_has_property(infoConfig, "PUERTO_ESCUCHA")){
		coordinador_Puerto_Escucha = config_get_int_value(infoConfig, "PUERTO_ESCUCHA");
	}

	if(config_has_property(infoConfig, "ALGORITMO")){
		coordinador_Algoritmo = config_get_string_value(infoConfig, "ALGORITMO");
	}

	if(config_has_property(infoConfig, "CANTIDAD_ENTRADAS")){
		cantidad_Entradas = config_get_int_value(infoConfig, "CANTIDAD_ENTRADAS");
	}

	if(config_has_property(infoConfig, "TAMANO_ENTRADAS")){
		tamanio_Entrada = config_get_int_value(infoConfig, "TAMANO_ENTRADAS");
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
	logT = log_create("../Recursos/Logs/Coordinador.log", "Coordinador", true, T);
	logI = log_create("../Recursos/Logs/Coordinador.log", "Coordinador", true, I);
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

	bool finalizar = false;
	int mensaje;
	while(1){

		recibirInt(socket, &mensaje);

		switch(mensaje){

		case GET_KEY:;
			ejecutarOperacionGET(socket);
			break;

		case SET_KEY:
			ejecutar_operacion_set(socket);
			break;

		case STORE_KEY:
			ejecutar_operacion_store(socket);
			break;
		case FINALIZACION_OK:
			finalizar = true;
			break;
		default:
			log_error(logE,"error de codigo, recibi %d", mensaje);
			finalizar = true;
			break;

		}
		if(finalizar){
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

		recibirMensajeConsolaPlanificador(argsConsolaPlanificador->socketPlanificador);
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
		retorno = buscarInstanciaContenedora(clave,instancia);
		if(retorno > 0){
			enviarInt(socket, CLAVE_ENCONTRADA);
		}else{
			simularBuscarInstanciaContenedora(clave, instancia);
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
int ejecutarOperacionGET(int socket){

	char * clave;
	clave = recibirMensajeArchivo(socket);
	logueaOperacion("GET",clave,"",socket);
	enviarInt(argsPlanificador->socketPlanificador, GET_KEY);
	enviarMensaje(argsPlanificador->socketPlanificador, clave);

	int codigo;
	if(recibirInt(argsPlanificador->socketPlanificador, &codigo)<=0){
		log_error(logE,"error al recibir status de la clave %s del planificador",codigo);
	}
	char key[LONGITUD_CLAVE];
	strcpy(key,clave);
	list_add(claves_sin_instancia,key);
	free(clave);
	switch(codigo){
		log_info(logI, "recibi %d", codigo);
		case CLAVE_OTORGADA:
			if(enviarInt(socket, EJECUCION_OK)<=0){
				log_error(logE,"error de conexion con ESI en socket %d",socket);
				return -1;
			}
			break;

		case CLAVE_BLOQUEADA:
			if(enviarInt(socket, CLAVE_BLOQUEADA)<=0){
				log_error(logE, "error de conexion con ESI en socket %d",socket);
				return -1;;
			}
		break;
	}
	return 1;
				//todo: validar mensajes de errores hacia ESI de operacion GET}
}


int bloquearKey(char key[LONGITUD_CLAVE]){


	log_trace(logT, "clave %s bloqueada", key);
	return 1;
}

int elegirInstancia(t_instancia * instancia, char key[LONGITUD_CLAVE], bool esSimulacion){

	if(strcmp(coordinador_Algoritmo,"EL")==0){
		int proximaPosicion = proxima_posicion_instancia;
		if(proximaPosicion  >= list_size(instancias) ){
			proximaPosicion = 0;
		}
		instancia = list_get(instancias,proximaPosicion);
		log_trace(logT,"se eligio la instancia %s para el guardado de clave", instancia->nombre);
		proximaPosicion++;
		if(esSimulacion){
			proxima_posicion_instancia = proximaPosicion;
		}
		return proximaPosicion;
	}else
		if(strcmp(coordinador_Algoritmo,"LSU")==0){
			return ejecutarAlgoritmoLSU(instancia);
		}else
			if(strcmp(coordinador_Algoritmo,"KE")==0){
				bool conectada(void* parametro) {
						t_instancia* inst = (t_instancia*) parametro;

						return (inst->socketInstancia > 0);
					}
				char letra = key[0];
				t_list * instancias_conectadas = list_filter(instancias,conectada);
				int q = list_size(instancias_conectadas);
				if(q<=0){
					log_trace(logT,"no hay instancias conectadas para almacenar la clave %s", key);
					return -1;
				}
				int q_letras = 26 / q;
				int resto = 26 % q;
				int i = 0;
				for(;i<q;i++){
					if(letra-26 >= (q_letras*i) && letra-26 < (q_letras*(i+1))){
						instancia = list_get(instancias_conectadas,i);
					}else
					if(resto>0 && q == i){
						instancia = list_get(instancias_conectadas,i);
					}
				}
				free(instancias_conectadas);
				return 1;
			}


	return -1;
}

int ejecutarAlgoritmoLSU(t_instancia* instancia){
	int menorCantidadDeEntradas=INT_MAX;
	t_instancia* instanciaIterada = malloc(sizeof(t_instancia));
	int i;
	for (i = 0; i < list_size(instancias); ++i) {
		instanciaIterada = list_get(instancias,i);
		if(instanciaIterada->entradasOcupadas < menorCantidadDeEntradas){
			instancia = instanciaIterada;
			menorCantidadDeEntradas = instanciaIterada->entradasOcupadas;
		}
	}
	return 1;
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
	return elegirInstancia(instancia, key, true);
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
int ejecutar_operacion_set(int socket){
	char * clave;
	char * valor;
	char key[LONGITUD_CLAVE];
	t_instancia * instancia;

	clave = recibirMensajeArchivo(socket);
	valor = recibirMensajeArchivo(socket);
	logueaOperacion("SET",clave,valor,socket);

	strcpy(key,clave);


	if(buscarInstanciaContenedora(key, instancia)<0){

		log_trace(logT,"No se encontro la clave %s en ninguna instancia", key);
		if(!key_creada(clave)){
			log_trace(logT,"La clave %s no se encuentra creada", key);
			enviarInt(socket, CLAVE_INEXISTENTE);
			free(valor);
			free(clave);
			return -1;
		}
		if(elegirInstancia(instancia,key,false)<=0){
			log_trace(logT,"No se puede almacenar la clave %s en ninguna instancia", key);
			enviarInt(socket, EJECUCION_INVALIDA);
			free(valor);
			free(clave);
			return -1;
		}
		enviarInt(argsPlanificador->socketPlanificador, SET_KEY);
		enviarMensaje(argsPlanificador->socketPlanificador, clave);
		int codigo;
		recibirInt(argsPlanificador->socketPlanificador, &codigo);
		free(clave);

		switch(codigo){
			case CLAVE_RESERVADA:
				if(ejecutar_operacion_set_instancia(key, valor, instancia)<=0){
					return ejecutar_operacion_set(socket);
				}else{
					int cantidadEntradasOcupadas;
					recibirInt(instancia->socketInstancia, &cantidadEntradasOcupadas);
					list_add(instancia->claves,&key);
					instancia->entradasOcupadas += cantidadEntradasOcupadas;
					enviarInt(socket, EJECUCION_OK);
				}
				break;
			case CLAVE_NO_RESERVADA:
				enviarInt(socket, EJECUCION_INVALIDA);
				break;
			case CLAVE_INEXISTENTE:
				enviarInt(socket, EJECUCION_INVALIDA);
				break;
		}
		free(valor);

	}

	return 1;

}

bool key_creada(char * key){
	bool* igualClave(void* parametro) {
			char* clave = (char*)parametro;
			return (strcmp(clave,key)==0);
		}

	return list_any_satisfy(claves_sin_instancia,igualClave);
}

int ejecutar_operacion_set_instancia(char key[LONGITUD_CLAVE], char * value, t_instancia * instancia){
	int socket = instancia->socketInstancia;
	if(enviarInt(socket,ENVIO_ENTRADA)<=0){
		instancia->socketInstancia = -1;
		log_trace(logE,"error de comunicacion con la instancia %s al enviar la clave %s",instancia->nombre, key);
		return -1;
	}
	if(enviarKey(key, socket)<=0){
		instancia->socketInstancia = -1;
		log_trace(logE,"error al enviar la clave %s a la instancia %s",key,instancia->nombre);
		return -1;

	}
	if(enviarValue(value,socket)){
		instancia->socketInstancia = -1;
		log_trace(logE,"error al enviar el valor %s de la clave %s a la instancia %s",value,key,instancia->nombre);
		return -1;
	}
	return 1;
}
/*************** FIN OPERACION SET ****************/

/*************** OPERACION STORE *****************/

int ejecutar_operacion_store(int socket){
	t_instancia * instancia;
	char* clave;
	clave = recibirMensajeArchivo(socket);

	char key[LONGITUD_CLAVE];
	strcpy(key,clave);
	int instancia_encontrada;
	logueaOperacion("STORE",clave,"",socket);
	instancia_encontrada = buscarInstanciaContenedora(key, instancia);
	if( instancia_encontrada< 0){
		if(!key_creada(clave)){
			log_trace(logT,"La clave %s no se encuentra creada", clave);
			enviarInt(socket, CLAVE_INEXISTENTE);
			free(clave);
			return -1;
		}else{
			log_trace(logT,"La clave %s no se encuentra seteada", clave);
			enviarInt(socket, EJECUCION_INVALIDA);

		}
	}
	enviarInt(argsPlanificador->socketPlanificador, STORE_KEY);
	enviarMensaje(argsPlanificador->socketPlanificador, clave);
	free(clave);
	int codigo;
	recibirInt(argsPlanificador->socketPlanificador, &codigo);
	switch(codigo){
		case CLAVE_LIBERADA:
			if(instancia_encontrada >= 0 ){
				if(ejecutar_operacion_store_instancia(key, instancia)<=0){
					enviarInt(socket, ERROR_EJECUCION);
					return -1;
				}
			}else{
				enviarInt(socket, EJECUCION_INVALIDA);
				break;
			}
			enviarInt(socket, EJECUCION_OK);
			break;
		case CLAVE_NO_RESERVADA:;
			enviarInt(socket, EJECUCION_INVALIDA);
			break;
		case CLAVE_INEXISTENTE:;
			enviarInt(socket, EJECUCION_INVALIDA);
			break;
	}
	return 1;
}

int ejecutar_operacion_store_instancia(char key[LONGITUD_CLAVE], t_instancia * instancia){
	int socket = instancia->socketInstancia;
	if(enviarInt(socket,STORE_ENTRADA)<=0){
		instancia->socketInstancia = -1;
		liberar_clave(key);
		log_trace(logE,"error al ejecutar STORE con instancia %s de la clave %s",instancia->nombre, key);
		return -1;
	}else{
		if(enviarKey(key,socket)<=0){
			instancia->socketInstancia = -1;
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

	list_remove(claves_sin_instancia,igualClave);
	log_trace(logT, "se libero la clave %s", key);
}
/*************** FIN OPERACION STORE *****************/






/**************** FUNCIONES PRUEBA ************************/
void simulaEntrada(int socket){

	char* lineaGET;
	lineaGET = readline("SET:" );
	char** parametrosGET;
	parametrosGET = string_split(lineaGET, " ");

	char keyGET[LONGITUD_CLAVE];
	strcpy(keyGET,parametrosGET[0]);

	//ejecutarOperacionGET(keyGET);

	char* linea;
	linea = readline("SET:" );
	char** parametros;
	parametros = string_split(linea, " ");


	char key[LONGITUD_CLAVE];
	strcpy(key,parametros[0]);
	/*
	 * parametro 0 = key
	 * parametro 1 = value
	 *
	 * */
	t_instancia * instancia;
	int a = buscarInstanciaContenedora(key, instancia);
	enviarInt(socket,ENVIO_ENTRADA);
	enviarKey(key,socket);
	enviarValue(parametros[1],socket);

	char* otra_linea = readline("STORE:" );
	char** otros_parametros;
	otros_parametros = string_split(otra_linea, " ");
	enviarInt(socket,STORE_ENTRADA);
	char otra_key[LONGITUD_CLAVE];
	strcpy(otra_key,otros_parametros[0]);
	enviarKey(otra_key,socket);

}
