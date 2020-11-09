#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/time.h>
#include "fs/operations.h"

#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100

/* Global variables */
int numberThreads = 0;
int numberCommands = 0;
int headQueue = 0;
char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];

pthread_mutex_t commandsLock;
pthread_cond_t empty, fill;

struct timeval begin, end;

/* ========================================================= */

void insertCommand(char* data) {
    strcpy(inputCommands[numberCommands], data);
    numberCommands = (numberCommands + 1) % MAX_COMMANDS; /* increment circularly*/
    printf("numberCommands: %d\n", numberCommands);
}

/* REVIEW: qq coisa esquisita aqui */
char *removeCommand() {
    char *tmp = inputCommands[headQueue];
    if (numberCommands > 0) {
        numberCommands = (numberCommands - 1) % MAX_COMMANDS;
        headQueue = (headQueue + 1) % MAX_COMMANDS; /* increment circularly*/
        return tmp;
    }
    return NULL;
}

void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

void insertProtectedCommand(char *line) {
    pthread_mutex_lock(&commandsLock);
    while (numberCommands == MAX_COMMANDS) {
        pthread_cond_wait(&empty, &commandsLock);
    }

    insertCommand(line);
    pthread_cond_signal(&fill);
    pthread_mutex_unlock(&commandsLock);
}
/* function inserts N shutdown commands, where N is the number of threads*/
void shutdown() {
    for (int i = 0; i < numberThreads; i++) {
        insertProtectedCommand("s X X"); // s command signals shutdown
    }
}

void processInput(FILE *inputfile){
    char line[MAX_INPUT_SIZE];

    /* break loop with ^Z or ^D */
    while (fgets(line, sizeof(line)/sizeof(char), inputfile)) { // reads line from inputfile
        char token, type;
        char name[MAX_INPUT_SIZE];

        int numTokens = sscanf(line, "%c %s %c", &token, name, &type);
        /* perform minimal validation */
        if (numTokens < 1) {
            continue;
        }
        switch (token) {
            case 'c':
                if(numTokens != 3) 
                    errorParse();

                insertProtectedCommand(line);
                break;
    
            case 'l':
                if(numTokens != 2)
                    errorParse();

                insertProtectedCommand(line);
                break;

            case 'd':
                if(numTokens != 2)
                    errorParse();

                insertProtectedCommand(line);
                break;

            case '#':
                break;
            
            default: { /* error */
                errorParse();
            }
        }
        
    }
    // marks end of commands to process
    shutdown();
}

void *applyCommands() {
    while (1) {
        pthread_mutex_lock(&commandsLock);
        while (numberCommands == 0) {
            pthread_cond_wait(&fill, &commandsLock);
        }

        const char* command = removeCommand();
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(&commandsLock);

        if (command == NULL){
            continue;
        }

        char token, type;
        char name[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s %c", &token, name, &type);
        if (numTokens < 2) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }

        int searchResult;
        switch (token) {
            case 'c': /* CREATE */
                switch (type) {
                    case 'f':
                        printf("Create file: %s\n", name);
                        create(name, T_FILE);
                        break;
                    case 'd':
                        printf("Create directory: %s\n", name);
                        create(name, T_DIRECTORY);
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
                
                searchResult = lookup(name, inodes_visited, &num_inodes_visited, READ);
                unlock_inodes(inodes_visited, num_inodes_visited);

                if (searchResult >= 0)
                    printf("Search: %s found\n", name);
                else
                    printf("Search: %s not found\n", name);
                break;
                }

            case 'd': /* DELETE */
                printf("Delete: %s\n", name);
                delete(name);
                break;

            case 's': /* SHUTDOWN */
                printf("shutting down\n");
                return NULL;

            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    return NULL;
}


int main(int argc, char* argv[]) {
    /* store possible arguments: inpufile outputfile numthreads */
    FILE *inputfile = fopen(argv[1], "r");
    FILE *outputfile = fopen(argv[2], "w+");
    numberThreads = atoi(argv[3]);
    
    if (!inputfile) { /* check for successful file opening */
        perror("Error");
        exit(EXIT_FAILURE);
    }

    /* init filesystem */
    init_fs();
    
    /* process input and print tree */
    processInput(inputfile);
    gettimeofday(&begin, NULL);
    fclose(inputfile);

    pthread_mutex_init(&commandsLock, NULL);

    pthread_t tid[numberThreads];
    /* create and assign thread pool to applyCommands */
    for (int i = 0; i < numberThreads; i++) {
        if (pthread_create(&tid[i], NULL, applyCommands, NULL) !=0) {
            exit(EXIT_FAILURE);
        } 
    }

    for (int i = 0; i < numberThreads; i++) {
        pthread_join(tid[i], NULL);
    }

    gettimeofday(&end, NULL);

    print_tecnicofs_tree(outputfile);
    fclose(outputfile);

    /* release allocated memory */
    destroy_fs();

    printf ("TecnicoFS completed in %.4f seconds.\n",
         (double) (end.tv_usec - begin.tv_usec) / 1000000 +
         (double) (end.tv_sec - begin.tv_sec));

    exit(EXIT_SUCCESS);
}
