#include "master.h"

typedef enum {idle, want_in, in_cs} state;
extern state flag[]; // flag for each process in shared memory
extern int turn = 5; 

main (int argc, char *argv[]) {
	printf("In slave: turn: %d\n", turn);
	turn = atoi(argv[1]);
	printf("In slave: turn: %d\n", turn);
}
/*
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
	critical_section();
	numWrites++;
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
} // end process() 
*/
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
	// write to file
	fprintf(fp, "%s", msg);
	// clean up
	free(msg);
	fclose(fp);
}
