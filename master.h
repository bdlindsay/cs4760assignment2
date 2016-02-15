#ifndef MASTER_H
#define MASTER_H

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

typedef enum {idle, want_in, in_cs} state;
extern state *flag; // flag for each process in shared memory
extern intptr_t turn;
extern int n;

#endif
