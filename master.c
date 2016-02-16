#include "master.h"

const int p_n = 5;
extern intptr_t turn = 1;
extern state *flag = idle;
extern int n = 4;

main() {
	char *arg1 = "slave"; // to send execl process argv[0]
	char *arg2 = malloc(sizeof(int)); // to send execl process args
	char *arg3 = malloc(sizeof(int)); // to send execl process args
	int pid;
	int key = 1; // key of turn shared int
	int shm_id; // shm_id of turn shared int
	int act_procs = 0; // active process counter
	int i = 0; // index var
	int flag_key = 21; // key of flag array
	int shm_id_flag; // shm_id of flag array
	int *turnptr;

	// create sharded flag array
	shm_id_flag = shmget(flag_key, sizeof(flag),IPC_CREAT | 0755);
	// error checking
	if (shm_id_flag == -1) {
		perror("shmget:");
		exit(1);
	}
	//printf("shmget key_flag:%d:%d\n",flag_key,shm_id_flag);
	flag = (state*)shmat(shm_id_flag,0,0);
	for(i = 0; i < n ; i++) { // init all process flags to idle
		flag[i] = want_in;
	}
	shmdt(flag);
	// create shared turn intptr_t
	shm_id = shmget(key, sizeof(intptr_t), IPC_CREAT | 0755);
	//printf("shmget id for turn:%d\n",shm_id);
	if (shm_id == -1) {
		perror("shmget:");
		exit(1);
	}
	// fork for each child process to create
	sprintf(arg3,"%d",shm_id); // id for turn shared variable - same for all processes
	for(i = 1; i <= p_n; i++) {
		sprintf(arg2,"%d",p_n);
		act_procs++;
		pid = fork();
		if (pid < 0) {
			perror("fork:");
		}
		if (pid == 0) { // don't let children spawn more children
			break;
		}	
	}
	if (pid == 0) { // children process actions
		execl("slave", arg1, arg2, arg3, 0);
	}
	else if (pid > 0) { // parent process actions
		for(i = 0; i < n; i++) {
			wait();
			act_procs--;
		}
		printf("In master-");
		printf("processes=%d\n", act_procs);
		// check flag	
		shm_id_flag = shmget(flag_key, sizeof(flag),IPC_CREAT | 0755);
		printf("shmget key_flag:%d:%d\n",flag_key,shm_id_flag);
		flag = (state*)shmat(shm_id_flag,0,0);
		for(i = 0; i <= n;i++) {
			printf("Master: flag[%d]: %d\n",i,flag[i]);
		}
		shmdt(flag);
		// check turn
		printf("Master checking turn ID: %d\n",shm_id);
		turnptr = (int*)shmat(shm_id,0,0);	
		memcpy(&turn,turnptr,sizeof(intptr_t));
		printf("Master-turn: %d\n",turn);
		shmdt(turnptr);
		// release shared memory for turn intptr_t
		//printf("Removing- turn  ID: %d\n",shm_id);
		if((shmctl(shm_id, IPC_RMID, NULL)) == -1){
			perror("shmctl");
			exit(1);
		}
		// release array shared memory
		//printf("Removing ID: shm_id_flag: %d\n",shm_id_flag);
		if((shmctl(shm_id_flag, IPC_RMID, NULL)) == -1 ) {
			perror("shmctl-flag");	
			exit(1);
		}
		// free argument memory for shm_id transfer and process num transfer 
		free(arg2);
		free(arg3);
	} // end else for pid > 0 -> parent process actions
}

