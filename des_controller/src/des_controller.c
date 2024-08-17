// des_controller.c
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/iomsg.h>
#include "../des.h"

void handleLeftScan();
void handleRightScan();
void handleWeightScale();
void handleLeftOpen();
void handleRightOpen();
void handleLeftClose();
void handleRightClose();
void handleGuardLeftUnlock();
void handleGuardLeftLock();
void handleGuardRightUnlock();
void handleGuardRightLock();

ControllerStatus status = { 1, 1, 1, 1 }; // Initialize all doors closed and locked

StateHandler currentStateHandler;
int coid;
Person person;
PersonState ps = OUTSIDE;
int enterFrom; //left = 0, right = 1

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <display_pid>\n", argv[0]);
		return EXIT_FAILURE;
	}

	int rcvid;
	int msgRply = 0;

	// Create channel for communication with inputs
	int chid = ChannelCreate(0);
	if (chid == -1) {
		perror("\nFailed to create the channel.");
		return EXIT_FAILURE;
	}

	// Connect to display
	coid = ConnectAttach(ND_LOCAL_NODE, atoi(argv[1]), 1, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1) {
		printf("\n Display Connection Error!!! ");
		ChannelDestroy(chid);
		return EXIT_FAILURE;
	}

	printf("The controller is running as PID: %d\n", getpid());

	// Set initial state handler
	currentStateHandler = handleLeftScan;

	while (1) {

		rcvid = MsgReceive(chid, &person, sizeof(person), NULL);
		if (rcvid == -1) {
			fprintf(stderr, "\n Receiving message Error!!! \n");
			perror(NULL);
			break;
		}

		if (strcmp(person.event, "exit") == 0) {
			printf("Received exit command. Exiting... \n");
			person.person_state = EXIT;
			MsgSend(coid, &person, sizeof(person), NULL, 0);
			break;
		}

		//state accepting functions in the controller set currentStateHandler to point to the
		//function representing the newly accepted state
		//and controller does call that function at runtime.
		currentStateHandler();

		if (MsgReply(rcvid, 1, &msgRply, sizeof(msgRply)) == -1) {
			printf("\nError replying to Inputs. \n");
			return EXIT_FAILURE;
		}
	}

	ConnectDetach(coid);
	ChannelDestroy(chid);
	return EXIT_SUCCESS;
}

void handleLeftScan() {
	if ((strcmp(person.event, "ls") == 0) && status.LDC == 1 && status.LDL == 1
			&& status.RDC == 1 && status.RDL == 1
			&& (ps == OUTSIDE || ps == INSIDE)) {
		person.person_state = LEFT_SCAN;
		ps = GETIN;
		enterFrom = 0;
		if (MsgSend(coid, &person, sizeof(person), NULL, 0) == -1L) {
			perror("Controller failed to send message\n");
			exit(EXIT_FAILURE);
		}

		//next state
		currentStateHandler = handleGuardLeftUnlock;
	}

}

void handleGuardLeftUnlock() {
	if ((strcmp(person.event, "glu") == 0) && status.LDC == 1 && status.LDL == 1
			&& status.RDC == 1 && status.RDL == 1
			&& (ps == GETIN || ps == OUTSIDE)) {
		status.LDL = 0;
		person.person_state = LEFT_UNLOCK;
		ps = GETIN;
		if (MsgSend(coid, &person, sizeof(person), NULL, 0) == -1L) {
			perror("Controller failed to send message\n");
			exit(EXIT_FAILURE);
		}
		currentStateHandler = handleLeftOpen;
	}
}

void handleLeftOpen() {
	if ((strcmp(person.event, "lo") == 0) && status.LDC == 1 && status.LDL == 0
			&& status.RDC == 1 && status.RDL == 1 && ps == GETIN) {
		status.LDC = 0;
		person.person_state = LEFT_OPEN;
		if (MsgSend(coid, &person, sizeof(person), NULL, 0) == -1L) {
			perror("Controller failed to send message\n");
			exit(EXIT_FAILURE);
		}
		if (enterFrom == 0) {
			currentStateHandler = handleWeightScale;
		} else {
			currentStateHandler = handleLeftClose;
		}
	}
}

void handleWeightScale() {
	if ((strcmp(person.event, "ws") == 0) && (ps == GETIN || ps == GETOUT)) {
		person.person_state = WEIGHT_SCALE;
		if (MsgSend(coid, &person, sizeof(person), NULL, 0) == -1L) {
			perror("Controller failed to send message\n");
			exit(EXIT_FAILURE);
		}
		if (enterFrom == 1) {
			currentStateHandler = handleRightClose;
		} else {
			currentStateHandler = handleLeftClose;
		}
	}
}

