#include "master.h"
main() {
	int pid;
	char *arg1 = "slave";
	char arg2 = '4';
	pid = fork();
	if (pid < 0) {
		perror("fork fail: ");
	} else if (pid > 0) {
		printf("In master-");
		wait();
		printf("Parent process\n");
	} else if (pid == 0) {
		// in the other process
		printf("In other process");
		execl("slave", arg1, &arg2);
	}
}

