#include "planificador.h"

int main(){
	configureLogger();
	cargar_configuracion();

	int socketEscucha;
	fd_set fdSocketsEscucha;

	inicializarColas();
	inicializarSemaforos();

	FD_ZERO(&fdSocketsEscucha);
	socketEscucha = escuchar(planificador_Puerto_Escucha);

	FD_ZERO(&fdConexiones);

	FD_SET(socketEscucha, &fdSocketsEscucha);

	conectarCoordinador();

	pthread_t threadEscucharConsola;
	pthread_t threadPlanificar;
	pthread_t threadConexionesNuevas;
	pthread_t threadCoordinador;
	t_esperar_conexion *esperarConexion;

	esperarConexion = malloc(sizeof(t_esperar_conexion));
	esperarConexion->fdSocketEscucha = fdSocketsEscucha;
	esperarConexion->socketEscucha = socketEscucha;

	int er1 = pthread_create(&threadEscucharConsola,NULL,iniciaConsola,NULL);
	int er2 = pthread_create(&threadPlanificar, NULL,planificar, NULL);
	int er3 = pthread_create(&threadConexionesNuevas, NULL,esperarConexionesESIs,(void*) esperarConexion);
	int er4 = pthread_create(&threadCoordinador, NULL,escucharCoordinador,NULL);

	pthread_join(threadEscucharConsola, NULL);
	pthread_join(threadPlanificar, NULL);
	pthread_join(threadConexionesNuevas, NULL);
	pthread_join(threadCoordinador, NULL);

	free(esperarConexion);
	return 0;
}

void inicializarColas(){
	colaListos = queue_create();
	colaTerminados = queue_create();
}

void inicializarSemaforos(){
	pausarPlanificacion = false;
	sem_init(pausarPlanificacionSem, 0, 1);
}

void configureLogger(){

	LogL = LOG_LEVEL_TRACE;
	logPlan = log_create("../Logs/planificador.log","Planificador", false, LogL);

}

