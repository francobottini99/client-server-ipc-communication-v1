/**
 * @file ServerUtils.h
 * @author Bottini, Franco Nicolas.
 * @brief Cabecera de funciones auxiliares utilizadas por el Servidor IPC.
 * @version 1.0
 * @date Marzo de 2023.
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __SERVER_UTILS_H__
#define __SERVER_UTILS_H__

#include "Common.h"

//Canal utilizado.
#define LOCK 1

//Canal libre.
#define UNLOCK 0

//Iniciar timer.
#define START 1

//Detener timer.
#define STOP 0

/**
 * @brief Comparte el PID del servidor. 
 * 
 * Guarda el PID del servidor en un archivo para que los clientes lo puedan levantar.
 * 
 * @return No devuelve ningun valor.
*/
void shared_server_pid(void);

/**
 * @brief Calcula y actualiza la tasa de entrada de mensajes. 
 * 
 * La tasa de entrada se calcula a partir del tiempo transcurrido entre los dos ultimos mensajes recibidos.
 * 
 * @return No devuelve ningun valor.
*/
void refresh_message_rate(void);

/**
 * @brief Actualiza y guarda las estadisticas del servidor.
 * 
 * Se debe invocar cada vez que se recibe un nuevo mensaje. Recibe como parametros los datos del mensaje recibido.
 * 
 * @param channel_type tipo de cliente que envio el mensaje.
 * @param msg mensaje recibido.
 * @param timeout indica si la conexion termino en timeout: 0 no hubo timeout. 1 si sucedio un timeout.
 * 
 * @return No devuelve ningun valor.
*/
void refresh_stats(ChannelType channel_type, const char* msg, int timeout);

/**
 * @brief Obtiene/genera el nombre del archivo donde se guardan las estadisticas de ejecucion del servidor.
 * 
 * @return Puntero a la cadena donde se almacena el nombre del archivo.
*/
const char* get_stats_file(void);

/**
 * @brief Configura los timers que se van a utilizar para detectar los timeouts.
 * 
 * @return No devuelve ninfun valor.
*/
void timers_init(void);

/**
 * @brief Determina si un canal IPC esta ocupado.
 * 
 * @param channel_type Canal IPC a testear el estado.
 * 
 * @return 1 en caso de estar ocupado el canal. 0 en caso contrario.
*/
int is_lock_channel(ChannelType channel_type);

/**
 * @brief Iniciar o detener un timer de control de timeout.
 * 
 * @param channel_type Canal a cambiar de estado de su timer.
 * @param state Nuevo estado del timer: 1 inicializa el timer. 0 detiene el timer.
 * 
 * @return No devuelve ningun valor.
*/
void change_timer_state(ChannelType channel_type, int state);

/**
 * @brief Cambia el estado de uso de un canal.
 * 
 * @param channel_type Canal a cambiar el estado de uso.
 * @param state Nuevo estado de uso del canal: 1 ocupar canal. 0 liberar canal.
 * @param pid ID del proceso que ejecuta el cambio de estado.
 * 
 * @return No devuelve ningun valor.
*/
void change_channel_state(ChannelType channel_type, int state, int pid);

/**
 * @brief Obtiene el canal al que pertenece un timer.
 * 
 * @param timer Timer del cual se quiere obtener el canal.
 * 
 * @return canal al cual pertenece el timer.
*/
ChannelType get_timer_type(timer_t timer);

/**
 * @brief Obtiene el ID del proceso que esta ocupando un canal.
 * 
 * @param channel_type Canal del cual se quiere obtener el ID del proceso que lo ocupa.
 * 
 * @return ID del proceso que ocupa el canal. 0 en caso de no estar ocupado el canal.
*/
pid_t get_pid(ChannelType channel_type);

/**
 * @brief Imprime por un determinado output informacion acerca de un mensaje.
 * 
 * @param channel_type Canal por el que se recibio el mensaje.
 * @param msg Mensaje recibido.
 * @param fp File descriptor del archivo de salida.
 * 
 * @return No devuelve ningun valor.
*/
void print_msg_info(ChannelType channel_type, const char* msg, FILE *fp);

/**
 * @brief Imprime por un determinado output informacion acerca de un timeout.
 * 
 * @param channel_type Canal en donde se produjo el timeout.
 * @param fp File descriptor del archivo de salida.
 * 
 * @return No devuelve ningun valor.
*/
void print_msg_timeout(ChannelType channel_type, FILE *fp);

/**
 * @brief Imprime por un determinado output las estadisticas del servidor.
 * 
 * @param fp File descriptor del archivo de salida.
 * 
 * @return No devuelve ningun valor.
*/
void print_stats(FILE *fp);

#endif //__SERVER_UTILS_H__