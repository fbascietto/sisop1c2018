#include "planificador.h"
#include "funcionesplanificador.h"

int main(){
	configureLogger();
	cargar_configuracion();

	int socketEscucha;
	fd_set fdSocketsEscucha;

	inicializarColas();

	FD_ZERO(&fdSocketsEscucha);
	socketEscucha = escuchar(planificador_Puerto_Escucha);

	FD_ZERO(&fdConexiones);

	FD_SET(socketEscucha, &fdSocketsEscucha);

	int socketCoordinador = conectarCoordinador();

	pthread_t threadEscucharConsola;
	pthread_t threadPlanificar;
	pthread_t threadConexionesNuevas;
	t_esperar_conexion *esperarConexion;

	esperarConexion = malloc(sizeof(t_esperar_conexion));
	esperarConexion->fdSocketEscucha = fdSocketsEscucha;
	esperarConexion->socketEscucha = socketEscucha;

	/*Al comenzar el socket coordinador es nuestro primer cliente,
	  por lo tanto, tiene el numero de socket mas grande.
	 */
	fdMaxConexionesActivas = socketCoordinador;
	FD_SET(socketCoordinador, &fdConexiones);

	int er1 = pthread_create(&threadEscucharConsola,NULL,iniciaConsola,NULL);
	int er2 = pthread_create(&threadPlanificar, NULL,planificar,(void*) esperarConexion);
	int er3 = pthread_create(&threadConexionesNuevas, NULL,esperarConexionesESIs,(void*) esperarConexion);

	pthread_join(threadEscucharConsola, NULL);
	pthread_join(planificar, NULL);
	pthread_join(esperarConexionesESIs, NULL);

	free(esperarConexion);
	return 0;
}

void inicializarColas(){
	colaListos = queue_create();
	colaTerminados = queue_create();
}

void configureLogger(){

	  LogL = LOG_LEVEL_TRACE;
	  logPlan = log_create("../Logs/planificador.log","Planificador", false, LogL);

}

void * iniciaConsola(){

	log_trace(logPlan,"Se inicializa la consola del planificador. \n");

	while(1) {
		char* linea = readline("DR-Planificador:" );

		if(linea)
			add_history(linea);

		if(!strncmp(linea, "exit", 4)) {
			log_trace(logPlan,"Consola recibe ""exit""");
			free(linea);
			exit_gracefully(1);
		} else if(!strncmp(linea, "pausa", 5))
		{
			log_trace(logPlan,"Consola recibe ""pausa""");
			free(linea);

		} else if(!strncmp(linea, "continuar", 9))
		{
			log_trace(logPlan,"Consola recibe ""continuar""");
			free(linea);

		} else if(!strncmp(linea, "bloquear", 8))
		{
			log_trace(logPlan,"Consola recibe ""bloquear""");
			free(linea);

		} else if(!strncmp(linea, "desbloquear", 11))
		{
			log_trace(logPlan,"Consola recibe ""desbloquear""");
			free(linea);

		} else if(!strncmp(linea, "listar", 6))
		{
			log_trace(logPlan,"Consola recibe ""listar""");
			free(linea);

		} else if(!strncmp(linea, "status", 6))
		{
			log_trace(logPlan,"Consola recibe ""status""");
			free(linea);

		} else if(!strncmp(linea, "kill", 4))
		{
			log_trace(logPlan,"Consola recibe ""kill""");
			free(linea);

		} else if(!strncmp(linea, "deadlock", 8))
		{
			log_trace(logPlan,"Consola recibe ""deadlock""");
			free(linea);

		} else if(!strncmp(linea, "info", 4))
		{
			log_trace(logPlan,"Consola recibe ""info""\n");
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
			printf("Para más información utilice el comando ""info"".\n");
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
		planificador_Algoritmo = config_get_string_value(infoConfig, "ALGORITMO");
	}

	if(config_has_property(infoConfig, "ESTIMACION_INICIAL")){
		estimacion_inicial = config_get_int_value(infoConfig, "ESTIMACION_INICIAL");
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
