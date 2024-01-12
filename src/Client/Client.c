/**
 * @file Client.c
 * @author Bottini, Franco Nicolas.
 * @brief Implementacion del Cliente IPC.
 * @version 1.0.1
 * @date Marzo de 2023.
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "Client.h"

/**
 * @struct flags
 * 
 * Flags de comunicacion con el servidor.
*/
struct
{
    //Flag de establecimiento de comunicacion con el servidor.
    int connect;

    //Flag que indica al cliente si enviar un mensaje o esperar de acuerdo al estado del canal.
    int wait;
} flags;

//Puntero que almacena la instancia del cliente creada al ejecutar el programa.
Client* client;

//Array auxiliar para obtener un elemento del enumerado 'ChannelType' en formato de cadena.
const char* ChannelStringType[] = { "FIFO", "SHARED MEMORY", "MESSAGE QUEUE" };

void print_help(void)
{
	fprintf(stdout, "\n\033[1;34m");
	fprintf(stdout, "Se debe dar como argumento de entrada el canal sobre el que va a operar el cliente a instanciar, existen 3 opciones:\n");
	fprintf(stdout, "	- 0: FIFO\n");
	fprintf(stdout, "	- 1: SHARED MEMORY\n");
	fprintf(stdout, "	- 2: MESSAGE QUEUE\n");
	fprintf(stdout, "\033[0m\n");
}

void signal_handler(int sig, siginfo_t *info, void* context)
{
	UNUSED(context);

	if (sig == SIGUSR1)
	{
		USRSignalType signal_type = (USRSignalType)info->si_value.sival_int;

		if (signal_type == START_WRITE)
			flags.wait = 0;
		else if (signal_type == WAIT)
			flags.wait = 1;

		flags.connect = 1;
	}
    else if(sig == SIGTERM || sig == SIGINT || sig == SIGHUP)
        end_client();
}

Client* client_factory(ChannelType type, int server_pid)
{
    Client* client = malloc(sizeof(Client));

    client->type = type;
	client->server_pid = server_pid;
    
    switch (type) 
	{
    	case FIFO:
        	client->send = &fifo_send;
			client->init = NULL;
        	break;

      	case SHARED_MEMORY:;
			client->send = &shared_memory_send;	
			client->init = &shared_memory_init;
        	break;

      	case MESSAGE_QUEUE:
			client->send = &message_queue_send;
			client->init = &message_queue_init;
        	break;

      	default:
        	free(client);
        	client = NULL;
			break;
    }

    return client;
}

void client_init(int argc, char* argv[])
{
	FILE *fp;
    char buffer[11];
	int server_pid, channel_type;

	if (argc != 2)
	{
		fprintf(stderr, "\033[1;31mNúmero de argumentos invalido !\033[0m\n");
		print_help();
		exit(EXIT_FAILURE);
	}

	channel_type = atoi(argv[1]);

	if(*argv[1] != '0' && channel_type == 0)
	{
		fprintf(stderr, "\033[1;31mArgumentos invalidos !\033[0m\n");
		print_help();
		exit(EXIT_FAILURE);
	}

	if (access(PID_SERVER_FILE, F_OK) == -1)
	{
		fprintf(stderr, "\033[1;31mNo se encontró un servidor en ejecucion !\033[0m\n");
		exit(EXIT_FAILURE);
	}

    fp = fopen(PID_SERVER_FILE, "r");

	server_pid = atoi(fgets(buffer, sizeof(buffer), fp));

	fclose(fp);
	
	client = client_factory((ChannelType)channel_type, server_pid);

	if(!client)
	{
		fprintf(stderr, "\033[1;31mNo fue posible crear el cliente !\033[0m\n");
		print_help();
		exit(EXIT_FAILURE);
	}

	if (client->type != FIFO)
		client->init();
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
	sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
}

void shared_memory_init(void)
{
	int shmid;
    key_t key = ftok("Server.c", 'B');

    if ((shmid = shmget(key, MSG_MAX_SIZE, 0666)) == -1) 
	{
        fprintf(stderr, "\033[1;31mNo se pudo obtener la region de memoria compartida por el servidor !\033[0m\n");
        exit(EXIT_FAILURE);
	}
    
	if ((client->shm = shmat(shmid, NULL, 0)) == (char *) -1) 
	{
        fprintf(stderr, "\033[1;31mNo se pudo agregar el espacio de memoria compartido al espacio del proceso !\033[0m\n");
        exit(EXIT_FAILURE);
    }
}

void message_queue_init(void)
{
	key_t key = ftok("Server.c", 'B');

	if ((client->msgid = msgget(key, 0666)) == -1) 
	{
        fprintf(stderr, "\033[1;31mNo se pudo conectar con la cola de mensajes del servidor !\033[0m\n");
        exit(EXIT_FAILURE);
	}
}

int request_send(void)
{
	flags.connect = 0;

	sigqueue(client->server_pid, SIGUSR1, (union sigval) { .sival_int = (int)client->type | (int)START_WRITE << 2 });

	time_t start_time = time(NULL);

    while (difftime(time(NULL), start_time) < 1 && !flags.connect) { continue; }

	if(!flags.connect)
		return 0;
	else if(flags.wait)
	{
		struct timespec wait_time = {0, rand() % 1000000 + 10000};

		nanosleep(&wait_time, NULL);

		request_send();
	}

	return 1;
}

void fifo_send(const char* msg)
{
	if (!request_send())
		return;
	
	sigqueue(client->server_pid, SIGUSR1, (union sigval) { .sival_int = (int)client->type | (int)END_WRITE << 2 });

	int fd = open(FIFO_NAME, O_WRONLY);

	write(fd, msg, strlen(msg) + 1);

	close(fd);
}

void message_queue_send(const char* msg)
{
	if (!request_send())
		return;

	MsgQueueElemnet mq;

	mq.type = 1;

	strcpy(mq.msg, msg);

	msgsnd(client->msgid, &mq, strlen(mq.msg) + 1, 0);

	sigqueue(client->server_pid, SIGUSR1, (union sigval) { .sival_int = (int)client->type | (int)END_WRITE << 2 });
}

void shared_memory_send(const char* msg)
{
	if (!request_send())
		return;

	strcpy(client->shm, msg);

	sigqueue(client->server_pid, SIGUSR1, (union sigval) { .sival_int = (int)client->type | (int)END_WRITE << 2 });
}

void end_client(void)
{
	fprintf(stderr, "\n\033[1;31mSe detuvo la ejecucion del servidor -> Cliente %s (%d) detenido !\033[0m\n", ChannelStringType[client->type], getpid());
	
	free(client);
	
	exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[])
{
	client_init(argc, argv);

	signal_handler_init();

	int n = 0;

	sleep((unsigned int)(rand() % 3));

	while (1)
	{
		char aux[11];

		sprintf(aux, "%d", n);

		client->send(aux);
		
		n++;

		sleep((unsigned int)(rand() % 5 + 1));

		if (access(PID_SERVER_FILE, F_OK) == -1)
			end_client();
	}

	return 0;
}