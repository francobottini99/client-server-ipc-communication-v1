/**
 * @file Server.c
 * @author Bottini, Franco Nicolas.
 * @brief Implementacion del Server IPC.
 * @version 1.0
 * @date Marzo de 2023.
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "Server.h"

/**
 * @struct fifo
 * 
 * Estructura para almacenar un archivo FIFO y un buffer para leer datos en él.
 */
struct
{
    // File descriptor del archivo FIFO.
    int fd;

    //Buffer para leer datos del archivo FIFO.
    char buffer[MSG_MAX_SIZE];
} fifo;


/**
 * @struct shm
 * 
 * Estructura para representar una región de memoria compartida.
 */
struct 
{
    //Identificador del segmento de memoria compartida.
    int shmid;

    //Puntero al segmento de memoria compartida.
    char *shm_ptr;
} shm;

/**
 * @struct msgqueue
 * 
 * Estructura para representar una cola de mensajes.
 */
struct
{
    //Identificador de la cola de mensajes.
    int id;

    //Elemento de cola de mensajes que almacena el mensaje actual.
    MsgQueueElemnet buffer;
} msgqueue;

void signal_handler(int sig, siginfo_t *info, void* context)
{
    UNUSED(context);

    if (sig == SIGUSR1)
    {
        ChannelType channel_type = (ChannelType)info->si_value.sival_int & 3;
        USRSignalType signal_type = (USRSignalType)(info->si_value.sival_int & ~3) >> 2;

        if (signal_type == END_WRITE)
        {
            if(is_lock_channel(channel_type))
            {
                recibe_msg(channel_type);
                change_channel_state(channel_type, UNLOCK, info->si_pid);
                change_timer_state(channel_type, STOP);
            }
        }
        else if (signal_type == START_WRITE)
        {
            USRSignalType response;

            if(is_lock_channel(channel_type))
                response = WAIT;
            else
            {
                response = START_WRITE;
                change_channel_state(channel_type, LOCK, info->si_pid);
                change_timer_state(channel_type, START);
            }

            sigqueue(info->si_pid, SIGUSR1, (union sigval) { .sival_int = (int)response });
        }
    }
    else if (sig == SIGRTMIN)
    {
        ChannelType channel_type = get_timer_type(*(timer_t*)info->si_value.sival_ptr);
        pid_t pid = get_pid(channel_type);

        refresh_stats(channel_type, NULL, 1);

        change_channel_state(channel_type, UNLOCK, pid);

        change_timer_state(channel_type, STOP);
    }
    else if(sig == SIGTERM || sig == SIGINT || sig == SIGHUP)
        end_server();
}

void signal_handler_init(void)
{
    struct sigaction sa = 
    {
        .sa_sigaction = signal_handler,
        .sa_flags = SA_SIGINFO
    };
    sigemptyset(&sa.sa_mask);
    
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGRTMIN, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
}

void create_fifo(void)
{
    if (mkfifo(FIFO_NAME, 0666) == -1)
    {
        fprintf(stderr, "\033[1;31mFallo la creacion de la FIFO: %s\033[0m\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void create_shared_memory_segment(void)
{
    key_t key = ftok("Server.c", 'B');

    if ((shm.shmid = shmget(key, MSG_MAX_SIZE, IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "\033[1;31mFallo la creacion del segmento de memoria compartida: %s\033[0m\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if ((shm.shm_ptr = shmat(shm.shmid, NULL, 0)) == (char *) -1)
    {
        fprintf(stderr, "\033[1;31mNo se pudo agregar el espacio de memoria compartido al espacio del proceso: %s\033[0m\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void create_message_queue(void)
{
    key_t key = ftok("Server.c", 'B');

    if ((msgqueue.id = msgget(key, IPC_CREAT | 0666)) == -1)
    {
        fprintf(stderr, "\033[1;31mFallo la creacion de la cola de mensajes: %s\033[0m\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void recibe_msg(ChannelType channel_type)
{
    switch (channel_type)
    {
        case FIFO:        
            fifo.fd = open(FIFO_NAME, O_RDONLY);

            read(fifo.fd, fifo.buffer, MSG_MAX_SIZE);

            close(fifo.fd);
            
            refresh_stats(channel_type, fifo.buffer, 0);

            break;

        case SHARED_MEMORY:
            refresh_stats(channel_type, shm.shm_ptr, 0);
            
            memset(shm.shm_ptr, 0, MSG_MAX_SIZE);

            break;

        case MESSAGE_QUEUE:;
            msgrcv(msgqueue.id, &msgqueue.buffer, sizeof(msgqueue.buffer), 1, 0);

            refresh_stats(channel_type, msgqueue.buffer.msg, 0);

            break;

        default:
            break;
    }
}

void end_server(void)
{
    sigset_t signal_set;

    sigemptyset(&signal_set);

    sigprocmask(SIG_SETMASK, &signal_set, NULL); 

    close(fifo.fd);

    unlink(FIFO_NAME);

    shmdt(shm.shm_ptr);

    shmctl(shm.shmid, IPC_RMID, NULL);

    msgctl(msgqueue.id, IPC_RMID, NULL);

    remove(PID_SERVER_FILE);

    fprintf(stdout, "\n\033[1;34mServer STOP! -> PID: %d\033[0m\n", getpid());

    exit(EXIT_SUCCESS);
}

int main()
{
    if (access(PID_SERVER_FILE, F_OK) != -1)
    {
        fprintf(stderr, "\033[1;31mYa existe un servidor en ejecucion !\033[0m\n");
        exit(EXIT_FAILURE);
    }

    signal_handler_init();
    timers_init();

    mkdir("data", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    create_fifo();
    create_shared_memory_segment();
    create_message_queue();

    shared_server_pid();

    fprintf(stdout, "\033[1;34mServer RUN! -> PID: %d\033[0m\n", getpid());

    while (1);

    return 0;
}