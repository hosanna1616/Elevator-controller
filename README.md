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
2.2 Circuit Design Principles
Power Distribution
[5V Source]
├── ATmega32 VCC pins (10, 30)
├── 4511 VCC pin (16)
├── All 10kΩ pull-up resistors
└── (Indirectly powers LEDs via microcontroller)
[Ground Network]
├── ATmega32 GND pins (11, 31)
├── 4511 GND pin (8)
├── All button switches
├── All LED cathodes
└── 7-segment common cathodes
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

Elevator State Machine
typedef enum {
STATE_IDLE, // Waiting for requests, door closed
STATE_MOVING_UP, // Ascending between floors
STATE_MOVING_DOWN, // Descending between floors
STATE_DOOR_OPENING, // Door opening sequence
STATE_DOOR_OPEN, // Door fully open
STATE_DOOR_CLOSING // Door closing sequence
} ElevatorState_t;
Movement Direction
typedef enum {
DIR_IDLE, // No movement
DIR_UP, // Moving upward
DIR_DOWN // Moving downward
} Direction_t;
✅ Data Structures
Request Tracking System
// Three parallel arrays track different request types
uint8_t internal_requests[5]; // Floor selections inside elevator
uint8_t external_up[5]; // UP calls from outside
uint8_t external_down[5]; // DOWN calls from outside
// Each array index corresponds to floor number (0-4)
// Value 1 = request pending, 0 = no request
System State Variables
uint8_t current_floor; // Current position (0-4)
Direction_t current_direction; // Current movement direction
ElevatorState_t elevator_state; // Current operational state
uint8_t target_floor; // Destination floor
uint8_t door_timer_counter; // Door operation timing
uint8_t travel_timer_counter; // Inter-floor travel timing
✅ Timing Configuration
Constant Value Purpose
TRAVEL_TIME_MS 2000 Time between floors (2 seconds)
DOOR_OPEN_TIME 3000 Door fully open duration (3 seconds)
DOOR_CLOSE_TIME 1000 Door opening/closing time (1 second)
MAIN_LOOP_DELAY 100 Main loop period (100ms)
✅ Module Organization
Main Program (elevator_controller.c)
├── Button Scanning Module
│ ├── scan_buttons()
│ ├── any_requests_pending()
│ └── clear_floor_request()
│
├── Display Control Module
│ ├── update_display()
│ ├── update_direction_leds()
│ └── update_door_leds()
│
├── Logic Processing Module
│ ├── find_highest_priority_request()
│ ├── should_stop_at_floor()
│ ├── find_next_request_in_direction()
│ └── check_external_call_at_current_floor()
│
├── Movement Control Module
│ ├── move_to_next_floor()
│ ├── begin_door_opening()
│ ├── begin_door_closing()
│ └── is_door_closed()
│
├── State Machine Module
│ └── elevator_state_machine()
│
└── Safety Module
├── is_safe_to_move()
└── emergency_stop()
✅ Core Algorithm Implementation

The elevator implements a four-rule priority system:
Rule 1: Complete Current Direction First
if (current_direction == UP) {
serve_all_UP_requests_before_reversing();
} else if (current_direction == DOWN) {
serve_all_DOWN_requests_before_reversing();
}
Rule 2: Nearest-First Within Same Direction
while (requests_exist_in_current_direction()) {
target = find_nearest_floor_in_current_direction();
move_to(target);
}
Rule 3: First-Come-First-Serve for Different Directions
first_request = get_oldest_pending_request();
initial_direction = determine_direction(first_request);
serve_all_in_direction(initial_direction);
Rule 4: Immediate Response to External Calls at Current Floor
if (external_button_pressed_at(current_floor) && door_closed) {
open_door_immediately();
}
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

