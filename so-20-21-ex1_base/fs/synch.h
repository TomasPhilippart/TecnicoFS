#ifndef SYNCH_H
#define SYNCH_H

#include <stdbool.h>

void init_lock(int synchstrategy, void **lock);
void lock(int synchstrategy, void *lock, bool read);
void unlock(int synchstrategy, void *lock);
void destroy_lock(int synchstrategy, void **lock);

#endif /* SYNCH_H */
