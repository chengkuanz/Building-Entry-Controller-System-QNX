#ifndef DES_H
#define DES_H

#define NUM 12

#define LEFT_SCAN 0
#define RIGHT_SCAN 1
#define WEIGHT_SCALE 2
#define LEFT_OPEN 3
#define RIGHT_OPEN 4
#define LEFT_CLOSED 5
#define RIGHT_CLOSED 6
#define LEFT_UNLOCK 7
#define LEFT_LOCK 8
#define RIGHT_UNLOCK 9
#define RIGHT_LOCK 10
#define EXIT 11


const char *displayMessage[NUM] = {
		"Left Scan",
		"Right Scan",
		"Weight Scale",
		"left door Opened",
		"Right door Opened",
		"Left door closed (automatically)",
		"Right door closed (automatically)",
		"Left door Unlocked by guard",
		"Left door Locked by guard",
		"Right door Unlocked by Guard",
		"Right door Locked by guard",
		"Display Exiting..."
};

typedef struct {
    int LDC;
    int LDL;
    int RDC;
    int RDL;
} ControllerStatus;


typedef struct {
	int person_id;
	int person_weight;
	int person_state;
	char event[20];
} Person;

typedef enum {
	OUTSIDE = 0, GETIN = 1, INSIDE = 2, GETOUT = 3
} PersonState;

typedef void (*StateHandler)();

#endif
