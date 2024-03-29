#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "configKernel.h"
#include <bibliotecaFunciones/sockets.h>
#include "consolaKernel.h"
#include "planificador.h"
#include "kernel_commons.h"
#include "socketsKernel.h"

char* ipMemoria;

void iniciar();
void crear_hilos_iniciales();
void setear_path_config(int nroPrueba);
char* chequearPath (int nroPrueba);


#endif /* KERNEL_H_ */
