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

				break;

			case PLANIFICADOR:;
			argsPlanificador->socketPlanificador = nuevoSocket;
			break;

			case CONSOLA_PLANIFICADOR:;

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

	if(enviarInt(nuevoSocket,cantidad_Entradas)<=0){
			return -1;
		}
		if(enviarInt(nuevoSocket,tamanio_Entrada)<=0){
			return -1;
		}

	if(existeInstancia(nombreInstancia,&instancia)){
		instancia->socketInstancia = nuevoSocket;
		enviarClavesAGuardar(instancia);
		free(nombreInstancia);
		int entradasOcupadas = 0;
		recibirInt(instancia->socketInstancia,&entradasOcupadas);
		instancia->entradasOcupadas = entradasOcupadas;
		log_trace(logT,"Conexión de %s con %d entradas ocupadas", instancia->nombre,entradasOcupadas);

		return 1;
	}

	instancia= malloc(sizeof(t_instancia));

	instancia->nombre = nombreInstancia;
	instancia->socketInstancia = nuevoSocket;



	instancia->claves = list_create();
	list_add(instancias,instancia);
	enviarInt(instancia->socketInstancia, FIN);
	int entradasOcupadas = 0;
	recibirInt(instancia->socketInstancia,&entradasOcupadas);
	instancia->entradasOcupadas = entradasOcupadas;
	log_trace(logT,"Conexión de %s con %d entradas ocupadas", instancia->nombre,entradasOcupadas);
	return 1;
}

void enviarClavesAGuardar(t_instancia * instancia){
	int size = list_size(instancia->claves);
	int i;
	for(i=0;i<size;i++){
		enviarInt(instancia->socketInstancia, MANTENER_KEY);
		enviarMensaje(instancia->socketInstancia, (char*)list_get(instancia->claves,i));
	}
	enviarInt(instancia->socketInstancia, FIN);
}