void * iniciaConsola(){

	log_trace(logPlan,"Se inicializa la consola del planificador. \n");

	//comandos de consola
	char* exit = "exit";
	char* pausar = "pausar";
	char* continuar = "continuar";
	char* bloquear = "bloquear";
	char* desbloquear = "desbloquear";
	char* listar = "listar";
	char* status = "status";
	char* kill = "kill";
	char* deadlock = "deadlock";
	char* info = "info";

	char* linea;
	char** parametros;

	while(1) {
		linea = readline("DR-Planificador:" );

		if(linea)
			add_history(linea);

		if(!strncmp(linea, exit, strlen(exit))) {
			log_trace(logPlan,"Consola recibe ""%s""\n", exit);
			parametros = string_split(linea, " ");
			if(parametros[1] != NULL){
				printf("La funcion no lleva argumentos.");
			}
			free(linea);
			exit_gracefully(1);
		} else if(!strncmp(linea, pausar, strlen(pausar)))
		{
			log_trace(logPlan,"Consola recibe ""%s""\n", pausar);
			parametros = string_split(linea, " ");
			if(parametros[1] != NULL){
				printf("La funcion no lleva argumentos.");
			}
			pauseScheduler();
			free(linea);

		} else if(!strncmp(linea, continuar, strlen(continuar)))
		{
			log_trace(logPlan,"Consola recibe ""%s""\n", continuar);
			parametros = string_split(linea, " ");
			if(parametros[1] != NULL){
				printf("La funcion no lleva argumentos.");
			}
			goOn();
			free(linea);

		} else if(!strncmp(linea, bloquear, strlen(bloquear)))
		{
			log_trace(logPlan,"Consola recibe ""%s""\n", bloquear);
			parametros = string_split(linea, " ");
			if(parametros[1] == NULL){
				printf("Faltan argumentos: bloquear [clave] [ID]\n");
			}else{
				if (parametros[2] == NULL){
					printf("Faltan argumentos: bloquear [clave] [ID]\n");
				}else{
					if(parametros[3] != NULL){
						printf("Demasiados argumentos: bloquear [clave] [ID]\n");
					}else{
						char* key = parametros[1];
						int ESI_ID = atoi(parametros[2]);
						block(key, ESI_ID);
					}
				}
			}

		} else if(!strncmp(linea, desbloquear, strlen(desbloquear)))
		{
			log_trace(logPlan,"Consola recibe ""%s""\n", desbloquear);
			parametros = string_split(linea, " ");
			if(parametros[1] == NULL){
				printf("Faltan argumentos: desbloquear [clave]\n");
			}else{
				if(parametros[2] != NULL){
					printf("Demasiados argumentos: desbloquear [clave]\n");
				}else{
					char* key = parametros[1];
					unblock(key);
				}
			}
			free(linea);

		} else if(!strncmp(linea, listar, strlen(listar)))
		{
			log_trace(logPlan,"Consola recibe ""%s""\n", listar);
			if(parametros[1] == NULL){
				printf("Faltan argumentos: listar [clave]\n");
			} else if(parametros[2] != NULL){
				printf("Demasiados argumentos: listar [clave]\n");
			}else{
				char* key = parametros[1];
				listBlockedProcesses(key);
			}
			free(linea);

		} else if(!strncmp(linea, status, strlen(status)))
		{
			log_trace(logPlan,"Consola recibe ""%s""\n", status);
			parametros = string_split(linea, " ");
			if(parametros[1] == NULL){
				printf("Faltan argumentos: status [clave]\n");
			}else{
				if(parametros[2] != NULL){
					printf("Demasiados argumentos: status [clave]\n");
				}else{
					char* key = parametros[1];
					getStatus(key);
				}
			}
			free(linea);

		} else if(!strncmp(linea, kill, strlen(kill)))
		{
			log_trace(logPlan,"Consola recibe ""%s""\n", kill);
			parametros = string_split(linea, " ");
			if(parametros[1] == NULL){
				printf("Faltan argumentos: kill [id]\n");
			}else if(parametros[2] != NULL){
				printf("Demasiados argumentos: kill [id]\n");
			}else{
				int ESI_ID = atoi(parametros[1]);
				matarProceso(ESI_ID);
			}
			free(linea);

		} else if(!strncmp(linea, deadlock, strlen(deadlock)))
		{
			log_trace(logPlan,"Consola recibe ""%s""\n", deadlock);
			free(linea);

		} else if(!strncmp(linea, info, strlen(info)))
		{
			log_trace(logPlan,"Consola recibe ""%s""\n", info);
			printf("Destino-Rusia Planificador: Ayuda\n");
			printf("Los parámetros se indican con <> \n------\n");
			printf("pausa - Pausar planificación : El Planificador no le dará nuevas órdenes de ejecución a ningún ESI mientras se encuentre pausado.\n");
			printf("continuar - Continuar planificación : Reanuda el Planificador.\n");
			printf("bloquear <clave> <ID>: Se bloqueará el proceso ESI hasta ser desbloqueado, especificado por dicho <ID> en la cola del recurso <clave>..\n");
			printf("desbloquear <clave>: Se desbloqueara el proceso ESI con el ID especificado. Solo se bloqueará ESIs que fueron bloqueados con la consola. Si un ESI está bloqueado esperando un recurso, no podrá ser desbloqueado de esta forma.\n");
			printf("listar <recurso>: Lista los procesos encolados esperando al recurso.\n");
			printf("kill <ID>: finaliza el proceso.\n");
			printf("deadlock: Analiza los deadlocks que existan en el sistema y a que ESI están asociados.\n");
			printf("exit - Sale del programa.\n");
			free(linea);
		}
		else {
			printf("No se reconoce el comando ""%s""\n",linea);
			printf("Para más información utilice el comando ""%s"".\n", info);
			free(linea);
		}

	}
}

void cargar_configuracion(){

	t_config* infoConfig;

	infoConfig = config_create("../Configuracion/planificador.config");

	if(config_has_property(infoConfig, "PUERTO_ESCUCHA")){
		planificador_Puerto_Escucha = config_get_int_value(infoConfig, "PUERTO_ESCUCHA");
	}

	if(config_has_property(infoConfig, "ALGORITMO")){
		planificador = config_get_string_value(infoConfig, "ALGORITMO");

		if(strcmp(planificador, "SJF_SIN_DESALOJO") == 0){
			planificador_Algoritmo = SJF_SIN_DESALOJO;
		}
		if(strcmp(planificador, "SJF_CON_DESALOJO") == 0){
			planificador_Algoritmo = SJF_CON_DESALOJO;
		}
		if(strcmp(planificador, "HRRN") == 0){
			planificador_Algoritmo = HRRN;
		}
	}

	if(config_has_property(infoConfig, "ESTIMACION_INICIAL")){
		estimacion_inicial = config_get_int_value(infoConfig, "ESTIMACION_INICIAL") / 100;
	}

	if(config_has_property(infoConfig, "IP_COORDINADOR")){
		coordinador_IP = config_get_string_value(infoConfig, "IP_COORDINADOR");
	}

	if(config_has_property(infoConfig, "PUERTO_COORDINADOR")){
		coordinador_Puerto = config_get_int_value(infoConfig, "PUERTO_COORDINADOR");
	}

	if(config_has_property(infoConfig, "CLAVES_INI_BLOQUEADAS")){
		claves_Ini_Bloqueadas = config_get_array_value(infoConfig, "CLAVES_INI_BLOQUEADAS");
	}

}

void exit_gracefully(int return_nr) {
	log_destroy(logPlan);
	exit(return_nr);
}
