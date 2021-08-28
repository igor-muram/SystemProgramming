#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

#define MAX_CONNECTIONS 5
#define SIZE 1024

typedef struct message
{
    long mtype;
    char mtext[SIZE];
} msg;

int check_port(char *arg)
{
    int length = strlen(arg);

    if (length > 5)
        return -1;

    for (int i = 0; i < length; i++)
        if (!isdigit(arg[i]))
            return -1;

    return atoi(arg);
}

int queue_size(int msgqid)
{
    struct msqid_ds stat;
    if (msgctl(msgqid, IPC_STAT, &stat) == -1)
    {
        return -1;
    }

    return stat.msg_qnum;
}

int get_client_from_queue(int msgqid)
{
    msg m;
    m.mtype = 1;

    if (msgrcv(msgqid, (void *)&m, SIZE, 1, 0) < 0)
        return -1;

    return atoi(m.mtext);
}

int add_client_to_queue(int msgqid, int client)
{
    msg m;
    m.mtype = 1;
    sprintf(m.mtext, "%d", client);

    if (msgsnd(msgqid, (void *)&m, SIZE, IPC_NOWAIT) < 0)
        return -1;

    return 0;
}

int main(int argc, char **argv)
{
    // Check arguments ================================================
    if (argc != 3)
    {
        errno = EINVAL;
        perror("Wrong number of args");
        return errno;
    }
    // ================================================================

    // Parse IP =======================================================
    struct in_addr ip;
    ip.s_addr = inet_addr(argv[1]);

    if (ip.s_addr == -1)
    {
        errno = EINVAL;
        perror("Incorrect ip");
        return errno;
    }
    // ================================================================

    // Parse port ===================================================
    int port = check_port(argv[2]);
    if (port == -1)
    {
        errno = EINVAL;
        perror("Incorrect port");
        return errno;
    }

    // Create socket =================================================
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("socket");
        return errno;
    }
    // ================================================================
   
    // Make socket non-blocking =======================================
    int flags = fcntl(sock, F_GETFL);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    // ================================================================

    // Create socket address ==========================================
    struct sockaddr_in address;
    address.sin_addr = ip;
    address.sin_family = AF_INET;
    address.sin_port = port;
    // ================================================================

    // Bind socket ====================================================
    if (bind(sock, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        perror("bind");
        return errno;
    }
    // ================================================================

    // Listen socket ==================================================
    if (listen(sock, MAX_CONNECTIONS) == -1)
    {
        perror("listen");
        return errno;
    }
    // ================================================================

    // Create message queue ==========================================
    int msgqid = msgget(0, IPC_CREAT | 0666);
    if (msgqid < 0)
    {
        perror("msgget");
        return errno;
    }
    // ================================================================

    //Create semaphore ================================================
    struct sembuf op[1];
    op[0].sem_num = 0;
    op[0].sem_flg = IPC_NOWAIT;
    op[0].sem_op = 0;

    int semid = semget(0xF00B00, 1, IPC_CREAT | 0600);
    if (semid == -1)
    {
        perror("semget");
        return errno;
    }
    //==================================================================

    // Start server ====================================================
    int current_client;
    int client_connected = 0;
    int timer = 0;

    while (timer < 10)
    {
        // Accept client ===================================================
        int client = accept(sock, NULL, NULL);
	if (client < 0 && queue_size(msgqid) == 0)
	{
	    printf("No new connections in queue. Sleep 10 sec\n");
	    sleep(10);
	    timer++;
	}
	else if (client > 0 || queue_size(msgqid) > 0)
	{
		if (client > 0)
		{
        		// Add client to queue =============================================
        		add_client_to_queue(msgqid, client);
	        	printf("Server added new client[%d]\n", client);
		}
        	timer = 0;
        	//==================================================================
	

        	// Check if child finished to process client =======================
        	if (semop(semid, op, 1) == 0)
        	{
	            client_connected = 0;
        	    current_client = 0;
       		}
        	//==================================================================

        	// If client connected send message ================================
        	if (client_connected)
        	{
            		char msg[256] = "Server is busy. Wait for your turn.";
         	   	send(client, msg, sizeof(msg), 0);
        	}
        	//==================================================================

        	// If no clients connected =========================================
        	if (!client_connected && queue_size(msgqid) > 0)
        	{
            		// Take client from queue ======================================
            		client_connected = 1;
            		current_client = get_client_from_queue(msgqid);
            		printf("Server started to process client[%d]\n", current_client);

            		// Start to process client in child process ====================
            		int child = fork();
            		if (child == 0)
            		{
                		// Lock semaphore ==================================================
                		union semun ctrl;
                		ctrl.val = 1;
                		if (semctl(semid, 0, SETVAL, ctrl) == -1)
                		{
                    			perror("semctl");
                    			exit(errno);
                		}
                		//==================================================================

                		// Process client ==================================================
                		printf("Client[%d] is being processed\n", current_client);
                		sleep(50);
                		char msg[256] = "Some data";
                		if (send(current_client, msg, sizeof(msg), 0) < 0)
                		{
                    			perror("send");
                    			exit(errno);
                		}
                		//==================================================================

                		close(current_client);
                		// Free semaphore ==================================================
                		op[0].sem_op = -1;
                		semop(semid, op, 1);
                		printf("Client[%d] has been processed\n", current_client);
                		//==================================================================

                		exit(0);
            		}

            		close(current_client);
        	}
		sleep(2);
    	}
	}
	printf("Server finished to work\n");
	msgctl(msgqid, IPC_RMID, NULL);
	semctl(semid, 0, IPC_RMID);
    	return 0;
}
