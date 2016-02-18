#include "master.h"

// slave.c
// Brett Lindsay
// Project 2 CS4760

const int p_n = 19;
const int n = 18;
extern info_t *s_info = NULL; // shared info
int process_num; // global to inform user on SIGINT
char *msg; // global to release on SIGINT if necessary
FILE *fp; // global to close on SIGINT if necessary

void intr_handler();

main (int argc, char *argv[]) {
	int shm_id = atoi(argv[2]); // shared turn in shm_id from parent process
	process_num = atoi(argv[1]); // process num sent from parent process
	int p_index = process_num - 1;
	int key = 20; // key for *s_info is 20 
	int rt_shm_id;
	
	signal(SIGINT,intr_handler);

	printf("Slave Process: %d\n",process_num);	

	// get and attach *s_info with key  
	rt_shm_id = shmget(key, sizeof(info_t*),IPC_CREAT | 0755);
	if (rt_shm_id != shm_id) {
		fprintf(stderr,"shm ids do not match - fatal error");
		exit(1);
	}
	s_info = (info_t*)shmat(shm_id,0,0);

	// run process to write to file
	process(p_index);

	// detach *s_info
	shmdt(s_info);
}

process (const int i) {
	int j; // local for each process
	int numWrites = 0; // var to only allow 3 writes each process
	time_t tcurrent;
	struct tm *timeinfo;

	do {
		do {
			time(&tcurrent);
			timeinfo = localtime(&tcurrent);
			fprintf(stderr, "Process number %d wants in at %d:%02d:%02d\n",
				(i+1), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
			s_info->flag[i] = want_in; // raise my flag
			j = s_info->turn; // set local variable
			while (j != i) { // while it's not my turn
				j = (s_info->flag[j] != idle) ? s_info->turn : (j + 1 ) % p_n;
			}
			// Declare intention to enter critial section
			s_info->flag[i] = in_cs;
			// Check that no one else in critical section
			for (j = 0; j < n; j++) {
				if ((j != i) && (s_info->flag[j] == in_cs)) {
					break;
				}
			}
		} while ((j < n) || (s_info->turn != i && s_info->flag[s_info->turn] != idle));	
		// Assign turn to self and enter critial section
		s_info->turn = i;
		time(&tcurrent);
		timeinfo = localtime(&tcurrent);
		fprintf(stderr, "Process %d entering critical section at %d:%02d:%02d\n", 
			(i+1), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
			
		critical_section(i+1);
		time(&tcurrent);
		timeinfo = localtime(&tcurrent);
		fprintf(stderr, "Process %d exiting critical section at %d:%02d:%02d\n",
			(i+1),timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

		numWrites++; // counter for how many writes the process has done

		// Exit critical section 
		// Check who else wants_in
		j = (s_info->turn + 1) % n; 
		while (s_info->flag[j] == idle) {
			j = (j + 1) % p_n;
		}

		// Assign turn to next waiting process; change own flag to idle
		s_info->turn = j;
		s_info->flag[i] = idle;

		// remainder_section();
	} while (numWrites < 3);
} // end process() 

critical_section(int id) {
	time_t tcurrent;
	struct tm *timeinfo;
	int r;
	srandom(time(NULL));
	// open file	
	fp = fopen("cstest", "a");
	if (!fp) { // error checking on file open
		perror("fopen error");
		return -1;
	}	
	// sleep for 0-2 seconds
	r = random() % 3;
	sleep(r);

	// create/alloc memory for string to write
	time(&tcurrent);
	timeinfo = localtime(&tcurrent);
	asprintf(&msg, "File modified by process number %d at time %d:%02d:%02d\n",
		id, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	// write to file
	fprintf(fp, "%s", msg);

	// sleep for 0-2 seconds
	r = random() % 3;
	sleep(r);

	// clean up
	free(msg);
	msg = NULL;
	fclose(fp);
	fp = NULL;
}

void intr_handler() {
	int pid;
	info_t *tmp;
	signal(SIGINT,SIG_DFL); // make sure handler stays defined
	if (s_info != NULL)  { // if attached to shared memory, detach
		shmdt(tmp);
	}	
	if (msg != NULL) { // if allocated memory, free it
		free(msg);
	}
	if (fp != NULL) { // if file open, close it
		fclose(fp);
	}	
	fprintf(stderr,"Recieved SIGINT: Process %d detached s_info and dying.\n",
		process_num);
	// let it do default actions for SIGINT by resending now
	raise(SIGINT);
}
