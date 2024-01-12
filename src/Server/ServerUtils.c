/**
 * @file ServerUtils.c
 * @author Bottini, Franco Nicolas.
 * @brief Implementacion de funciones auxiliares utilizadas por el Server IPC.
 * @version 1.0
 * @date Marzo de 2023.
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "ServerUtils.h"

//Path base del archivo donde se almacenan las estadisticas del servidor.
#define SERVER_STATS_FILE_BASE "data/server_stats_"

/**
 * @struct locks
 * 
 * Estructura para almacenar el estado de bloque de los canales de comunicacion del servidor.
*/
struct
{
    //lock para la FIFO.
    int fifo;

    //lock para la Memoria Compartida.
    int memory_shared;

    //lock para la Cola de Mensajes.
    int message_queue;
} locks;

/**
 * @struct pid_channel_use
 * 
 * Estructura para almacenar los ID de los procesos que ocupan los canales del servidor.
*/
struct
{
    //ID del proceso que esta ocupando la FIFO.
    pid_t fifo;

    //ID del proceso que esta ocupando la Memoria Compartida.
    pid_t memory_shared;

    //ID del proceso que esta ocupando la Cola de Mensajes.
    pid_t message_queue;
} pid_channel_use;

/**
 * @struct timers
 * 
 * Estructura que permite manejar los timers de timeout de los distintos canales.
*/
struct
{
    //Estructura de configuracion comun para todos los timers.
    struct itimerspec its;

    //Timer que maneja el timeout del acceso a la FIFO.
    timer_t fifo;

    //Timer que maneja el timeout del acceso a la Memoria Compartida.
    timer_t memory_shared;

    //Timer que maneja el timeout del acceso a la Cola de Mensajes.
    timer_t message_queue;
} timers;

/**
 * @struct stats
 * 
 * Estructura que almacena las estadisticas de ejecucion del servidor.
*/
struct
{
    //Cantidad de mensajes recibidos por el canal FIFO.
    long fifo;

    //Cantidad de mensajes recibidos por el canal de Memoria Compartida.
    long memory_shared;

    //Cantidad de mensajes recibidos por el canal de Cola de Mensajes.
    long message_queue;

    //Cantidad de conexiones finalizadas en timeout.
    long timeout;

    //Cantidad total de mensajes recibidos + conexiones perdidas por timeout.
    long total;

    //Porcentaje de mensajes recibidos por FIFO del total.
    float fifo_percent;

    //Porcentaje de mensajes recibidos por Memoria Compartida del total.
    float memory_shared_percent;

    //Porcentaje de mensajes recibidos por Cola de Mensajes del total.
    float message_queue_percent;

    //Porcentaje de conexiones finalizadas en timeout del total.
    float timeout_percent;

    //Tasa de mensajes recibidos por segundo.
    float msg_rate;
} stats;

//Array auxiliar para obtener un elemento del enumerado 'ChannelType' en formato de cadena.
const char* ChannelStringType[] = { "FIFO", "SHARED MEMORY", "MESSAGE QUEUE" };

void refresh_message_rate(void)
{
    static int swaper = 0;
    static struct timespec start;
    struct timespec end;
 
    if(!swaper)
        clock_gettime(CLOCK_MONOTONIC, &start);
    else
    {
        clock_gettime(CLOCK_MONOTONIC, &end);

        float frecuency = 1.0f / ((float)(end.tv_sec - start.tv_sec) + (float)(end.tv_nsec - start.tv_nsec) / 1000000000.0f);

        float alpha = 1.0f - (float)exp(-1.0 / (frecuency * 2.0 * 3.14159265358979323846));
        
        stats.msg_rate = alpha * frecuency + (1.0f - alpha) * stats.msg_rate;
    }

    swaper = !swaper;
}

void refresh_stats(ChannelType channel_type, const char* msg, int timeout)
{
    refresh_message_rate();

    stats.total++;

    if(!timeout)
    {
        switch (channel_type)
        {
            case FIFO:        
                stats.fifo++;
                stats.fifo_percent = ((float)stats.fifo / (float)stats.total) * 100.0f;
                break;
    
            case SHARED_MEMORY:
                stats.memory_shared++;
                stats.memory_shared_percent = ((float)stats.memory_shared / (float)stats.total) * 100.0f;
                break;
    
            case MESSAGE_QUEUE:;
                stats.message_queue++;
                stats.message_queue_percent = ((float)stats.message_queue / (float)stats.total) * 100.0f;
                break;
    
            default:
                break;
        }

        print_msg_info(channel_type, msg, stdout);
        print_stats(stdout);

        FILE *fp = fopen(get_stats_file(), "w");

        print_stats(fp);

        fclose(fp);
    }
    else
    {
        stats.timeout++;
        stats.timeout_percent = ((float)stats.timeout / (float)stats.total) * 100.0f;

        print_msg_timeout(channel_type, stdout);
        print_stats(stdout);
    
        FILE *fp = fopen(get_stats_file(), "w");
        
        print_stats(fp);
    
        fclose(fp);
    }
}

const char* get_stats_file(void)
{
    static char file[256];

    if(strlen(file) > 0)
        return file;

    char pid[11];
    char datetime[20];
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    
    strftime(datetime, sizeof(datetime), "%Y%m%d%H%M%S", tm_now);
    sprintf(pid, "%d", getpid());

    strcpy(file, SERVER_STATS_FILE_BASE);
    strcat(file, pid);
    strcat(file, "_");
    strcat(file, datetime);
    strcat(file, ".txt");

    return file;
}

