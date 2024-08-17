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

	int rply = 0;
	int rcvid;
	Person person;

	printf("The display is running as process_id: %d\n", getpid());

	// Create channel for communication with controller
	int chid = ChannelCreate(0);
	if (chid == -1) {
		perror("\nFailed to create the channel. ");
		exit(EXIT_FAILURE);
	}

	while (1) {
		//receive message from controller
		rcvid = MsgReceive(chid, &person, sizeof(person), NULL);

		if (person.person_state == EXIT) {
			printf(displayMessage[person.person_state]);
			break;
		} else if (person.person_state == LEFT_SCAN
				|| person.person_state == RIGHT_SCAN) {
			printf("Person scanned ID, ID = %d \n", person.person_id);
		} else if (person.person_state == WEIGHT_SCALE) {
			printf("Person weighed, Weight = %d \n", person.person_weight);
		} else {
			printf("%s \n", displayMessage[person.person_state]);
		}

		if (MsgReply(rcvid, EOK, &person, sizeof(person)) == -1) {
			perror("Failed to send message\n");
			exit(EXIT_FAILURE);
		}

	}

	ChannelDestroy(chid);
	return EXIT_SUCCESS;
}
