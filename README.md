# Sistema de Dosagem Rotativa (8051 Assembly)

## Sobre o projeto

  O projeto visa implementar um sistema de dosagem rotativa utilizando um microcontrolador da família 8051. Esse sistema simula o controle de um mecanismo industrial que dispensa uma quantidade fixa de parafusos para serem ensacodos atravez da manipulação de um motor DC. Os requisitos para o projeto são que o dosador deve girar exatamente 10 voltas e deve conter uma medida no caso de travamento, fazendo com que o motor gire no sentido contrário para destravar a linha de produção.



## Objetivos

Os principais objetivos para esse projeto são:

* Compreender a arquitetura interna de um microcontrolador 8051
* Desenvolvimento low level 
* 
* Simulate hardware behavior using the EdSim51 environment

## Descrição do sistema

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
### <ins>Checkpoint 1 – Display and Input<ins>
Reading switch inputs
Mapping numeric values to 7-segment display patterns
Using DPTR and MOVC to access lookup tables in program memory
### <ins>Checkpoint 2 – Direction Control<ins>
Monitoring a switch to determine motor direction
Storing the current state in a flag
Implementing subroutines for state comparison and direction change
### <ins>Checkpoint 3 – Event Counting<ins>
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
