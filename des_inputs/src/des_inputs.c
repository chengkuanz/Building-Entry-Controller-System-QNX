// des_inputs.c
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/iomsg.h>
#include "../../des_controller/des.h"

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <controller_pid>\n", argv[0]);
		return EXIT_FAILURE;
	}

	//create connection with controller
	int coid = ConnectAttach(ND_LOCAL_NODE, atoi(argv[1]), 1, _NTO_SIDE_CHANNEL,
			0);
	if (coid == -1) {
		printf("Connection Error!!!\n");
		return EXIT_FAILURE;
	}
	Person person;
	char event[256];

	while (1) {
		printf(
				"\nEnter the event type (ls= left scan, rs= right scan, ws= weight scale, lo =left open, ro=right open, lc = left closed, rc = right closed , gru = guard right unlock, grl = guardright lock, gll=guard left lock, glu = guard left unlock, exit = exit programs)\n");
		scanf(" %s", event);

		if (strcmp(event, "ls") == 0) {
			printf("\n Enter the person's id: \n");
			scanf("%d", &person.person_id);
			person.person_state = LEFT_SCAN;

		}
		if (strcmp(event, "rs") == 0) {
			printf("\n Enter the person's id: \n");
			scanf("%d", &person.person_id);
			person.person_state = RIGHT_SCAN;

		} else if (strcmp(event, "ws") == 0) {
			printf("\n Enter the person's weight: \n");
			scanf("%d", &person.person_weight);
			person.person_state = WEIGHT_SCALE;
		}

		else if (strcmp(event, "exit") == 0) {
			printf("Inputs Exiting...\n");
			strcpy(person.event, event);
			MsgSend(coid, &person, sizeof(person), NULL, 0);
			break;
		}

		strcpy(person.event, event);

		//sent event to controller
		if (MsgSend(coid, &person, sizeof(person), NULL, 0) == -1) {
			perror("Failed to send message\n");
			exit(EXIT_FAILURE);
		}

	}
	// Detach the connection
	ConnectDetach(coid);
	return EXIT_SUCCESS;
}
