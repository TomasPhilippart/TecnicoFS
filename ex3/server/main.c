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
#define READ 1

/* Global variables */
int numberThreads = 0;
int sockfd;
struct sockaddr_un server_addr;
socklen_t addrlen;

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

void *applyCommands() {
    struct sockaddr_un client_addr;
    char in_buffer[INDIM];;
    int c;

    while (1) {
		addrlen = sizeof(struct sockaddr_un);
		c = recvfrom(sockfd, in_buffer, sizeof(in_buffer)-1, 0, (struct sockaddr *)&client_addr, &addrlen);
		if (c <= 0) continue;
		//Preventivo, caso o cliente nao tenha terminado a mensagem em '\0', 
		in_buffer[c]='\0';

        const char* command = in_buffer;

        char token, type;
        char name[MAX_INPUT_SIZE], secondArgument[MAX_INPUT_SIZE];

        int numTokens = sscanf(command, "%c %s %s", &token, name, secondArgument);
        type = (char) secondArgument[0];

        if (numTokens < 2) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }

        int status;
        switch (token) {
            case 'c': /* CREATE */
                switch (type) {
                    case 'f':
                        printf("Create file: %s\n", name);
                        status = create(name, T_FILE); 
                        
                        break;
                    case 'd':
                        printf("Create directory: %s\n", name);
                        status = create(name, T_DIRECTORY);
                        break;
                    default:
                        fprintf(stderr, "Error: invalid node type\n");
                        exit(EXIT_FAILURE);
                }
                break;

            case 'l': /* LOOKUP */
                {  
                int inodes_visited[INODE_TABLE_SIZE];
	            int num_inodes_visited = 0;
                
                status = lookup(name, inodes_visited, &num_inodes_visited, READ);
                unlock_inodes(inodes_visited, num_inodes_visited);

                if (status >= 0) {
                    printf("Search: %s found\n", name);
                } else {
                    printf("Search: %s not found\n", name);
                }

                break;
                }

            case 'd': /* DELETE */
                printf("Delete: %s\n", name);
                status = delete(name);
                break;
            
            case 'm': /* MOVE */
                printf("Move: %s %s\n", name, secondArgument);
                status = move(name, secondArgument);
                break;

            case 'p': /* PRINT */
                printf("Print tree\n");
                status = print_tecnicofs_tree(name);
                break;
            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    sendto(sockfd, &status, sizeof(status), 0, (struct sockaddr *)&client_addr, addrlen);
    }
    return NULL;
}


int main(int argc, char* argv[]) {
    char *socketName;

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


    /* init filesystem */
    init_fs();    

    pthread_t tid[numberThreads];
    /* create and assign thread pool to handleRequests */
    for (int i = 0; i < numberThreads; i++) {
        if (pthread_create(&tid[i], NULL, applyCommands, NULL) !=0) {
            exit(EXIT_FAILURE);
        } 
    }

    for (int i = 0; i < numberThreads; i++) {
        pthread_join(tid[i], NULL);
    }

    close(sockfd);

    /* release allocated memory */
    destroy_fs();

    exit(EXIT_SUCCESS);
}
