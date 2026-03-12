![elevator](https://github.com/user-attachments/assets/58688d36-49bf-4bd6-a3fd-e012b0606bd3)
5-Floor Elevator Controller System

This project implements a complete 5-floor elevator control system using the ATmega32
microcontroller. The system simulates real elevator operations with intelligent scheduling
algorithms that optimize passenger service based on direction and proximity.
The elevator serves floors Ground (0) through 4, with comprehensive user interfaces both inside
and outside the elevator. The system is fully simulated in Proteus 8 with C code developed in
Microchip Studio, demonstrating professional embedded system design principles.
✅ Core Features
● Complete 5-floor operation with Ground, 1st, 2nd, 3rd, and 4th floors
● Dual interface system:
● Internal controls: 5 buttons for floor selection inside the elevator
● External controls: 8 buttons for calling the elevator (4 UP, 4 DOWN)
● Intelligent scheduling algorithm that prioritizes:
● Nearest floor first when moving in same direction
● Completion of current direction before reversing
● Chronological order for different directions
● Real-time visual feedback:
● 7-segment display showing current floor
● Direction indicators (UP/DOWN LEDs)
● Door status indicators (Open/Closed LEDs)
● Automated door operations with timed opening, holding, and closing sequences
● Safety mechanisms including door-closed verification before movement
✅ Technical Specifications
Category Specification
Microcontroller ATmega32 (8-bit AVR)
Clock Source Internal 1MHz RC Oscillator
Programming Language Embedded C
Development Environment Microchip Studio
Simulation Software Proteus 8 Professional
Operating Voltage 5V DC
Memory Usage <2KB Flash, <100 bytes SRAM
Response Time <100ms button detection
Floor Travel Time 2 seconds
Door Open Time 3 seconds
Door Operation Time 1 second (opening/closing)
✅ Hardware Design
● ATmega32 1 Main controller U1
● 4511 BCD
● Decoder
● 1 7-segment driver U2
● 7-SEG-COM-CAT 1 Floor display DISP1
● Push Button 13 User input SW1-SW13
● LED-GREEN 1 UP direction D2
● LED-RED 1 DOWN direction D3
● LED-YELLOW 1 Door open D4
● LED-BLUE 1 Door closed D5
● 220Ω Resistor 11 Current limiting R1-R11
● 10kΩ Resistor 13 Pull-up resistors R12-R24
● POWER Terminal 1 5V supply VCC
● GROUND Terminal 1 Common ground GND
● Peripheral Connections
● Display Subsystem
● ATmega32 → 4511 Decoder → 7-Segment Display
● PORTA[0:3] → BCD Inputs → Segment Outputs
(4 wires) (A,B,C,D) (a,b,c,d,e,f,g)
● Button Matrix
● Internal Buttons (5): PB0-PB4 → Floors 0-4
● External UP (4): PC0-PC3 → Floors 0-3 UP
● External DOWN (4): PC4-PC7 → Floors 1-4 DOWN
● LED Indicators
PD0 → UP LED (Green) → 220Ω → GND
PD1 → DOWN LED (Red) → 220Ω → GND
PD6 → DOOR OPEN (Yellow) → 220Ω → GND
PD7 → DOOR CLOSED (Blue) → 220Ω → GND

Input Protection
● 10kΩ pull-up resistors on all button inputs prevent floating pins
● Active-low configuration (pressed = LOW, not pressed = HIGH) for noise immunity
● Direct GND connection when button pressed creates clean LOW signal
Output Protection
● 220Ω series resistors limit LED current to ~15mA (safe operating range)
● BCD output through 4511 provides buffer between microcontroller and display
● Port initialization ensures known states on power-up
✅ Pin Allocation Strategy
Port Pin Function Direction Description
PORTA PA0 BCD Bit 0 (LSB) Output To 4511 Pin 7 (A)
PA1 BCD Bit 1 Output To 4511 Pin 1 (B)
PA2 BCD Bit 2 Output To 4511 Pin 2 (C)
PA3 BCD Bit 3 (MSB) Output To 4511 Pin 6 (D)
PORTB PB0 Internal Floor 0 Input Ground floor
button
PB1 Internal Floor 1 Input 1st floor button
PB2 Internal Floor 2 Input 2nd floor button
PB3 Internal Floor 3 Input 3rd floor button
PB4 Internal Floor 4 Input 4th floor button
PORTC PC0 External UP Floor 0 Input Ground floor UP
call
PC1 External UP Floor 1 Input 1st floor UP call
PC2 External UP Floor 2 Input 2nd floor UP call
PC3 External UP Floor 3 Input 3rd floor UP call
PC4 External DOWN
Floor 1
Input 1st floor DOWN
call
PC5 External DOWN
Floor 2
Input 2nd floor DOWN
call
PC6 External DOWN
Floor 3
Input 3rd floor DOWN
call
PC7 External DOWN
Floor 4
Input 4th floor DOWN
call
PORTD PD0 UP Direction LED Output Green LED
indicator
PD1 DOWN Direction
LED
Output Red LED
indicator
PD6 Door Open LED Output Yellow indicator
PD7 Door Closed LED Output Blue indicator
✅ Software Architecture

✅ Request Processing Algorithm
Algorithm: Elevator Scheduling
Input: Button presses from internal and external interfaces
Output: Movement sequence and door operations
1. Initialize system: floor=0, direction=IDLE, state=IDLE
2. Loop every 100ms:
a. Scan all buttons, update request arrays
b. Check for external call at current floor
If found: open door immediately, clear request
c. If any requests pending:
i. Determine target floor using priority rules
ii. If target > current: set direction=UP
iii. If target < current: set direction=DOWN
iv. If target = current: open door
v. Move floor-by-floor, checking intermediate stops
d. Update displays and LEDs
e. Handle door timing operations
f. Implement safety checks
3. Continue loop
4.3 Key Functions Implementation
4.3.1 Button Scanning Function
4.3.2 Priority Calculation Function
4.3.3 External Call at Current Floor Detection

