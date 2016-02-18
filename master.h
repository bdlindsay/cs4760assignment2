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
#include <signal.h>

typedef enum {false, true} bool;

typedef enum {idle, want_in, in_cs} state;

typedef struct info_t {
	state flag[18]; // flag for each process in shared memory
	int turn; // says whose turn it is
} info_t;

extern info_t *s_info;


#endif