void handleLeftClose() {
	if ((strcmp(person.event, "lc") == 0) && status.LDC == 0 && status.LDL == 0
			&& status.RDC == 1 && status.RDL == 1 && ps == GETIN) {
		status.LDC = 1;
		person.person_state = LEFT_CLOSED;
		if (MsgSend(coid, &person, sizeof(person), NULL, 0) == -1L) {
			perror("Controller failed to send message\n");
			exit(EXIT_FAILURE);
		}
		currentStateHandler = handleGuardLeftLock;
	}

}

void handleGuardLeftLock() {
	if ((strcmp(person.event, "gll") == 0) && status.LDC == 1 && status.LDL == 0
			&& status.RDC == 1 && status.RDL == 1 && ps == GETIN) {
		status.LDL = 1;
		person.person_state = LEFT_LOCK;
		ps = INSIDE;
		if (MsgSend(coid, &person, sizeof(person), NULL, 0) == -1L) {
			perror("Controller failed to send message\n");
			exit(EXIT_FAILURE);
		}
		if (enterFrom == 1) {
			currentStateHandler = handleLeftScan;
		} else {
			currentStateHandler = handleGuardRightUnlock;
		}
	}
}

void handleGuardRightUnlock() {
	if ((strcmp(person.event, "gru") == 0) && status.LDC == 1 && status.LDL == 1
			&& status.RDC == 1 && status.RDL == 1
			&& (ps == GETOUT || ps == INSIDE)) {
		status.RDL = 0;
		person.person_state = RIGHT_UNLOCK;
		ps = GETOUT;
		if (MsgSend(coid, &person, sizeof(person), NULL, 0) == -1L) {
			perror("Controller failed to send message\n");
			exit(EXIT_FAILURE);
		}
		currentStateHandler = handleRightOpen;
	}
}

void handleRightOpen() {
	if ((strcmp(person.event, "ro") == 0) && status.LDC == 1 && status.LDL == 1
			&& status.RDC == 1 && status.RDL == 0 && ps == GETOUT) {
		status.RDC = 0;
		person.person_state = RIGHT_OPEN;
		if (MsgSend(coid, &person, sizeof(person), NULL, 0) == -1L) {
			perror("Controller failed to send message\n");
			exit(EXIT_FAILURE);
		}
		if (enterFrom == 1) {
			currentStateHandler = handleWeightScale;
		} else {
			currentStateHandler = handleRightClose;
		}
	}
}

void handleRightClose() {
	if ((strcmp(person.event, "rc") == 0) && status.LDC == 1 && status.LDL == 1
			&& status.RDC == 0 && status.RDL == 0 && ps == GETOUT) {
		status.RDC = 1;
		person.person_state = RIGHT_CLOSED;
		if (MsgSend(coid, &person, sizeof(person), NULL, 0) == -1L) {
			perror("Controller failed to send message\n");
			exit(EXIT_FAILURE);
		}
		currentStateHandler = handleGuardRightLock;
	}

}

void handleGuardRightLock() {
	if ((strcmp(person.event, "grl") == 0) && status.LDC == 1 && status.LDL == 1
			&& status.RDC == 1 && status.RDL == 0 && ps == GETOUT) {
		status.RDL = 1;
		person.person_state = RIGHT_LOCK;
		ps = OUTSIDE;
		if (MsgSend(coid, &person, sizeof(person), NULL, 0) == -1L) {
			perror("Controller failed to send message\n");
			exit(EXIT_FAILURE);
		}
		if (enterFrom == 0) {
			currentStateHandler = handleRightScan;
		} else {
			currentStateHandler = handleGuardLeftUnlock;
		}

	}

}
void handleRightScan() {
	if ((strcmp(person.event, "rs") == 0) && status.LDC == 1 && status.LDL == 1
			&& status.RDC == 1 && status.RDL == 1
			&& (ps == OUTSIDE || ps == INSIDE)) {
		person.person_state = RIGHT_SCAN;
		ps = GETOUT;
		enterFrom = 1;

		if (MsgSend(coid, &person, sizeof(person), NULL, 0) == -1L) {
			perror("Controller failed to send message\n");
			exit(EXIT_FAILURE);
		}
		currentStateHandler = handleGuardRightUnlock;
	}
}

