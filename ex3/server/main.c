#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <unistd.h>
#include "fs/operations.h"

#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100

#define INDIM 30
#define OUTDIM 512

/* Global variables */
int numberThreads = 0;
int numberCommands = 0;
int headQueue = 0;
int insertIndex = 0;
char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];

pthread_mutex_t commandsLock;
pthread_cond_t empty, fill;

struct timeval begin, end;

/* ================================================================= */

int setSockAddrUn(char *path, struct sockaddr_un *addr) {

	if (addr == NULL)
		return 0;

	bzero((char *)addr, sizeof(struct sockaddr_un));
	addr->sun_family = AF_UNIX;
	strcpy(addr->sun_path, path);

	return SUN_LEN(addr);
}

void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}


/* prints the program's usage */
void usage() {
    fprintf(stderr, "Usage: /tecnicofs <numberOfThreads> <socketName>\n");
}

void *handleRequests() {
    //COLOCAR WHILE AQUI
    return NULL;
}

int main(int argc, char* argv[]) {
    int sockfd;
    struct sockaddr_un server_addr;
    socklen_t addrlen;
    char *socketName = "socketFS"; 

    /* Check arguments */
    if (argc != 3) {
        usage();
        exit(EXIT_FAILURE);
    }

    /* store possible arguments: numthreads */
    numberThreads = atoi(argv[1]);
    socketName = argv[2];

    /* check numberOfThreads argument */
    if (!(isdigit(numberThreads) || numberThreads > 0)) {
        fprintf(stderr, "Error: numberOfThreads must be a positive integer.\n");
        usage();
        exit(EXIT_FAILURE);
    }

    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        perror("server: can't open socket");
        exit(EXIT_FAILURE);
    }

	unlink(socketName);
    addrlen = setSockAddrUn(socketName, &server_addr);
	if (bind(sockfd, (struct sockaddr *) &server_addr, addrlen) < 0) {
		perror("server: bind error");
		exit(EXIT_FAILURE);
	}

    // TODO: Colocar em funcao handleRequests
    while (1) {
		struct sockaddr_un client_addr;
		char in_buffer[INDIM], out_buffer[OUTDIM];
		int c;

		addrlen = sizeof(struct sockaddr_un);
		c = recvfrom(sockfd, in_buffer, sizeof(in_buffer)-1, 0, (struct sockaddr *)&client_addr, &addrlen);
		if (c <= 0) continue;
		//Preventivo, caso o cliente nao tenha terminado a mensagem em '\0', 
		in_buffer[c]='\0';
		
		printf("Recebeu %s de %s\n", in_buffer, client_addr.sun_path);

		c = sprintf(out_buffer, "Mensagem %s recebida\n", in_buffer);
		
		sendto(sockfd, out_buffer, c+1, 0, (struct sockaddr *)&client_addr, addrlen);
	}
    close(sockfd);


    /* init filesystem */
    init_fs();    

    pthread_t tid[numberThreads];
    /* create and assign thread pool to handleRequests */
    for (int i = 0; i < numberThreads; i++) {
        if (pthread_create(&tid[i], NULL, handleRequests, NULL) !=0) {
            exit(EXIT_FAILURE);
        } 
    }

    gettimeofday(&begin, NULL);

    for (int i = 0; i < numberThreads; i++) {
        pthread_join(tid[i], NULL);
    }

    gettimeofday(&end, NULL);

    //print_tecnicofs_tree(outputfile);

    /* release allocated memory */
    destroy_fs();

    printf ("TecnicoFS completed in %.4f seconds.\n",
         (double) (end.tv_usec - begin.tv_usec) / 1000000 +
         (double) (end.tv_sec - begin.tv_sec));

    exit(EXIT_SUCCESS);
}
