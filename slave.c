#include "master.h"

//typedef enum {idle, want_in, in_cs} state;
const int p_n = 5;
extern state *flag = want_in; // flag for each process in shared memory
extern intptr_t turn = 1; 
extern int n = 4;

main (int argc, char *argv[]) {
	int shmid = atoi(argv[2]); // shared turn in shm_id from parent process
	int process_num = atoi(argv[1]); // process num sent from parent process
	int p_index = process_num - 1;
	int key = 1; // key for turn intptr_t is 1
	int shmid_flag;
	int key_flag = 21; // key for flag array is 21
	int *turnptr;

	printf("Slave code: %d\n",process_num);	

	// get and attach flag array with key
	shmid_flag = shmget(key_flag, sizeof(flag),IPC_CREAT | 0755);
	flag = (state*)shmat(shmid_flag,0,0);

	// attach turn pointer with turn shmid
	turnptr = (int*)shmat(shmid,0,0);
	memcpy(&turn,turnptr,sizeof(intptr_t));

	// run process to write to file
	process(p_index,turnptr);

	// detach
	shmdt(flag);
	shmdt(&turn);
	/*printf("Removing process %d turn: %d\n",key, shmid);
	shmctl(shmid, IPC_RMID, NULL);
	*/
}

process (const int i,int *turnptr) {
	int j; // local for each process
	int numWrites = 0; // var to only allow 3 writes each process
	do {
		do {
			flag[i] = want_in; // raise my flag
			j = turn; // set local variable
			while (j != i) { // while it's not my turn
				j = (flag[j] != idle) ? turn : (j + 1 ) % n;
			}
			// Declare intention to enter critial section
			flag[i] = in_cs;
			// Check that no one else in critical section
			for (j = 0; j < n; j++) {
				if ((j != i) && (flag[j] == in_cs)) {
					break;
				}
			}
		} while ((j < n) || (turn != i && flag[turn] != idle));	
		// Assign turn to self and enter critial section
		turn = i;
		memcpy(turnptr,&turn,sizeof(intptr_t)); // copy so other processes see
		fprintf(stderr, "Process %d entering critical section\n", (i+1));
		critical_section(i+1);
		fprintf(stderr, "Process %d exiting critical section\n", (i+1));
		numWrites++; // counter for how many writes the process has done
		// Exit critical section
		j = (turn + 1) % n;
		while (flag[j] == idle) {
			j = (j + 1) % n;
		}

		// Assign turn to next waiting process; change own flag to idle
		turn = j;
		memcpy(turnptr,&turn,sizeof(intptr_t)); // copy so other processes see
		flag[i] = idle;

		// remainder_section();
	} while (numWrites < 3);
	exit(0);
} // end process() 

critical_section(int id) {
	FILE *fp;
	time_t tcurrent;
	struct tm *timeinfo;
	char *msg;
	int r;
	srandom(time(NULL));
	time(&tcurrent);
	timeinfo = localtime(&tcurrent);
	// create/alloc memory for string
	asprintf(&msg, "File modified by process number %d at time %d:%02d\n",
		id, timeinfo->tm_hour, timeinfo->tm_min);
	// open file	
	fp = fopen("cstest", "a");
	if (!fp) {
		perror("fopen error");
		return -1;
	}	
	// sleep for 0-2 seconds
	r = random() % 3;
	sleep(r);
	// write to file
	fprintf(fp, "%s", msg);
	// sleep for 0-2 seconds
	r = random() % 3;
	sleep(r);
	// clean up
	free(msg);
	fclose(fp);
}
