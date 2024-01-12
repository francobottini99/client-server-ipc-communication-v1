/**
 * @file Common.h
 * @author Bottini, Franco Nicolas.
 * @brief Cabezera de elementos comunes al Server y al Cliente.
 * @version 1.0
 * @date Marzo de 2023.
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/types.h>

//Se utiliza para evitar las warnings del compilador producidas por el parametro 'context' sin utilizar en el manejador de señales.
#define UNUSED(x) (void)(x)

//Path de la FIFO creada por el servidor
#define FIFO_NAME "data/.fifo"

//Path del archivo en donde se guarda el PID del servidor para permitir a los clientes consultarlo y conectarse.
#define PID_SERVER_FILE "data/.ipcserverpid"

//Longitud maxima admitida para los mensajes enviados por los clientes
#define MSG_MAX_SIZE 1024

/**
 * Enumerado que define los tipos de señales con las que trabaja el cliente y el servidor.
 */
typedef enum USRSignalType
{
    /**
     * Enviado por un cliente: Solicita inicio de escritura en el servidor.
     * Enviado por el Servidor: Solicitud de inicio de escritura aceptada.
    */
    START_WRITE,

    /**
     * Enviado por un cliente: Finaliza la escritura de un mensaje.
     * Servidor no envia.
    */
    END_WRITE,

    /**
     * Cliente no envia.
     * Enviado por el Servidor: Solicitud de inicio de escritura rechazada. Esperar y volver a intentar.
    */
    WAIT
} USRSignalType;

/**
 * Tipo enumerado que define los tipos de canales sobre que se pueden instanciar clientes.
*/
typedef enum ChannelType
{
    //Cliente que se comunica con el servidor mediante un FIFO.
    FIFO,

    //Cliente que se comunica con el servidor mediante Memoria Compartida.
    SHARED_MEMORY,

    //Cliente que se comunica con el servidor mediante una Cola de Mensajes.
    MESSAGE_QUEUE
} ChannelType;

/**
 * Estructura auxiliar que define un elemento de la cola de mensajes.
*/
typedef struct MsgQueueElemnet
{
    //Valor numérico que indica el tipo de mensaje que se está enviando o recibiendo.
    long type;

    //Cadena de caracteres que contiene el mensaje en sí mismo.
    char msg[MSG_MAX_SIZE];
} MsgQueueElemnet;

#endif //__COMMON_H__