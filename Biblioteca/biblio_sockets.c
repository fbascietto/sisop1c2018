#include "biblio_sockets.h"


/*
 * Recibe un mensaje y lo envia al socketDestino
 * Retorna los bytes enviados o -1 si hubo un error
 */
int enviarMensaje(int socketDestino, char* mensaje) {
	int totalEnviado=0, bytesRestantes, lenEnviado;

	int len = strlen(mensaje);
	enviarInt(socketDestino, len);
	bytesRestantes = len;
	while(totalEnviado < len) {
		lenEnviado = send(socketDestino, mensaje, len, 0);
		if(lenEnviado == -1){ perror("error al enviar\n"); return -1;}
		totalEnviado = totalEnviado + lenEnviado;
		bytesRestantes = bytesRestantes - lenEnviado;
	}
	return lenEnviado;
}

/*
 * Recibe un mensaje de socketDestino
 * y devuelve un char* con el mensaje
 * Retorna en el char* "-1" en caso de error
 */
char *recibirMensajeArchivo(int socketDestino) {

	/*
	 * size es el tamaño que previamente me envio socketDestino, y mensaje tiene ese tamaño
	 * este valor se va a decrementar hasta que sea 0
	 * es decir, que no quede nada mas para leer
	 * en ese caso, se cambia llegoTodo a 1
	 */
	int size;
	recibirInt(socketDestino, &size);

	int largoLeido = 0, llegoTodo = 0, totalLeido = 0, tamanio = size;
	void *mensaje = malloc(size+1); // +1 para el \0

	while(!llegoTodo){
		largoLeido = recv(socketDestino, mensaje + totalLeido, size, 0);

		//toda esta fumada es para cuando algun cliente se desconecta.
		if(largoLeido == -1){
			printf("Socket dice: Cliente en socket N° %d se desconecto\n", socketDestino);
			mensaje = "-1";
			return (mensaje);
		}

		//si el largo leido es menor que el size, me quedan cosas por leer, sino llego el mensaje completo
		//se actualiza totalLeido y size para seguir haciendo recv hasta que size sea 0
		totalLeido += largoLeido;
		size -= largoLeido;
		if(size <= 0) llegoTodo = 1;
	}

	char* contenido = (char*) mensaje;
	contenido[tamanio] = '\0';
	return contenido;
	//TODO no se si poner free(mensaje) ya que se asigna directamente a un puntero;
}

/*
 * Recibe un puerto y se queda esperando una conexion
 * a dicho puerto.
 *
 * Retorna -1 en caso de error
 * Si recibe una conexion, retorna el socket escuchado
 */
int escuchar(int puerto) {
	int socketEscucha;
	struct sockaddr_in address;
	if ((socketEscucha = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		printf("Error al crear el socket\n");
		return -1;
	}
	int activador = 1;
	if (setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, (char *) &activador,
			sizeof(activador)) < 0) {
		printf("Error al setear socketopt\n");
		return -1;
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(puerto);

	if (bind(socketEscucha, (struct sockaddr *) &address, sizeof(address))
			< 0) {
		printf("Error al bindear\n");
		return -1;
	}

	if (listen(socketEscucha, MAX_CLIENTES) < 0) {
		printf("Error al escuchar\n");
		return -1;
	}
	return socketEscucha;
}


/*
 * Dado un ip y un puerto, se conecta al server
 * Retorna 0 en caso de no poder conectarse
 * Retorna el id del socket propio en caso de conectarse
 *
 */
int conectarseA(char *ip, int puerto) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(ip);
	direccionServidor.sin_port = htons(puerto);
	int cliente = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(cliente, (void*) &direccionServidor, sizeof(direccionServidor))
			!= 0) {
		perror("No se pudo conectar\n");
		return 0;
	}
	return cliente;
}

int aceptarConexion(int socketEscucha) {
	int nuevoSocket;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	if ((nuevoSocket = accept(socketEscucha, (struct sockaddr *) &address,
			(socklen_t*) &addrlen)) < 0) {
		return 1;
	}
	return nuevoSocket;
}


//dado un set y un socket de escucha, verifica mediante select, si hay alguna conexion nueva para aceptar
int esperarConexionesSocket(fd_set *master, int socketEscucha) {
	int nuevoSocket = -1;
	fd_set readSet;
	FD_ZERO(&readSet);
	readSet = *(master);
	if (select(socketEscucha + 1, &readSet, NULL, NULL, NULL) == -1) {
		perror("select");
		exit(4);
	}
	if (FD_ISSET(socketEscucha, &readSet)) {
		// handle new connections
		nuevoSocket = aceptarConexion(socketEscucha);
	}
	return nuevoSocket;
}

/*
 * Permite el envio de un int
 * Devuelve los bytes enviados (equivalente a sizeof(int))
 */
int enviarInt(int socketDestino, int num){
	int status;
	void* bufferEnviarInt = malloc(sizeof(int));
	memcpy(bufferEnviarInt,&num,sizeof(int));
	status = send(socketDestino,bufferEnviarInt,sizeof(int),0);
	free(bufferEnviarInt);
	return status;
}

/*
 * Permite recibir un int
 * Devuelve los bytes recibidos (equivalente a sizeof(int))
 * y el valor se actualiza en *i
 */
int recibirInt(int socketDestino, int* i) {
	int len_leida;
	len_leida = recv(socketDestino, i, sizeof(int), 0);
	return len_leida;
}

