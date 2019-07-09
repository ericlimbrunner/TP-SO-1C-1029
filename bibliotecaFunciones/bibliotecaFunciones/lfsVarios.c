#include "lfsVarios.h"

char **splitDeOperaciones(char *unaLineaReadline)
{
	//char *linea = string_from_format("%s",unaLineaReadline);
	char **operaciones;
	int ini,fin;
	int i;
	int top;

	ini = 0;
	fin = 0;

	top = strlen(unaLineaReadline);

	for(i=0; i < top ;i++)
	{
		if(unaLineaReadline[i] == '"' && ini == fin)
			ini = i;

		if(unaLineaReadline[i] == '"' && i != ini)
			fin = i;
	}

	if(ini != fin)
	{
		for(;ini <= fin;ini++)
		{
			if(unaLineaReadline[ini] == '"')
				unaLineaReadline[ini] = ' ';
			else if(unaLineaReadline[ini] == ' ')
				unaLineaReadline[ini] = ';';
		}

		operaciones = string_split(unaLineaReadline," ");

		top = strlen(operaciones[3]);
		for(i=0; i < top;i++)
		{
			if(operaciones[3][i] == ';')
				operaciones[3][i] = ' ';
		}

	}
	else
		operaciones = string_split(unaLineaReadline," ");

	return operaciones;
}

void enviarRespuesta(int socketReceptor, int protocoloID, char *respuesta)
{
	char* stringSerializado;
	t_UnString *unString = definirT_UnString(respuesta);


	//printf("\nRespuesta definida:\n	longString: %d\n	String: %s\n",unString->longString,unString->String);///

	stringSerializado = serializarT_UnString(unString);

	//printf("\nstrlen(pathSerializado): %d\npathSerializado: %s\n",strlen(pathSerializado),pathSerializado);

	//printf("empaquetarEnviarMensaje():\n"); ///

	//ESTO VARIA SEGUN LA STRUCTURA Q PIENSO MANDAR, hay q hacerlo a mano
	int tamanioStructSerializado = sizeof(uint32_t) + unString->longString;

	empaquetarEnviarMensaje(socketReceptor,protocoloID,tamanioStructSerializado,stringSerializado);

	free(stringSerializado);
	freeT_UnString(unString);

	//printf("-----------------------------------------------\n"); ///
}



