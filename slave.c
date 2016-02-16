#include "master.h"

//typedef enum {idle, want_in, in_cs} state;
const int CREATE_NUM = 5;
extern state *flag = idle; // flag for each process in shared memory
extern intptr_t turn = 0; 
extern int n = 0;

main (int argc, char *argv[]) {
	int shmid = atoi(argv[2]); // shared turn in shm_id from parent process
	int process_num = atoi(argv[1]); // process num sent from parent process
	int key = 1; // key for turn intptr_t is 1
	int shmid_flag;
	int key_flag = 21; // key for flag array is 21
	int *turnptr;

	printf("Slave code: %d\n",process_num);	

	// get and attach flag array
	shmid_flag = shmget(key_flag, sizeof(flag),IPC_CREAT | 0755);
	//printf("shmget key_flag:%d:%d\n",key_flag,shmid_flag);
	flag = (state*)shmat(shmid_flag,0,0);
	//printf("In slave - %d - setting flag[%d]\n",process_num,process_num);
	flag[process_num] = want_in;

	// get and attach turn pointer
	//shmid = shmget(key, sizeof(intptr_t),IPC_CREAT | 0755);
	printf("Process Number: %d shmid: %d\n",process_num,shmid);
	turnptr = (int*)shmat(shmid,0,0);
	memcpy(&turn,turnptr,sizeof(intptr_t));
	printf("Slave setting turn ID: %d\n",shmid);
	//turn = 2; // TODO do I need this cast
	memcpy(turnptr,&turn,sizeof(intptr_t));
	/*printf("ENTERING PROCESS(%d)",key); // TODO
	process(key);
	printf("EXIT FROM PROCESS(%d)",key);*/
	// detach
	shmdt(flag);
	shmdt(&turn);
	/*printf("Removing process %d turn: %d\n",key, shmid);
	shmctl(shmid, IPC_RMID, NULL);
	*/
}

process (const int i) {
	int j; // local for each process
	int numWrites = 0;
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
	fprintf(stderr, "Process %d entering critical section", i);
	critical_section();
	fprintf(stderr, "Process %d exiting critical section", i);
	numWrites++; // counter for how many writes the process has done
	// Exit critical section
	j = (turn + 1) % n;
	while (flag[j] == idle) {
		j = (j + 1) % n;
	}

	// Assign turn to next waiting process; change own flag to idle
	turn = j;
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
	// TODO
	// write to file
	fprintf(fp, "%s", msg);
	// sleep for 0-2 seconds
	// TODO
	// clean up
	free(msg);
	fclose(fp);
}
