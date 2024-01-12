/**
 * @file Client.h
 * @author Bottini, Franco Nicolas.
 * @brief Cabecera de la implementacion del Cliente IPC.
 * @version 1.0.1
 * @date Marzo de 2023.
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "Common.h"

/**
 * Una estructura que representa un cliente que se conecta a un servidor.
 * Contiene información sobre el canal del cliente, el ID del proceso del servidor, 
 * el ID de la cola de mensajes, un puntero a la memoria compartida,
 * y dos punteros a funciones que inicializan y envían mensajes.
 */
typedef struct Client
{
    // El canal sobre el cual opera el cliente (FIFO, SHARED_MEMORY, MESSAGE_QUEUE).
    ChannelType type;

    // El ID del proceso del servidor al que el cliente se conectará.
    int server_pid;

    // El ID del de la cola de mensajes.
    int msgid;

    // Un puntero a la memoria compartida.
    char* shm;

    // Un puntero a la función que inicializa el cliente.
    void (*init)(void);

    // Un puntero a la función que envía mensajes desde el cliente al servidor.
    void (*send)(const char* msg);
} Client;

/**
 * @brief Imprime en la consola información sobre los argumentos de entrada requeridos. 
 * 
 * Imprime en la consola una descripción detallada sobre los argumentos de entrada que deben ser proporcionados para que el programa pueda ejecutarse correctamente. 
 * En concreto, se debe especificar el tipo de cliente que se desea instanciar: FIFO, Shared Memory o Message Queue. 
 * 
 * @return No devuelve ningún valor.
 */
void print_help(void);

/**
 * @brief Manejador de señales SIGUSR1. 
 * 
 * Si la señal SIGUSR1 es recibida por el proceso, esta función se ejecutará automáticamente.
 * 
 * @param sig El número de señal recibido
 * @param info Puntero a una estructura siginfo_t que contiene información adicional sobre la señal recibida
 * @param context Puntero a un contexto de señal
 * 
 * @return No devuelve ningún valor.
 */
void signal_handler(int sig, siginfo_t *info, void* context);

/**
 * @brief Crea nuevas instancias de Clientes. 
 * 
 * Crea y devuelve un objeto de tipo Client según el valor del parámetro channel_type y server_pid.
 * 
 * @param channel_type canal sobre el cual va a operar el cliente (FIFO, SHARED_MEMORY, MESSAGE_QUEUE).
 * @param server_pid El ID del proceso del servidor al que el cliente se conectará.
 * @return Un puntero a un objeto de tipo Client creado dinámicamente, o NULL si no se reconoce el tipo de cliente.
 */
Client* client_factory(ChannelType channel_type, int server_pid);

/**
 * @brief Inicializa el cliente con los argumentos de entrada especificados. 
 * 
 * Inicializa el cliente con los argumentos de entrada proporcionados en el programa.
 * La función espera que se proporcione un argumento que especifica el tipo de cliente a instanciar, que puede ser FIFO (0), Shared Memory (1) o Message Queue (2).
 * Si se proporciona un número incorrecto de argumentos o un argumento inválido, se imprime un mensaje de error y se finaliza la ejecución del programa.
 * Si no se encuentra un servidor en ejecución, se imprime un mensaje de error y se finaliza la ejecución del programa.
 * 
 * @param argc Número de argumentos proporcionados.
 * @param argv Arreglo de argumentos proporcionados.
 * 
 * @return No devuelve ningun valor.
 */
void client_init(int argc, char* argv[]);

/**
 * @brief Inicializa el manejador de señales para SIGUSR1. 
 *
 * Configura el proceso para que escuche y procese señales del tipo SIGUSR1.
 * 
 * @return No devuelve ningun valor.
 */
void signal_handler_init(void);

/**
 * @brief Inicializa la memoria compartida. 
 * 
 * Permite conectarse a la region de memoria compartida creada por el servidor.
 * Se crea una clave a partir del nombre del archivo del servidor y un carácter arbitrario.
 * A continuación, se utiliza esta clave para obtener el identificador de la región de memoria compartida.
 * Finalmente, se agrega la región de memoria compartida al espacio de memoria del proceso del cliente.
 * 
 * @return No devuelve ningún valor.
 */
void shared_memory_init(void);

/**
 * @brief Inicializa la cola de mensajes. 
 * 
 * Permite conectarse a la cola de mensajes creada por el servidor.
 * Se crea una clave a partir del nombre del archivo del servidor y un carácter arbitrario.
 * A continuación, se utiliza esta clave para obtener el identificador de la cola de mensajes.
 * 
 * @return No devuelve ningún valor.
 */
void message_queue_init(void);

/**
 * @brief Envía una solicitud de envio de mensaje al servidor. 
 * 
 * Envía una señal al servidor solicitando escribir un mensaje y espera una respuesta.
 * Si la conexión no se establece en un plazo de 1 segundo, la función devuelve un valor de 0.
 * Si el servidor está ocupado, la función espera un tiempo aleatorio y de vuelve a intentarlo.
 * 
 * @return Devuelve un valor entero:
 *          - 1 si la conexión se establece con éxito.
 *          - 0 si la conexión no se establece.
 */
int request_send(void);

/**
 * @brief Envia un mensaje al servidor a través de la FIFO.
 *
 * @param msg Un puntero a la cadena de caracteres que representa el mensaje que se enviará al servidor.
 *
 * @return No devuelve ningun valor.
 *
 * @note Se asume que el cliente ya ha sido inicializado y que la variable client es válida.
 * @warning No se garantiza que el servidor haya recibido o procesado el mensaje enviado.
 */
void fifo_send(const char* msg);

/**
 * @brief Envia un mensaje al servidor a través de la SHARED MEMORY.
 *
 * @param msg Un puntero a la cadena de caracteres que representa el mensaje que se enviará al servidor.
 *
 * @return No devuelve ningun valor.
 *
 * @note Se asume que el cliente ya ha sido inicializado y que la variable client es válida.
 * @warning No se garantiza que el servidor haya recibido o procesado el mensaje enviado.
 */
void shared_memory_send(const char* msg);

/**
 * @brief Envia un mensaje al servidor a través de la MESSAGE QUEUE.
 *
 * @param msg Un puntero a la cadena de caracteres que representa el mensaje que se enviará al servidor.
 *
 * @return No devuelve ningun valor.
 *
 * @note Se asume que el cliente ya ha sido inicializado y que la variable client es válida.
 * @warning No se garantiza que el servidor haya recibido o procesado el mensaje enviado.
 */
void message_queue_send(const char* msg);

/**
 * @brief Finaliza la ejecucion del programa. 
 * 
 * @return No devuelve ningun valor.
*/
void end_client(void);

#endif //__CLIENT_H__