#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum {idle, want_in, in_cs} state;
extern int turn;
extern state flag[];