bool existeInstancia(char* nombreInstancia, t_instancia ** instancia){
	bool encontro = false;
	void mismo_nombre(void* parametro) {
		t_instancia* inst = (t_instancia*) parametro;
		if( strcmp(inst->nombre,nombreInstancia) == 0){
			*instancia = inst;
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

int enviarKey(char * key, int socket ){
	/*int totalEnviado = 0;
	int lenEnviado = 0;
	while(totalEnviado < LONGITUD_CLAVE) {
		lenEnviado = 0;
		lenEnviado = send(socket, &key[totalEnviado], LONGITUD_CLAVE-totalEnviado, 0);
		if(lenEnviado < 0){ perror("error al enviar"); return -1;}
		totalEnviado = totalEnviado + lenEnviado;
	}
	return totalEnviado;*/
	 return enviarMensaje(socket,key);

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
	infoConfig = config_create("../../Recursos/Configuracion/coordinador.config");
	*/


	/* para correr desde la VM Server */
	infoConfig = config_create("coordinador.config");


	if(config_has_property(infoConfig, "PUERTO_ESCUCHA")){
		coordinador_Puerto_Escucha = config_get_int_value(infoConfig, "PUERTO_ESCUCHA");
	}

	if(config_has_property(infoConfig, "ALGORITMO")){
		coordinador_Algoritmo = string_from_format("%s",config_get_string_value(infoConfig, "ALGORITMO"));
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

	config_destroy(infoConfig);
}


void configureLoggers(){

	T = LOG_LEVEL_TRACE;
	I = LOG_LEVEL_INFO;
	E = LOG_LEVEL_ERROR;


	/* para correr desde ECLIPSE
	//vaciarArchivo("../Recursos/Logs/Coordinador.log");
	logT = log_create("../Recursos/Logs/Coordinador.log", "Coordinador", true, T);
	logI = log_create("../Recursos/Logs/Coordinador.log", "Coordinador", true, I);
	logE = log_create("../Recursos/Logs/Coordinador.log", "Coordinador", true, E);
*/
	/* para correr desde CONSOLA

	vaciarArchivo("../../Recursos/Logs/Coordinador.log");
	logT = log_create("../../Recursos/Logs/Coordinador.log", "Coordinador", true, T);
	logI = log_create("../../Recursos/Logs/Coordinador.log", "Coordinador", true, I);
	logE = log_create("../../Recursos/Logs/Coordinador.log", "Coordinador", true, E);
*/

	/* para correr desde la VM Server*/
	//vaciarArchivo("Coordinador.log");

	logT = log_create("Coordinador.log", "Coordinador", true, T);
	logI = log_create("Coordinador.log", "Coordinador", true, I);
	logE = log_create("Coordinador.log", "Coordinador", true, E);


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

	fprintf(operaciones,"%s\n", linea);

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
	free(nuevoESI);
	return (void*)pthread_detach(pthread_self());
}

void recibirMensajeESI(int socket){

	bool finalizar = false;
	int mensaje;
	while(1){
		char* key;
		char* value;

		if(recibirInt(socket, &mensaje)<=0){
			log_trace(logT, "ESI socket %d desconectado", socket);
			break;
		}

		usleep(retardo*1000); // Según enunciado, a cada instrucción de ESI se le aplica un retardo para "simular el paso en el tiempo en la ejecución"

		switch(mensaje){

		case GET_KEY:;
		if(ejecutarOperacionGET(socket)<0){
			finalizar = true;
			enviarInt(socket, EJECUCION_INVALIDA);
		}
		break;

		case SET_KEY:
			key = recibirMensajeArchivo(socket);
			value = recibirMensajeArchivo(socket);
			if(ejecutar_operacion_set(socket,key,value)<0){
				finalizar = true;
				enviarInt(socket, EJECUCION_INVALIDA);
			}
			break;
		case STORE_KEY:

			if(ejecutar_operacion_store(socket)<0){
				finalizar = true;
				enviarInt(socket, EJECUCION_INVALIDA);
			}
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

		//recibirMensajePlanificador(argsPlanificador->socketPlanificador);
		sleep(5);

	}

}

void* atenderConsolaPlanificador(void *args){

	while(1){

		if(recibirMensajeConsolaPlanificador(argsConsolaPlanificador->socketPlanificador)<=0){
			break;;
		}
	}

	return NULL;

}

int recibirMensajeConsolaPlanificador(int socket){
	int mensaje;

	int len =recibirInt(socket, &mensaje);

	switch(mensaje){

	case DONDE_ESTA_LA_CLAVE:
		procesarDondeEstaLaClave(socket);
		break;

	case OBTENER_VALOR_DE_KEY:
		procesarObtenerValorKey(socket);
		break;

	case CREAR_KEY_INICIALMENTE_BLOQUEADA:;

		char* keyName = recibirMensajeArchivo(socket);
		list_add(claves_sin_instancia, keyName);

		log_trace(logT, "Se crea nueva clave inicialmente bloqueada: %s", keyName);

		break;

	default:

		if(len <= 0){
			log_error(logE, "Planificador Desconectado");
			return -1;
		}

		log_error(logE, "No reconozco ese mensaje %d\n", mensaje);
	}
	return 1;
}

void procesarObtenerValorKey(int socket){
	char* clave;
	int retorno;
	char* keyValue;

	clave = recibirMensajeArchivo(socket);
	retorno = obtenerValue(clave, &keyValue);
	if(retorno == LISTA_INSTANCIAS_VACIA){
		enviarInt(socket,NO_HAY_INSTANCIAS_CONECTADAS);
	}else if(retorno == CLAVE_INEXISTENTE){
		if(key_creada(clave)){
			enviarInt(socket, CLAVE_CREADA);
		} else {
			enviarInt(socket, CLAVE_NO_ENCONTRADA);
		}
	} else {
		enviarInt(socket, CLAVE_ENCONTRADA);
		enviarMensaje(socket, keyValue);
		free(keyValue);
	}

	free(clave);
}

void procesarDondeEstaLaClave(int socket){
	char* clave;
	int retorno;
	t_instancia* instancia;

	clave = recibirMensajeArchivo(socket);
	retorno = buscarInstanciaContenedora(clave,&instancia);
	if(retorno > 0){
		enviarInt(socket, CLAVE_ENCONTRADA);
	}else{
		if(simularBuscarInstanciaContenedora(clave, &instancia) == LISTA_INSTANCIAS_VACIA){
			enviarInt(socket, NO_HAY_INSTANCIAS_CONECTADAS);
			return;
		}
		enviarInt(socket, CLAVE_NO_ENCONTRADA);
	}

	enviarMensaje(socket, instancia->nombre);
}

void recibirMensajePlanificador(int socket){

	int mensaje;

	recibirInt(socket, &mensaje);

	switch(mensaje){

	case CREAR_KEY_INICIALMENTE_BLOQUEADA:;

	char* keyName = recibirMensajeArchivo(socket);
	list_add(claves_sin_instancia, keyName);

	log_trace(logT, "Se crea nueva clave inicialmente bloqueada: %s", keyName);

	break;

	default:

		log_error(logE, "No reconozco ese mensaje: %d", mensaje);

	}

}

/*********** OPERACION GET **************/
int ejecutarOperacionGET(int socket){

	char * clave;
	clave = recibirMensajeArchivo(socket);
	logueaOperacion("GET",clave,"",socket);
	log_trace(logT,"recibi operacion get con la clave %s",clave);
	enviarInt(argsPlanificador->socketPlanificador, GET_KEY);
	enviarMensaje(argsPlanificador->socketPlanificador, clave);

	int codigo;
	if(recibirInt(argsPlanificador->socketPlanificador, &codigo)<=0){
		log_error(logE,"error al recibir status de la clave %s del planificador",codigo);
	}


	t_instancia * instancia;

	switch(codigo){
	log_info(logI, "recibi %d", codigo);
	case CLAVE_OTORGADA:

		if(enviarInt(socket, EJECUCION_OK)<=0){
			log_error(logE,"error de conexion con ESI en socket %d",socket);
			return -1;
		}
		if(buscarInstanciaContenedora(clave,&instancia)<=0){
				list_add(claves_sin_instancia,clave);
		}else{
			free(clave);
		}

		break;

	case CLAVE_BLOQUEADA:
		free(clave);
		if(enviarInt(socket, CLAVE_BLOQUEADA)<=0){
			log_error(logE, "error de conexion con ESI en socket %d",socket);
			return -1;;
		}
		break;
	}
	return 1;
	//todo: validar mensajes de errores hacia ESI de operacion GET}
}


int bloquearKey(char * key){
	log_trace(logT, "clave %s bloqueada", key);
	return 1;
}

int elegirInstancia(t_instancia ** instancia, char * key, bool esSimulacion){
	bool conectada(void* parametro) {
		t_instancia* inst = (t_instancia*) parametro;

		return (inst->socketInstancia > 0);
	}
	t_list * instancias_conectadas = list_filter(instancias,conectada);

	if(list_is_empty(instancias_conectadas)){
		list_destroy(instancias_conectadas);
		return LISTA_INSTANCIAS_VACIA;
	}
	if(strcmp(coordinador_Algoritmo,"EL")==0){
		int proximaPosicion = proxima_posicion_instancia;
		if(proximaPosicion  >= list_size(instancias_conectadas) ){
			proximaPosicion = 0;
		}
		*instancia = list_get(instancias_conectadas,proximaPosicion);
		proximaPosicion++;
		if(!esSimulacion){
			proxima_posicion_instancia = proximaPosicion;
		}
		list_destroy(instancias_conectadas);
		return proximaPosicion;
	}else
		if(strcmp(coordinador_Algoritmo,"LSU")==0){
			list_destroy(instancias_conectadas);
			return ejecutarAlgoritmoLSU(instancia);
		}else
			if(strcmp(coordinador_Algoritmo,"KE")==0){
				char letra = key[0];
				int q = list_size(instancias_conectadas);
				if(q<=0){
					log_trace(logT,"no hay instancias conectadas para almacenar la clave %s", key);
					list_destroy(instancias_conectadas);
					return -1;
				}

				char valor_a = 'a';
				char valor_z = 'z';
				int q_letras = (valor_z-valor_a + 1) / q;
				int resto = (valor_z-valor_a + 1) % q;
				int i = 0;

				if(resto>0){
					q_letras++;
				}


				int valor_final = letra-valor_a;

				for(;i<q;i++){
					int limite_inferior = (q_letras*i);
					int limite_superior = (q_letras*(i+1));
					if(q !=i+1 && valor_final >= limite_inferior && valor_final < limite_superior){
						*instancia = list_get(instancias_conectadas,i);
						break;
					}else
					if( q == i+1){
						*instancia = list_get(instancias_conectadas,i);
					}

				}
				list_destroy(instancias_conectadas);
				return 1;
			}


	return -1;
}

int ejecutarAlgoritmoLSU(t_instancia** instancia){
	int menorCantidadDeEntradas=INT_MAX;
	t_instancia* instanciaIterada;
	int i;
	t_list* instanciasConectadas = obtenerInstanciasConectadas();
	for (i = 0; i < list_size(instanciasConectadas); ++i) {
		instanciaIterada = list_get(instanciasConectadas,i);
		if(instanciaIterada->entradasOcupadas < menorCantidadDeEntradas){
			*instancia = instanciaIterada;
			menorCantidadDeEntradas = instanciaIterada->entradasOcupadas;
		}
	}
	list_destroy(instanciasConectadas);
	return 1;
}

t_list* obtenerInstanciasConectadas(){
	bool estaConectado(void* instancia){
		t_instancia* inst = (t_instancia*) instancia;
		return inst->socketInstancia != -1;
	}
	return list_filter(instancias, estaConectado);
}

int buscarInstanciaContenedora(char * key, t_instancia ** instancia){
	int pos = -1;
	int encontro = 0;
	bool contieneClave(void* parametro) {
		t_instancia* inst = (t_instancia*) parametro;
		encontro = contieneClaveInstancia(inst,key);

		++pos;
		return encontro >0;
	}

	*instancia = (t_instancia *)list_find(instancias,contieneClave);
	return encontro;

}

int simularBuscarInstanciaContenedora(char * key, t_instancia** instancia){
	log_trace(logT, "Comienza simulacion");
	int retorno = elegirInstancia(instancia, key, true);
	log_trace(logT, "Termina simulacion");
	return retorno;
}


int contieneClaveInstancia(t_instancia * instancia, char * key){

	bool contieneClave(void* parametro) {
		char* clave_aux = (char*) parametro;
		return (strcmp(clave_aux,key)==0); // && instancia->socketInstancia != -1;
	}

	t_list* list = list_filter(instancia->claves,contieneClave);
	int size = list_size(list);
	list_destroy(list);
	return size;
}
/**************** FIN OPERACION GET ***************/


/***************** OPERACION SET ******************/
int ejecutar_operacion_set(int socket, char* clave, char * valor){
	t_instancia * instancia;

	logueaOperacion("SET",clave,valor,socket);



	int instancia_encontrada = buscarInstanciaContenedora(clave, &instancia);
	if(instancia_encontrada<=0){

		log_trace(logT,"No se encontro la clave %s en ninguna instancia", clave);
		if(!key_creada(clave)){
			log_trace(logT,"La clave %s no se encuentra creada", clave);
			enviarInt(socket, CLAVE_INEXISTENTE);
			liberar_clave(clave);
			free(valor);
			free(clave);
			return -1;
		}
		if(elegirInstancia(&instancia,clave,false)<=0){
			log_trace(logT,"No se puede almacenar la clave %s en ninguna instancia", clave);
			enviarInt(socket, EJECUCION_INVALIDA);
			liberar_clave(clave);
			free(valor);
			free(clave);
			return -1;
		}
		log_trace(logT,"se eligio la instancia %s para el guardado de clave", instancia->nombre);

	}
	enviarInt(argsPlanificador->socketPlanificador, SET_KEY);
	enviarMensaje(argsPlanificador->socketPlanificador, clave);
	int codigo;
	recibirInt(argsPlanificador->socketPlanificador, &codigo);

	switch(codigo){
	case CLAVE_RESERVADA:
		if(instancia_encontrada>0 && instancia->socketInstancia == -1){
			log_error(logE,"error de comunicacion con la instancia %s al enviar la clave %s",instancia->nombre, clave);
			remover_clave(instancia,clave);
			free(clave);
			return INSTANCIA_DESCONECTADA;
		}

		if(ejecutar_operacion_set_instancia(clave, valor, instancia)<=0){
			if(instancia_encontrada>0){
				log_error(logE,"error de comunicacion con la instancia %s al enviar la clave %s",instancia->nombre, clave);
				remover_clave(instancia,clave);
				free(clave);
				return INSTANCIA_DESCONECTADA;
			}
			return ejecutar_operacion_set(socket, clave, valor);
		}else{

			if(instancia_encontrada<=0){
				list_add(instancia->claves,clave);
			}else{
				free(clave);
			}

			enviarInt(socket, EJECUCION_OK);
		}
		break;
	case CLAVE_NO_RESERVADA:
		free(clave);
		enviarInt(socket, EJECUCION_INVALIDA);
		break;
	case CLAVE_INEXISTENTE:
		free(clave);
		enviarInt(socket, EJECUCION_INVALIDA);
		break;
	}
	free(valor);

	return 1;

}

void actualizarEntradasOcupadas(t_instancia* instancia){
	recibirInt(instancia->socketInstancia, &(instancia->entradasOcupadas));
}

bool key_creada(char * key){
	bool igualClave(void* parametro) {
		char* clave = (char*)parametro;
		return (strcmp(clave,key)==0);
	}

	return list_any_satisfy(claves_sin_instancia,igualClave);
}

int ejecutar_operacion_set_instancia(char * key, char * value, t_instancia * instancia){
	int socket = instancia->socketInstancia;
	enviarInt(socket,ENVIO_ENTRADA);

	int resultado;
	int conecto = recibirInt(socket,&resultado);
	if(conecto<=0 || resultado != ENVIO_ENTRADA){
		instancia->socketInstancia = -1;
		// TODO: eliminar clave
		log_trace(logE,"error de comunicacion con la instancia %s al enviar la clave %s",instancia->nombre, key);
		return INSTANCIA_DESCONECTADA;
	}

	if(resultado<=0){
		instancia->socketInstancia = -1;
		log_trace(logE,"error de comunicacion con la instancia %s al enviar la clave %s",instancia->nombre, key);
		return -1;
	}
	if(enviarKey(key, socket)<=0){
		instancia->socketInstancia = -1;
		log_trace(logE,"error al enviar la clave %s a la instancia %s",key,instancia->nombre);
		return -1;

	}
	if(enviarValue(value,socket)<=0){
		instancia->socketInstancia = -1;
		log_trace(logE,"error al enviar el valor %s de la clave %s a la instancia %s",value,key,instancia->nombre);
		return -1;
	}

	bool continuar_while = true;
	while(continuar_while){
		int retorno;
		recibirInt(socket, &retorno);
		switch(retorno){
			case COMPACTACION:
				informarCompactacion(instancia);
				break;
			case ENTRADAS_OCUPADAS:
				continuar_while = false;
				actualizarEntradasOcupadas(instancia);
				break;
		}
	}


	return 1;
}

void informarCompactacion(t_instancia * instancia){
	bool resto_instancias(void * parametro){
		t_instancia * instancia_aux = (t_instancia*)parametro;
		return (instancia->socketInstancia != instancia_aux->socketInstancia && instancia->socketInstancia != -1);
	}

	t_list * avisar_a = list_filter(instancias,resto_instancias);
	int size = list_size(avisar_a);
	int i;
	for(i = 0; i<size; i++){
		t_instancia * instancia_actual = (t_instancia *) list_get(avisar_a,i);
		enviarInt(instancia_actual->socketInstancia, COMPACTACION);
	}

	free(avisar_a);
}
/*************** FIN OPERACION SET ****************/

/*************** OPERACION STORE *****************/

int ejecutar_operacion_store(int socket){
	t_instancia * instancia;
	char* clave;
	log_trace(logT,"Se recibio solicitud de STORE");
	clave = recibirMensajeArchivo(socket);

	int instancia_encontrada;
	logueaOperacion("STORE",clave,"",socket);
	instancia_encontrada = buscarInstanciaContenedora(clave, &instancia);
	if( instancia_encontrada< 0){
		if(!key_creada(clave)){
			log_trace(logT,"La clave %s no se encuentra creada", clave);
			enviarInt(socket, CLAVE_INEXISTENTE);
			free(clave);
			return -1;
		}else{
			log_trace(logT,"La clave %s no se encuentra seteada", clave);
			free(clave);
			enviarInt(socket, EJECUCION_INVALIDA);
			return -1;
		}
	}
	enviarInt(argsPlanificador->socketPlanificador, STORE_KEY);
	enviarMensaje(argsPlanificador->socketPlanificador, clave);

	int codigo;
	recibirInt(argsPlanificador->socketPlanificador, &codigo);
	switch(codigo){
	case CLAVE_LIBERADA:
		if(instancia_encontrada >= 0 ){
			if(ejecutar_operacion_store_instancia(clave, instancia)<=0){
				enviarInt(socket, ERROR_EJECUCION);
				return -1;
			}
		}else{
			enviarInt(socket, EJECUCION_INVALIDA);
			break;
		}
		actualizarEntradasOcupadas(instancia);
		enviarInt(socket, EJECUCION_OK);
		break;
	case CLAVE_NO_RESERVADA:;
	enviarInt(socket, EJECUCION_INVALIDA);
	break;
	case CLAVE_INEXISTENTE:;
	enviarInt(socket, EJECUCION_INVALIDA);
	break;
	}
	free(clave);
	return 1;
}

int ejecutar_operacion_store_instancia(char * key, t_instancia * instancia){
	int socket = instancia->socketInstancia;
	int resultado;
	enviarInt(socket,STORE_ENTRADA);
	int conecto = recibirInt(socket,&resultado);
	if(conecto<=0 || resultado != ENVIO_ENTRADA){
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

void liberar_clave(char * key){

	bool igualClave(void* parametro) {
		char* clave = (char*)parametro;
		return (strcmp(clave,key)==0);
	}

	char * clave_a_liberar = (char*) list_remove_by_condition(claves_sin_instancia,igualClave);
	if (clave_a_liberar != NULL){
		free(clave_a_liberar);
	}
	log_trace(logT, "se libero la clave %s", key);
}
/*************** FIN OPERACION STORE *****************/


int obtenerValue(char * key, char** value){
	t_instancia * instancia;
	t_list * instancias_conectadas = obtenerInstanciasConectadas();
	if(list_is_empty(instancias_conectadas)){
		list_destroy(instancias_conectadas);
		return LISTA_INSTANCIAS_VACIA;
	}
	list_destroy(instancias_conectadas);
	if(buscarInstanciaContenedora(key, &instancia)<=0){
		log_error(logE,"La clave %s no se encuentra en ninguna instancia",key);
		return CLAVE_INEXISTENTE;
	}

	if(enviarInt(instancia->socketInstancia,OBTENER_VALUE)<=0){
		log_error(logE, "error de conexion con la instancia %s",instancia->nombre);
		instancia->socketInstancia = -1;
	}

	if(enviarMensaje(instancia->socketInstancia,key)<=0){
		log_error(logE, "error de conexion con la instancia %s",instancia->nombre);
		instancia->socketInstancia = -1;
	}

	int resultado;

	if(recibirInt(instancia->socketInstancia, &resultado)<=0){
		log_error(logE, "error al recibir el valor de la clave %s en la instancia %s",key,instancia->nombre);
		return -1;
	}

	switch(resultado){
		case CLAVE_INEXISTENTE:
			log_trace(logT, "la instancia %s no tiene la clave %s",instancia->nombre,key);
			remover_clave(instancia,key);
			break;
		case CLAVE_ENCONTRADA:
			*value = recibirMensajeArchivo(instancia->socketInstancia);

			log_trace(logT, "se recibio el valor %s para la clave %s", *value,key);
			break;

	}

	return resultado;
}


void remover_clave(t_instancia* instancia, char * key){
	bool igualA(void * parametro){
		char* clave_actual = (char*) parametro;
		return(strcmp(key,clave_actual)==0);
	}

	char * clave_borrar = list_remove_by_condition(instancia->claves, igualA);
	free(clave_borrar);
	log_trace(logT, "Se borró la clave %s de la instancia %s.",key, instancia->nombre);
}

/***** La mini consola mágica del Coordinador *****/


void* escucharConsola(){

	char* linea;

	while(1) {
		linea = readline("DR-Coordinador:" );

		if(!strncmp(linea, "exit", 4)) {
			log_trace(logT,"Se finaliza el Coordinador a pedido.");
			free(linea);
			exit_gracefully();
			exit(1);
			break;
		} else if(!strncmp(linea, "clear", 4)) {
				cls();
		} else {
			printf("No se reconoce el comando %s\n",linea);
			printf("El Coordinador sólo acepta ""exit"" o ""clear"".\n");
		}

		free(linea);
	}

	return NULL;
}

void * cls(){
	return (void*) system("clear");
}

/***** Fin mini consola Mágica del Coordinador ******/

void exit_gracefully(){

	void liberarClaves(void * clave){
				free(clave);
	}

	list_destroy_and_destroy_elements(claves_sin_instancia,liberarClaves);

	void liberarInstancias(void* parametro){
		t_instancia * inst = (t_instancia *) parametro;
		free(inst->nombre);
		list_destroy_and_destroy_elements(inst->claves,liberarClaves);
		free(inst);

	}

	pthread_cancel(threadAtencionConsolaPlanificador);
	pthread_detach(threadAtencionConsolaPlanificador);
	pthread_cancel(threadEscucharConsola);
	pthread_detach(threadEscucharConsola);

	free(esperarConexion);
	free(argsPlanificador);
	free(coordinador_Algoritmo);
	free(argsConsolaPlanificador);
	destroyLoggers();
	list_destroy_and_destroy_elements(instancias,liberarInstancias);
	pthread_mutex_destroy(&mx_logOp);

}


