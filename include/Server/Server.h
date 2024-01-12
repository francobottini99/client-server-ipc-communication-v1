/**
 * @file Server.h
 * @author Bottini, Franco Nicolas.
 * @brief Cabecera de la implementacion del Server IPC.
 * @version 1.0
 * @date Marzo de 2023.
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __SERVER_H__
#define __SERVER_H__

#include "ServerUtils.h"

/**
 * @brief Manejador de señales del server.
 *
 * @param sig El número de la señal recibida.
 * @param info Estructura siginfo_t que contiene información adicional sobre la señal.
 * @param context Contexto de ejecución en el momento en que se recibió la señal.
 * 
 * @return No devuelve ningun valor.
 */
void signal_handler(int sig, siginfo_t *info, void* context);

/**
 * @brief Inicializa el manejador de señales del server.
 *
 * Configura el manejador de señales para el servidor. Registra la función
 * signal_handler como manejador de señales para SIGUSR1, SIGRTMIN, SIGTERM, SIGINT y SIGHUP.
 * 
 * @return No devuelve ningun valor.
 */
void signal_handler_init(void);

/**
 * @brief Crea la FIFO del servidor. 
 *
 * Si la creación de la FIFO falla, la función muestra un mensaje de error y termina el programa.
 *
 * @return No devuelve ningun valor.
 */
void create_fifo(void);

/**
 * @brief Crea el segmento de memoria compartida del servidor. 
 *
 * Si la creación o la asignación del segmento de memoria compartida fallan, la función muestra un mensaje de error y termina el programa.
 *
 * @return No devuelve ningun valor.
 */
void create_shared_memory_segment(void);

/**
 * @brief Crea la cola de mensages del servidor. 
 *
 * Si la creación de la col de mensajes falla, la función muestra un mensaje de error y termina el programa.
 *
 * @return No devuelve ningun valor.
 */
void create_message_queue(void);

/**
 * @brief Recibe los mensajes de los diferentes tipos de clientes.
 * 
 * Muestra los mensajes y actualiza la estadistica.
 * 
 * @param channel_type Canal sobre el cual se envía el mensaje.
 * 
 * @return No devuelve ningun valor.
 */
void recibe_msg(ChannelType channel_type);

/**
 * @brief Finaliza la ejecucion del programa. 
 * 
 * Elimina todos los mecanismos de IPC creados y termina la ejecucion del servidor. Tambien elimina el archivo con el PID del server.
 * 
 * @return No devuelve ningun valor.
*/
void end_server(void);

#endif //__SERVER_H__