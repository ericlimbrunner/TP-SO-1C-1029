#ifndef LFS_H_
#define LFS_H_

#include <stdio.h> // Por dependencia de readline en algunas distros de linux :)
#include <openssl/md5.h> // Para calcular el MD5
#include <string.h>
#include <stdlib.h> // Para malloc
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include <readline/readline.h> // Para usar readline
#include <readline/history.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <libgen.h>
#include <pthread.h>

#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <commons/txt.h>

#include <bibliotecaFunciones/lfsSerializacion.h>
#include <bibliotecaFunciones/sockets.h>

#include "varios.h"
#include "fs.h"

#define maximoDeConexiones 20

//### CONFIG #################
t_config *archivoConfig;

typedef struct
{
	int puertoEscucha;
	char *puntoMontaje;
	int retardo;
	int tamanioValue;
	int tiempoDump;
}t_ConfigLFS;


//##########################
typedef struct
{
	int socket;
	char nombre[10];
	pthread_t hiloID;

}t_Memoria;

t_list* listaMemorias;
//##########################
typedef struct
{
	char *nombreTabla;
	t_list *registros;
	t_list *temporales; //NOMBRES DE ARCHIVOS .tmp .tmpc
}t_Tabla;

t_list* memTable;

typedef struct
{
	int TIMESTAMP;
	uint16_t KEY;
	char *VALUE;   //char VALUE[configLFS.tamanioValue +1];  //EL COLPILADOR NO TE DEJA  :/
}t_Registro;
//##########################
//##########################

//### VARIABLES GLOBALES ############
t_ConfigLFS configLFS;
t_log *logger;




//PROTOTIPOS DE FUNCIONES
void leerConfigLFS(void);
void mostrarValoresDeConfig(void);
void crearListasGenerales(void);
void destruirListasGenerales(void);
void exitLFS(int);
void iniciarEscucha(void);
int aceptarConexiones(int);
void realizarMultiplexacion(int);
void realizarProtocoloDelPackage(t_PaqueteDeDatos *, int);
void realizarHandshakeAMemoria(t_log *,int , t_PaqueteDeDatos *,char*);
void enviarRespuesta(int socketReceptor, int protocoloID, char *respuesta);

void *asignadorLISSANDRA(void *arg);
void *receptorDePackages(void *unSocketCliente);
void *connection_handler(void *socket_desc);

//###### SEGUN PROTOCOLO #############
char *realizarINSERT(t_INSERT *unINSERT, int socketMemoria);
t_Tabla *existeEnMemtable(char *nombreTabla);
char *realizarCREATE(t_CREATE *unCREATE);

//####################################

void consolaAPI(void);

#endif /* LFS_H_ */