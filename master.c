#include "master.h"

extern int n = 0;
main() {
	int pid;
	int i = 0;
	char *arg1 = "slave";
	char *arg2 = malloc(sizeof(int));
	
	pid = fork();
	n++;
	sprintf(arg2,"%d",n);
	if (pid < 0) {
		perror("fork fail: ");
	} else if (pid == 0) {
		// in the other process
		execl("slave", arg1, arg2, 0);
	}
	else if (pid > 0) {
		wait();
		n--;
		printf("In master-");
		printf("processes=%d\n", n);
	}	
	free(arg2);
}

