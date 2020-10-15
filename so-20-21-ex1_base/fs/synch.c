#include "synch.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void init_lock(int synchstrategy, void **lock) {
    if (synchstrategy == 0) { /* NOSYNC */
        /* do nothing */
    } else if (synchstrategy == 1) { /* MUTEX*/  
        *lock = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init((pthread_mutex_t *)*lock, NULL);
    } else if (synchstrategy == 2) { /* RWLOCK */
        *lock = malloc(sizeof(pthread_rwlock_t));
        pthread_rwlock_init((pthread_rwlock_t *)*lock, NULL);
    }
}

void lock(int synchstrategy, void *lock, bool read) {
    if (synchstrategy == 0) { /* NOSYNC */
        /* do nothing */
    } else if (synchstrategy == 1) { /* MUTEX*/  
        if (pthread_mutex_lock((pthread_mutex_t *)lock)) {
            exit(EXIT_FAILURE);
        }
    } else if (synchstrategy == 2 && read) { /* RWLOCK */
        if (pthread_rwlock_rdlock((pthread_rwlock_t *)lock)) {
            exit(EXIT_FAILURE);
        }
    } else if (synchstrategy == 2 && !read) {
        if (pthread_rwlock_wrlock((pthread_rwlock_t *)lock)) {
            exit(EXIT_FAILURE);
        }
    }
}

void unlock(int synchstrategy, void *lock) {
    if (synchstrategy == 0) { /* NOSYNC */
        /* do nothing */
    } else if (synchstrategy == 1) { /* MUTEX*/  
        if (pthread_mutex_unlock((pthread_mutex_t *)lock)) {
            exit(EXIT_FAILURE);
        }
    } else if (synchstrategy == 2) { /* RWLOCK */
        if (pthread_rwlock_unlock((pthread_rwlock_t *)lock)) {
            exit(EXIT_FAILURE);
        }
    }
}
void destroy_lock(int synchstrategy, void **lock) {
    if (synchstrategy == 0) { /* NOSYNC */
        /* do nothing */
    } else if (synchstrategy == 1) { /* MUTEX*/  
        if (pthread_mutex_destroy((pthread_mutex_t *)lock)) {
            free(lock);
            exit(EXIT_FAILURE);
        }
    } else if (synchstrategy == 2) { /* RWLOCK */
        if (pthread_rwlock_destroy((pthread_rwlock_t *)lock)) {
            free(lock);
            exit(EXIT_FAILURE);
        }
    }
}