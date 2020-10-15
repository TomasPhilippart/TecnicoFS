#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>
#include "fs/operations.h"
#include "fs/synch.h"

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

/* Synch strategies*/
#define NOSYNC 0
#define MUTEX 1
#define RWLOCK 2

/* Global variables */
int numberThreads = 0;
char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

/* 0 -> NOSYNC, 1 -> MUTEX, 2 -> RWLOCK */
int synch;

/* Locks */
void *lock1;
void *lock2; /* This lock is only intended to be used for removeCommands function*/


int insertCommand(char* data) {
    if(numberCommands != MAX_COMMANDS) {
        strcpy(inputCommands[numberCommands++], data);
        return 1;
    }
    return 0;
}

char* removeCommand() {
    int synch2 = synch;
    /* removeCommand can only be secured with NOSYNC or MUTEX locks*/
    if (synch == 2) { /* if synchstrategy is RWLOCK, go with MUTEX instead*/
        synch2 = 1;
    }

    lock(synch2, lock2, false); /* LOCK */
    if(numberCommands > 0){
        numberCommands--;
        unlock(synch2, lock2); /* UNLOCK */
        return inputCommands[headQueue++];  
    }
    unlock(synch2, lock2); /* UNLOCK */
    return NULL;
}

void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
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
                if(insertCommand(line))
                    break;
                return;
            
            case 'l':
                if(numTokens != 2)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
            
            case 'd':
                if(numTokens != 2)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
            
            case '#':
                break;
            
            default: { /* error */
                errorParse();
            }
        }
    }
}

void *applyCommands(){
    while (numberCommands > 0){
        const char* command = removeCommand();
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
            case 'c':
                switch (type) {
                    case 'f':
                        lock(synch, lock1, false);

                        printf("Create file: %s\n", name);
                        create(name, T_FILE);

                        unlock(synch, lock1);
                        break;
                    case 'd':
                        lock(synch, lock1, false);

                        printf("Create directory: %s\n", name);
                        create(name, T_DIRECTORY);

                        unlock(synch, lock1);
                        break;
                    default:
                        fprintf(stderr, "Error: invalid node type\n");
                        exit(EXIT_FAILURE);
                }
                break;
            case 'l': 
                lock(synch, lock1, true);

                searchResult = lookup(name);
                if (searchResult >= 0)
                    printf("Search: %s found\n", name);
                else
                    printf("Search: %s not found\n", name);

                unlock(synch, lock1);
                break;
            case 'd':
                lock(synch, lock1, false);

                printf("Delete: %s\n", name);
                delete(name);

                unlock(synch, lock1);
                break;
            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    
    return NULL;
}

int main(int argc, char* argv[]) {

    /* store possible arguments: inpufile outputfile numthreads synchstrategy */
    FILE *inputfile = fopen(argv[1], "r");
    numberThreads = atoi(argv[3]);
    char *synchstrategy = argv[4];
    
    if (!inputfile) { /* check for successful file opening */
        perror("Error");
        exit(EXIT_FAILURE);
    }

    /* determine synch strategy choice */
    if (!strcmp(synchstrategy, "nosync")) {
        if (numberThreads == 1) {
            synch = NOSYNC;
        } else {
            fprintf(stderr, "Error: invalid synchstrategy and numthreads\n");
            exit(EXIT_FAILURE);
        }

    } else if (!strcmp(synchstrategy, "mutex")) {
        synch = MUTEX;

    } else if (!strcmp(synchstrategy, "rwlock")) {
        synch = RWLOCK;
    } else {
        fprintf(stderr, "Error: invalid synchstrategy\n");
        exit(EXIT_FAILURE);
    }

    /* init filesystem */
    init_fs();

    /* initialize lok1 with desired synchstrategy*/
    init_lock(synch, &lock1); 

    clock_t begin = clock();

    /* initialize lock2 with mutex/nosync only*/
    if (synch == 2) {
        init_lock(1, &lock2);
    } else {
        init_lock(synch, &lock2); 
    }
        
    /* process input and print tree */
    processInput(inputfile);
    fclose(inputfile);
    
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

    print_tecnicofs_tree(stdout);

    /* release allocated memory */
    destroy_fs();
    // FIXME - destroy_lock
    destroy_lock(synch, &lock1);

    if (synch == 2) {
        destroy_lock(1, &lock2);
    } else {
        destroy_lock(synch, &lock2);
    }
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    fprintf(stdout, "TecnicoFS completed in %.4f seconds.\n", time_spent);

    exit(EXIT_SUCCESS);
}
