# Rotational Dosing System (8051 Assembly)

##Overview

This project implements a rotational dosing system using an 8051 microcontroller. The system simulates an industrial mechanism that dispenses a fixed quantity of screws by controlling the rotation of a DC motor.

The implementation focuses on low-level embedded programming, emphasizing direct hardware interaction, deterministic control, and efficient use of microcontroller resources through Assembly language.

##Objectives

The main objectives of this project are:

*Understanding the internal architecture of the 8051 microcontroller
*Developing low-level programming skills using Assembly
*Utilize concepts such as timers, interrupts, and digital I/O
*Simulate hardware behavior using the EdSim51 environment

##System Description

The system controls a motor-driven rotational  with the following behavior:

The motor performs exactly 10 rotations per dosing cycle
Each rotation generates a pulse counted by the microcontroller
A switch defines the direction of rotation
A 7-segment display shows the current number of rotations (0–9)
The decimal point of the display indicates the rotation direction
When the direction is changed, the system resets and restarts
A reverse operation allows the system to recover from mechanical jams
Features
Rotation counting using Timer 1 in external counter mode
Real-time display of rotation count on a 7-segment display
Dynamic motor direction control via digital input
Automatic counter reset after 10 rotations
Interrupt-based control for efficient event handling
Modular code structure using subroutines and stack operations
Technologies
8051 Microcontroller (MCS-51 family)
Assembly language
EdSim51 simulator
Digital I/O interfaces (switches, display, motor control)
Project Structure
checkpoint1.asm: Implementation of basic input/output and 7-segment display control
checkpoint2.asm: Motor direction control logic using switch input
checkpoint3.asm: Rotation counting using Timer 1 as an event counter
final.asm: Fully integrated system with interrupts and reset logic
README.md: Project documentation
Implementation Details
Checkpoint 1 – Display and Input
Reading switch inputs
Mapping numeric values to 7-segment display patterns
Using DPTR and MOVC to access lookup tables in program memory
Checkpoint 2 – Direction Control
Monitoring a switch to determine motor direction
Storing the current state in a flag
Implementing subroutines for state comparison and direction change
Checkpoint 3 – Event Counting
Configuring Timer 1 as an external event counter
Counting pulses corresponding to motor rotations
Updating the display dynamically
Final Integration
Resetting the counter after 10 rotations
Resetting the system when direction changes
Implementing interrupt-driven counter control
Indicating direction through the display decimal point
System Behavior

At runtime, the system continuously monitors inputs and updates outputs as follows:

The motor begins rotating in the selected direction
Each pulse increments the rotation counter
The display updates in real time from 0 to 9
Upon reaching 10 rotations, the counter resets automatically
If the direction switch is toggled:
The motor stops immediately
The counter is reset
The motor restarts in the opposite direction
Engineering Considerations
Interrupts are used to improve efficiency and avoid constant polling
Subroutines provide modularity and improve code organization
Direct register manipulation ensures precise hardware control
Reset logic guarantees consistent system behavior across state changes
Demonstration

The system is validated through simulation in EdSim51, demonstrating:

Accurate rotation counting
Proper handling of direction changes
Reliable interrupt-driven reset behavior
Learning Outcomes

This project demonstrates:

Proficiency in 8051 Assembly programming
Understanding of microcontroller architecture and peripherals
Ability to design real-time embedded systems
Experience with hardware-software integration in a simulated environment
