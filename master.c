#include "master.h"

// master.c
// Brett Lindsay
// Project 2 CS4760

const int p_n = 19; // process number to send each process
const int n = 18; // its respective place in the flag array (1 less)
extern info_t *s_info = NULL;
char *arg2; // to send execl process args
char *arg3; // to send execl process args
int pids[18] = { 0 };
bool timed_out = false;
void timeout();
void free_mem();

main(int argc, char *argv[]) {
	char *arg1 = "slave"; // to send execl process argv[0]
	arg2 = malloc(sizeof(int)); // to send execl process args
	arg3 = malloc(sizeof(int)); // to send execl process args
	int pid;
	int key = 20; // key of turn shared int
	int shm_id; // shm_id of turn shared int
	int act_procs = 0; // active process counter
	int i = 0; // index var

	// signal handling: timeout - 60s, on ctrl-c free memory allocated and quit
	signal(SIGALRM,timeout);
	signal(SIGINT,free_mem);
	
	if (argc > 1) { // user specified timeout period
		i = atoi(argv[1]); 
		fprintf(stderr,"Setting timeout for %d seconds\n",i);
		alarm(i);
	} else { // default
		fprintf(stderr,"Setting timeout for 60 seconds (default)\n");
		alarm(60);
	}
	// create shared info_t to hold flags and turn & error checking
	if((shm_id = shmget(key, sizeof(info_t*),IPC_CREAT | 0755)) == -1) {
		perror("shmget:");
		exit(1);
	}
	s_info = (info_t*)shmat(shm_id,0,0);
	for(i = 0; i < n ; i++) { // init all process flags to idle
		s_info->flag[i] = idle;
	}
	s_info->turn = 0; // init turn to 0
	shmdt(s_info);

	// fork for each child process to create
	sprintf(arg3,"%d",shm_id); // var for shm_id to s_info for each process
	for(i = 1; i <= p_n; i++) { // 1 through 19
		sprintf(arg2,"%d", i); // var for process number for each process
		act_procs++; // increment each time a new process is created
		if (act_procs > 20) {
			fprintf(stderr,"Too many processes created. Fatal error.");
			raise(SIGINT);
		}
		pid = fork();
		if (pid < 0) { // error checking
			perror("fork:");
		}
		if (pid == 0) { // don't let children spawn more children
			break;        // for clarity, could just use execl at this point
		} else {
			pids[(i-1)] = pid; // save each process pid

		}
	}
	if (pid == 0) { // children process actions
		execl("slave", arg1, arg2, arg3, 0); // start a slave process
	}
	else if (pid > 0) { // parent process actions
		for(i = 0; i < n; i++) { // wait for children to finish
			wait(); 
			act_procs--;
		}
		printf("In master-finished tasks. Cleaning up and quiting.\n");

		// release shared memory for s_info 
		if((shmctl(shm_id, IPC_RMID, NULL)) == -1){
			perror("shmctl:IPC_RMID");
			exit(1);
		}
		// free argument memory for shm_id transfer and process num transfer 
		free(arg2);
		free(arg3);
	} // end else for pid > 0 -> parent process actions
} // end main

void free_mem() {
	int shm_id;
	int i; // counter
	int key = 20; // key for info_t struct
	
	fprintf(stderr,"Received SIGINT. Cleaning up and quiting.\n");
	// kill each process if program timed out
	for(i = 0; i < 19; i++) { // 0-18
		kill(pids[i],SIGINT); // kill child process
		waitpid(pids[i],NULL,0);
	}
	// to be safe
	system("killall slave");

	// get the shm_id of s_info
	if((shm_id = shmget(key, sizeof(info_t*),0755)) == -1) {
		perror("shmget:");
		exit(1);
	}
	// free allocated memory
	if((shmctl(shm_id, IPC_RMID, NULL)) == -1) {
		perror("shmctl:IPC_RMID");
		exit(1);
	}	
	free(arg2);
	free(arg3);
	signal(SIGINT,SIG_DFL); // restore default action to SIGINT
	raise(SIGKILL); // take normal action for SIGINT after cleanup
}

void timeout() {
	// timeout duration passed send SIGINT
	timed_out = true;
	fprintf(stderr,"Timeout duration reached.\n");
	raise(SIGINT);
}