void shared_server_pid(void)
{
    FILE *fp;

    char aux[11];

    sprintf(aux, "%d", getpid());

    fp = fopen(PID_SERVER_FILE, "w");

    fprintf(fp, "%s", aux);
    
    fclose(fp);
}

void timers_init(void)
{
    struct sigevent sev =
    {
        .sigev_notify = SIGEV_SIGNAL,
        .sigev_signo = SIGRTMIN,
    };

    sev.sigev_value.sival_ptr = &timers.fifo;
    timer_create(CLOCK_MONOTONIC, &sev, &timers.fifo);
    
    sev.sigev_value.sival_ptr = &timers.memory_shared;
    timer_create(CLOCK_MONOTONIC, &sev, &timers.memory_shared);

    sev.sigev_value.sival_ptr = &timers.message_queue;
    timer_create(CLOCK_MONOTONIC, &sev, &timers.message_queue);

    timers.its.it_value.tv_sec = 0;
    timers.its.it_interval.tv_sec = 0;
    timers.its.it_interval.tv_nsec = 0;
}

int is_lock_channel(ChannelType channel_type)
{
    switch (channel_type)
    {
        case FIFO:
            return locks.fifo;

        case SHARED_MEMORY:
            return locks.memory_shared;

        case MESSAGE_QUEUE:
            return locks.message_queue;

        default:
            return 0;
    }   
}

ChannelType get_timer_type(timer_t timer)
{
    if (timer == timers.fifo)
        return FIFO;
    else if (timer == timers.memory_shared)
        return SHARED_MEMORY;
    else if (timer == timers.message_queue)
        return MESSAGE_QUEUE;
    else
    {
        fprintf(stderr, "\033[1;31mTipo de cliente invalido\033[0m\n");
        exit(EXIT_FAILURE);
    }
}

pid_t get_pid(ChannelType channel_type)
{
    switch (channel_type)
    {
        case FIFO:
            return pid_channel_use.fifo;

        case SHARED_MEMORY:
            return pid_channel_use.memory_shared;

        case MESSAGE_QUEUE:
            return pid_channel_use.message_queue;

        default:
            return 0;
    }   
}

void change_channel_state(ChannelType channel_type, int state, int pid)
{
    switch (channel_type)
    {
        case FIFO:
            locks.fifo = state;
            pid_channel_use.fifo = (state == UNLOCK) ? 0 : pid;
            break;

        case SHARED_MEMORY:
            locks.memory_shared = state;
            pid_channel_use.memory_shared = (state == UNLOCK) ? 0 : pid;
            break;

        case MESSAGE_QUEUE:
            locks.message_queue = state;
            pid_channel_use.message_queue = (state == UNLOCK) ? 0 : pid;
            break;

        default:
            break;
    }
}

void change_timer_state(ChannelType channel_type, int state)
{
    timers.its.it_value.tv_nsec = (state == START) ? 10000000 : 0;

    switch (channel_type)
    {
        case FIFO:
            timer_settime(timers.fifo, 0, &timers.its, NULL);
            break;

        case SHARED_MEMORY:
            timer_settime(timers.memory_shared, 0, &timers.its, NULL);
            break;

        case MESSAGE_QUEUE:
            timer_settime(timers.message_queue, 0, &timers.its, NULL);
            break;

        default:
            break;
    }  
}

void print_msg_info(ChannelType channel_type, const char* msg, FILE *fp)
{
    int pid = get_pid(channel_type);

    if(fp == stdout)
        fprintf(fp, "\033[1;32m");

    fprintf(fp, "\nMensaje Recibido ! -> Cliente %s (%d) -> MSG: %s\n", ChannelStringType[channel_type], pid, msg);

    if(fp == stdout)
        fprintf(fp, "\033[0m");
}

void print_msg_timeout(ChannelType channel_type, FILE *fp)
{
    if(fp == stdout)
        fprintf(fp, "\033[1;31m");

    fprintf(fp, "\nTimeout ! -> Cliente %s (%d)\n", ChannelStringType[channel_type], get_pid(channel_type));

    if(fp == stdout)
        fprintf(fp, "\033[0m");
}

void print_stats(FILE *fp)
{
    if(fp == stdout)
        fprintf(fp, "\x1b[36m");

    fprintf(fp, "\n");
    fprintf(fp, "FIFO           : %ld (%.2f %%)\n", stats.fifo, stats.fifo_percent);
    fprintf(fp, "SHARED MEMORY  : %ld (%.2f %%)\n", stats.memory_shared, stats.memory_shared_percent);
    fprintf(fp, "MESSAGE QUEUE  : %ld (%.2f %%)\n", stats.message_queue, stats.message_queue_percent);
    fprintf(fp, "TOTAL          : %ld\n", stats.total);
    fprintf(fp, "\n");
    fprintf(fp, "TIMEOUT        : %ld (%.2f %%)\n", stats.timeout, stats.timeout_percent);

    if(fp == stdout)
    {
        fprintf(fp, "\n");
        fprintf(fp, "MESSAGE RARTE  : %.2f m/s\n", stats.msg_rate);
    }

    fprintf(fp, "\n");
    
    if(fp == stdout)
        fprintf(fp, "\033[0m");
}