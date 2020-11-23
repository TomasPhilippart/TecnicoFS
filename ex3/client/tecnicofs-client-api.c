#include "tecnicofs-client-api.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

#define SUCCESS 0
#define FAIL 1

char socketName[MAX_FILE_NAME];
char *serverSocket;
int sockfd;
socklen_t servlen, clilen;
struct sockaddr_un serv_addr, client_addr;

int setSockAddrUn(char *path, struct sockaddr_un *addr) {
  if (addr == NULL)
    return 0;

  bzero((char *)addr, sizeof(struct sockaddr_un));
  addr->sun_family = AF_UNIX;
  strcpy(addr->sun_path, path);

  return SUN_LEN(addr);
}

int tfsCreate(char *filename, char nodeType) {
	char message[MAX_FILE_NAME]; //REVIW size
	sprintf(message, "c %s %c", filename, nodeType);
	if (sendto(sockfd, message, strlen(message)+1, 0, (struct sockaddr *) &serv_addr, servlen) < 0) {
		perror("client: sendto error");
		exit(EXIT_FAILURE);
	} 

	char buffer[1024];
	if (recvfrom(sockfd, buffer, sizeof(buffer), 0, 0, 0) < 0) {
		perror("client: recvfrom error");
		exit(EXIT_FAILURE);
	} 

	printf("Recebeu resposta do servidor: %s\n", buffer);

	return 0;
}

int tfsDelete(char *path) {
	return -1;
}

int tfsMove(char *from, char *to) {
	return -1;
}

int tfsLookup(char *path) {
	return -1;
}

int tfsMount(char * sockPath) {
	sprintf(socketName, "/tmp/clientSocketFS_%d", getpid());
	serverSocket = sockPath; // save the server socket name in a global variable (3aii)

	if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0) ) < 0) {
    	perror("client: can't open socket");
    	exit(EXIT_FAILURE);
  	}

  	clilen = setSockAddrUn(socketName, &client_addr);
	servlen = setSockAddrUn(serverSocket, &serv_addr);
	unlink(socketName);
  	if (bind(sockfd, (struct sockaddr *) &client_addr, clilen) < 0) {
		perror("client: bind error");
    	exit(EXIT_FAILURE);
  	}

	return 0;	  
}

int tfsUnmount() {
	unlink(socketName);
	return -1;
}
