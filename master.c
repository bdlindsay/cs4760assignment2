#include "master.h"

const int CREATE_NUM = 5;
extern intptr_t turn = 0;
extern state *flag = idle;

main() {
	char *arg1 = "slave";
	char *arg2 = malloc(sizeof(int));
	int pid;
	int key = 1;
	int shm_id[CREATE_NUM];
	int act_procs = 0;
	int i = 0;
	//int create_num = 5;
	int flag_key = 21;
	int shm_id_flag;

	// init shm_id[]
	memset(shm_id,0,sizeof(shm_id));

	// create sharded flag array
	shm_id_flag = shmget(flag_key, sizeof(flag),IPC_CREAT | 0755);
	// error checking
	if (shm_id_flag == -1) {
		perror("shmget:");
		exit(1);
	}
	printf("shmget key_flag:%d:%d\n",flag_key,shm_id_flag);
	flag = (state*)shmat(shm_id_flag,0,0);
	for(i = 0; i < CREATE_NUM; i++) {
		flag[i] = idle;
	}
	shmdt(flag);
	for(key = 1; key <= CREATE_NUM; key++) {
	  shm_id[(key-1)] = shmget(key, sizeof(intptr_t), IPC_CREAT | 0755);
		printf("shmget key:%d:%d\n",key,shm_id[(key-1)]);
		if (shm_id[(key-1)] == -1) {
			perror("shmget:");
			exit(1);
		}
		sprintf(arg2,"%d",key);
		act_procs++;
		pid = fork();
		if (pid < 0) {
			perror("fork:");
		}
		if (pid == 0) { // don't let children spawn more children
			break;
		}	
	}
	if (pid == 0) {
		// in the a child process
		execl("slave", arg1, arg2, 0);
	}
	else if (pid > 0) {
		for(i = 0; i < CREATE_NUM; i++) {
			wait();
			act_procs--;
		}
		printf("In master-");
		printf("processes=%d\n", act_procs);
		// check flag	
		shm_id_flag = shmget(flag_key, sizeof(flag),IPC_CREAT | 0755);
		printf("shmget key_flag:%d:%d\n",flag_key,shm_id_flag);
		flag = (state*)shmat(shm_id_flag,0,0);
		for(i = 1; i <=CREATE_NUM;i++) {
			printf("Master: flag[%d]: %d\n",i,flag[i]);
		}
		shmdt(flag);
		// check turn
		printf("Master checking turn ID: %d\n",shm_id[0]);
		turn = (intptr_t)shmat(shm_id[0],0,0);	
		printf("Master-turn: %d\n",turn);
		shmdt(&turn);
		// release shared memory
		for (i = 0; i < CREATE_NUM; i++) {
			printf("Removing-%d ID: %d\n",i,shm_id[i]);
			if((shmctl(shm_id[i], IPC_RMID, NULL)) == -1){
				perror("shmctl");
				exit(1);
			}
		}	
		// release array shared memory
		printf("Removing ID: shm_id_flag: %d\n",shm_id_flag);
		if((shmctl(shm_id_flag, IPC_RMID, NULL)) == -1 ) {
			perror("shmctl-flag");	
			exit(1);
		}
	}
	// free argument memory for id number transfer
	free(arg2);
}

