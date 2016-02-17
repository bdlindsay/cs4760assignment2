#include "master.h"

const int p_n = 19; // process number to send each process
const int n = 18; // its respective place in the flag array (1 less)
extern info_t *s_info = NULL;

main() {
	char *arg1 = "slave"; // to send execl process argv[0]
	char *arg2 = malloc(sizeof(int)); // to send execl process args
	char *arg3 = malloc(sizeof(int)); // to send execl process args
	int pid;
	int key = 20; // key of turn shared int
	int shm_id; // shm_id of turn shared int
	int act_procs = 0; // active process counter
	int i = 0; // index var

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
		pid = fork();
		if (pid < 0) { // error checking
			perror("fork:");
		}
		if (pid == 0) { // don't let children spawn more children
			break;        // for clarity, could just use execl at this point
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
		printf("In master-");
		printf("processes=%d\n", act_procs);

		// release shared memory for s_info 
		//printf("Removing- turn  ID: %d\n",shm_id);
		if((shmctl(shm_id, IPC_RMID, NULL)) == -1){
			perror("shmctl:IPC_RMID");
			exit(1);
		}
		// free argument memory for shm_id transfer and process num transfer 
		free(arg2);
		free(arg3);
	} // end else for pid > 0 -> parent process actions
} // end main

