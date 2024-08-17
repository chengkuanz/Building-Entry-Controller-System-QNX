# Building Entry Controller System - QNX

## Overview

This project is a simulation of a building entry controller system designed as part of the CST8244 - Real-Time Programming course. The system manages the entry and exit of individuals through a controlled two-door system using card scanners, a scale to weigh individuals, and manual guard-operated locks. The project is implemented in C and runs on the QNX Neutrino RTOS. The system is designed to handle multiple tasks concurrently, including card scanning, weight measurement, and door control, while ensuring real-time responsiveness and data integrity.

The project demonstrates the use of semaphores, message passing, and shared memory to synchronize tasks and communicate between processes. The system is tested for various scenarios, including invalid card scans, weight limit violations, and concurrent access to shared resources. The project aims to provide a reliable and efficient solution for managing building entry control systems in real-time environments.

## System Components

- **Card Scanners**: Located at each door, used to scan a unique personal ID card.
- **Doors**: Two doors (left and right) that control access to the building. Each door is self-closing but not self-locking.
- **Weight Scale**: Measures the weight of the person after they enter the space between the two doors.
- **Human Guard**: Manages the unlocking and locking of each door via a switch.

## Project Components

The project consists of three primary programs, each responsible for a specific aspect of the building entry controller system:

- **`des_display`**: Displays the status of the system, showing which door is open/closed and other relevant information. This program runs in the background and prints status updates to the console. It responds to state changes managed by the `des_controller` program.

- **`des_controller`**: The core program that runs the state machine for the building entry controller system. It manages persistent data and state transitions, handling inputs from the `des_inputs` program and sending status updates to the `des_display` program. The main logic for the entry control system resides in the `des_controller/des_controller.c` file, while the shared data structures and constants used across the programs are defined in the `des_controller/des.h` file.

- **`des_inputs`**: Simulates input events from the devices (e.g., card readers, door latches, scale). It prompts the user for inputs and sends them as messages to the `des_controller` program. The command-line interface for simulating various input events is implemented in the `des_inputs/des_inputs.c` file.

## Usage

### `des_inputs`

- The program will prompt for various inputs representing events in the system.
- Available events: `ls`, `rs`, `ws`, `lo`, `ro`, `lc`, `rc`, `glu`, `gll`, `gru`, `grl`, `exit`.
- Depending on the event, you may need to provide additional input such as `person_id` or `weight`.

### `des_display`

- Displays the status of the system each time a status update is received from `des_controller`.

### `des_controller`

- Operates the state machine to manage the control system. Responds to inputs from `des_inputs` and updates `des_display` accordingly.

## Installation and Setup

### Prerequisites

- **QNX Momentics IDE**: The project is developed in QNX, and the QNX Momentics IDE is required to build and run the project.
- **QNX Neutrino RTOS**: The code is intended to run on the QNX Neutrino real-time operating system.

### Setting Up the Project

1. **Clone the Project**: Download or clone the project from the repository.

2. **Import the Project into QNX Momentics**:
    - Open QNX Momentics IDE.
    - Create a new workspace named `cst8244_assign1`.
    - Import the project by selecting `File > Import... > General > Existing Projects into Workspace`.

3. **Build the Project**:
    - Ensure all projects (`des_display`, `des_controller`, `des_inputs`) are included in the workspace.
    - Clean and build all projects by selecting `Project > Clean...` and then `Project > Build All`.

4. **Copy the binary files** to the target machine's `/tmp` folder.

### Running the Project

1. **Start `des_display`**:

   cd /tmp  
   ./des_display &

    - The `des_display` program will output its process ID when it starts running.

2. **Start `des_controller`**:

   cd /tmp  
   ./des_controller <display_pid> &

    - Replace `<display_pid>` with the process ID of the `des_display` program obtained in step 1.

3. **Start `des_inputs`**:

   cd /tmp  
   ./des_inputs <controller_pid>

    - Replace `<controller_pid>` with the process ID of the `des_controller` program obtained in step 2.

4. **Follow the prompts** to simulate various events and observe the system's behavior in the `des_display` terminal.

### Exiting the Programs

- To terminate the programs, enter `exit` as an event in the `des_inputs` program.

## State Machine Design

### Inputs

- **Card Scans**: Left (`ls`), Right (`rs`)
- **Door Status**: Left Open (`lo`), Left Closed (`lc`), Right Open (`ro`), Right Closed (`rc`)
- **Guard Actions**: Guard Left Unlock (`glu`), Guard Left Lock (`gll`), Guard Right Unlock (`gru`), Guard Right Lock (`grl`)
- **Weight Scale**: Weight Scale (`ws`)

### Outputs

- **Door Actions**: Open/Close/Lock/Unlock doors based on the state transitions.
- **Display Updates**: Send messages to the `des_display` program to update the current state.

### States

- **OUTSIDE**: Person is outside the building.
- **GETIN**: Person is entering the building.
- **INSIDE**: Person is inside the building.
- **GETOUT**: Person is exiting the building.

### State Transitions

- Transitions occur based on events such as card scans, door status changes, and guard actions. The state machine ensures that only one person can be in the lock at a time and that all doors are properly controlled.

### Error Handling

- The system remains in its current state when an invalid event is received, waiting for the correct input to transition to the next state.

## Sample Usage

### Scenario 1: Entering the Building

This scenario represents a person entering the building, beginning with a left scan (`ls`) of the person’s ID:

ls 12345 glu  
lo  
ws 123  
lc  
gll  
gru  
ro  
rc  
grl  
exit

### Scenario 2: LeftRightScan

This scenario represents a sequence where two individuals enter, with one using the left door and the other using the right door:

ls 12345 glu  
lo  
ws 123  
lc  
gll  
gru  
ro  
rc  
grl  
rs 54321  
gru  
ro  
ws 321  
rc  
grl  
glu  
lo  
lc  
gll  
exit

### Scenario 3: ExitScan

This scenario represents the command to exit the simulation:

exit

### Scenario 4: ErrorScan

This scenario demonstrates the system's response to invalid input events. Here, "ls 54321" is a valid input, but all subsequent inputs are invalid:

ls 54321  
lo  
ro  
lo  
ro  
letmeout  
exit  
