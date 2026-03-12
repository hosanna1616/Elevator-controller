#include <avr/io.h>
#include <util/delay.h>
// ============ BUTTON PIN DEFINITIONS ============
// Internal buttons (PORTB)
#define INT_BTN_0 PB0 // Ground floor (0)
#define INT_BTN_1 PB1 // 1st floor
#define INT_BTN_2 PB2 // 2nd floor
#define INT_BTN_3 PB3 // 3rd floor
#define INT_BTN_4 PB4 // 4th floor
// External UP buttons (PORTC lower nibble)
#define EXT_UP_0 PC0 // Ground floor UP
#define EXT_UP_1 PC1 // 1st floor UP
#define EXT_UP_2 PC2 // 2nd floor UP
#define EXT_UP_3 PC3 // 3rd floor UP
// External DOWN buttons (PORTC upper nibble)
#define EXT_DOWN_1 PC4 // 1st floor DOWN
#define EXT_DOWN_2 PC5 // 2nd floor DOWN
#define EXT_DOWN_3 PC6 // 3rd floor DOWN
#define EXT_DOWN_4 PC7 // 4th floor DOWN
// LED definitions (PORTD)
#define UP_LED_PIN PD0
#define DOWN_LED_PIN PD1
#define DOOR_OPEN_LED PD6
#define DOOR_CLOSE_LED PD7
// ============ TIMING CONSTANTS ============
#define TRAVEL_TIME_MS 2000 // 2 seconds between floors
#define DOOR_OPEN_TIME 3000 // 3 seconds door open
#define DOOR_CLOSE_TIME 1000 // 1 second door close
#define MAIN_LOOP_DELAY 100 // 100ms main loop delay
// ============ ELEVATOR STATES ============
typedef enum {
STATE_IDLE,
STATE_MOVING_UP,
STATE_MOVING_DOWN,
STATE_DOOR_OPENING,
STATE_DOOR_OPEN,
STATE_DOOR_CLOSING
} ElevatorState_t;
typedef enum {
DIR_IDLE,
DIR_UP,
DIR_DOWN
} Direction_t;
// ============ GLOBAL VARIABLES ============
uint8_t current_floor = 0;
Direction_t current_direction = DIR_IDLE;
ElevatorState_t elevator_state = STATE_IDLE;
uint8_t target_floor = 0;
// Request tracking
uint8_t internal_requests[5] = {0, 0, 0, 0, 0};
uint8_t external_up[5] = {0, 0, 0, 0, 0};
uint8_t external_down[5] = {0, 0, 0, 0, 0};
// Timing counters (in multiples of MAIN_LOOP_DELAY)
uint8_t door_timer_counter = 0;
uint8_t travel_timer_counter = 0;
// ============ BUTTON SCANNING FUNCTIONS ============
void scan_buttons(void)
{
// Read internal buttons (PORTB)
uint8_t portb_val = PINB;
for(uint8_t i = 0; i < 5; i++)
{
if(!(portb_val & (1 << i)))
{
internal_requests[i] = 1;
}
}
// Read external buttons (PORTC)
uint8_t portc_val = PINC;
// External UP buttons (PC0-PC3)
for(uint8_t i = 0; i < 4; i++)
{
if(!(portc_val & (1 << i)))
{
external_up[i] = 1;
}
}
// External DOWN buttons (PC4-PC7)
// Note: Array index 0-4, but buttons are for floors 1-4
for(uint8_t i = 4; i < 8; i++)
{
if(!(portc_val & (1 << i)))
{
if(i == 4) external_down[1] = 1; // PC4 -> floor 1
else if(i == 5) external_down[2] = 1; // PC5 -> floor 2
else if(i == 6) external_down[3] = 1; // PC6 -> floor 3
else if(i == 7) external_down[4] = 1; // PC7 -> floor 4
}
}
}
void clear_floor_request(uint8_t floor)
{
internal_requests[floor] = 0;
external_up[floor] = 0;
external_down[floor] = 0;
}
uint8_t any_requests_pending(void)
{
for(uint8_t i = 0; i < 5; i++)
{
if(internal_requests[i] || external_up[i] || external_down[i])
return 1;
}
return 0;
}
// ============ DISPLAY & LED FUNCTIONS ============
void update_display(uint8_t floor)
{
PORTA = floor; // Assuming 7-segment decoder connected to PORTA
}
void update_direction_leds(Direction_t dir)
{
if(dir == DIR_UP)
{
PORTD |= (1 << UP_LED_PIN);
PORTD &= ~(1 << DOWN_LED_PIN);
}
else if(dir == DIR_DOWN)
{
PORTD &= ~(1 << UP_LED_PIN);
PORTD |= (1 << DOWN_LED_PIN);
}
else
{
PORTD &= ~((1 << UP_LED_PIN) | (1 << DOWN_LED_PIN));
}
}
void update_door_leds(uint8_t door_open)
{
if(door_open)
{
PORTD |= (1 << DOOR_OPEN_LED);
PORTD &= ~(1 << DOOR_CLOSE_LED);
}
else
{
PORTD &= ~(1 << DOOR_OPEN_LED);
PORTD |= (1 << DOOR_CLOSE_LED);
}
}
uint8_t is_door_closed(void)
{
// Door is closed in IDLE and MOVING states
return (elevator_state == STATE_IDLE ||
elevator_state == STATE_MOVING_UP ||
elevator_state == STATE_MOVING_DOWN);
}
// ============ NEW FUNCTION: Check for external call at current floor ============
uint8_t check_external_call_at_current_floor(void)
{
// Check if there's an external UP or DOWN request for the current floor
// Only when door is closed and elevator is idle
if(is_door_closed() && elevator_state == STATE_IDLE)
{
// For current floor 0, only check external UP request
if(current_floor == 0)
{
if(external_up[0])
return 1;
}
// For current floor 4, only check external DOWN request
else if(current_floor == 4)
{
if(external_down[4])
return 1;
}
// For floors 1-3, check both UP and DOWN requests
else
{
if(external_up[current_floor] || external_down[current_floor])
return 1;
}
}
return 0;
}
// ============ ELEVATOR LOGIC FUNCTIONS ============
uint8_t should_stop_at_floor(uint8_t floor)
{
// Check if there's any request for this floor that aligns with our direction
if(internal_requests[floor])
return 1;
if(current_direction == DIR_UP)
{
// When going UP, stop for UP requests at or above current floor
if(external_up[floor] && floor >= current_floor)
return 1;
}
else if(current_direction == DIR_DOWN)
{
// When going DOWN, stop for DOWN requests at or below current floor
if(external_down[floor] && floor <= current_floor)
return 1;
}
else if(current_direction == DIR_IDLE)
{
// When idle, stop for any external request
if(external_up[floor] || external_down[floor])
return 1;
}
return 0;
}
int8_t find_next_request_in_direction(Direction_t dir)
{
if(dir == DIR_UP)
{
// Search floors above current floor
for(uint8_t floor = current_floor + 1; floor < 5; floor++)
{
if(internal_requests[floor] || external_up[floor])
return floor;
}
return -1;
}
else if(dir == DIR_DOWN)
{
// Search floors below current floor
for(int8_t floor = current_floor - 1; floor >= 0; floor--)
{
if(internal_requests[floor] || external_down[floor])
return floor;
}
return -1;
}
return -1;
}
int8_t find_highest_priority_request(void)
{
// Logic 1: Complete current direction first
if(current_direction == DIR_UP)
{
// First check for requests above in UP direction
for(uint8_t floor = current_floor + 1; floor < 5; floor++)
{
if(internal_requests[floor] || external_up[floor])
return floor;
}
// If no UP requests above, check for any DOWN requests
for(int8_t floor = 4; floor >= 0; floor--)
{
if(internal_requests[floor] || external_down[floor])
return floor;
}
}
else if(current_direction == DIR_DOWN)
{
// First check for requests below in DOWN direction
for(int8_t floor = current_floor - 1; floor >= 0; floor--)
{
if(internal_requests[floor] || external_down[floor])
return floor;
}
// If no DOWN requests below, check for any UP requests
for(uint8_t floor = 0; floor < 5; floor++)
{
if(internal_requests[floor] || external_up[floor])
return floor;
}
}
else // DIR_IDLE
{
// Find closest request
int8_t closest_up = -1;
int8_t closest_down = -1;
// Check above
for(uint8_t floor = current_floor + 1; floor < 5; floor++)
{
if(internal_requests[floor] || external_up[floor] || external_down[floor])
{
closest_up = floor;
break;
}
}
// Check below
for(int8_t floor = current_floor - 1; floor >= 0; floor--)
{
if(internal_requests[floor] || external_up[floor] || external_down[floor])
{
closest_down = floor;
break;
}
}
// Choose closest
if(closest_up >= 0 && closest_down >= 0)
{
uint8_t dist_up = closest_up - current_floor;
uint8_t dist_down = current_floor - closest_down;
return (dist_up <= dist_down) ? closest_up : closest_down;
}
else if(closest_up >= 0)
return closest_up;
else if(closest_down >= 0)
return closest_down;
}
return -1;
}
void begin_door_opening(void)
{
elevator_state = STATE_DOOR_OPENING;
// Close LED stays ON during opening
update_door_leds(0);
door_timer_counter = DOOR_CLOSE_TIME / MAIN_LOOP_DELAY;
}
void begin_door_closing(void)
{
elevator_state = STATE_DOOR_CLOSING;
// Close LED stays ON during closing
update_door_leds(0);
door_timer_counter = DOOR_CLOSE_TIME / MAIN_LOOP_DELAY;
}
void move_to_next_floor(void)
{
// Safety check: Only move if door is closed
if(!is_door_closed())
return;
if(current_direction == DIR_UP && current_floor < 4)
{
current_floor++;
elevator_state = STATE_MOVING_UP;
}
else if(current_direction == DIR_DOWN && current_floor > 0)
{
current_floor--;
elevator_state = STATE_MOVING_DOWN;
}
update_display(current_floor);
update_direction_leds(current_direction);
// Close LED ON during movement
update_door_leds(0);
// Start travel timer
travel_timer_counter = TRAVEL_TIME_MS / MAIN_LOOP_DELAY;
}
// ============ STATE MACHINE HANDLER ============
void elevator_state_machine(void)
{
switch(elevator_state)
{
case STATE_IDLE:
// Close LED ON when idle
update_door_leds(0);
// NEW LOGIC: Check if someone pressed external button at current floor
if(check_external_call_at_current_floor())
{
// Clear the external request and open the door immediately
clear_floor_request(current_floor);
begin_door_opening();
break;
}
if(any_requests_pending())
{
int8_t next_floor = find_highest_priority_request();
if(next_floor >= 0)
{
target_floor = next_floor;
// Set direction
if(target_floor > current_floor)
current_direction = DIR_UP;
else if(target_floor < current_floor)
current_direction = DIR_DOWN;
else
{
// If current floor is requested, open door immediately
begin_door_opening();
break;
}
// Start moving (door is already closed in IDLE state)
move_to_next_floor();
}
}
break;
case STATE_MOVING_UP:
case STATE_MOVING_DOWN:
// Close LED ON during movement
update_door_leds(0);
// Check travel timer
if(travel_timer_counter > 0)
{
travel_timer_counter--;
}
else
{
// We've reached the next floor
travel_timer_counter = 0;
// Check if we should stop at this floor
if(should_stop_at_floor(current_floor))
{
// Stop and open door
elevator_state = STATE_IDLE;
current_direction = DIR_IDLE;
update_direction_leds(current_direction);
begin_door_opening();
}
else if(current_floor == target_floor)
{
// Reached target floor
elevator_state = STATE_IDLE;
current_direction = DIR_IDLE;
update_direction_leds(current_direction);
begin_door_opening();
}
else
{
// Continue to next floor
int8_t next_stop =
find_next_request_in_direction(current_direction);
if(next_stop >= 0)
{
// Update target if there's a request before current target
if((current_direction == DIR_UP && next_stop <
target_floor) ||
(current_direction == DIR_DOWN && next_stop >
target_floor))
{
target_floor = next_stop;
}
}
// Move to next floor
move_to_next_floor();
}
}
break;
case STATE_DOOR_OPENING:
// Close LED stays ON during opening
update_door_leds(0);
if(door_timer_counter > 0)
{
door_timer_counter--;
}
else
{
// Door is now open
elevator_state = STATE_DOOR_OPEN;
// Open LED ON, Close LED OFF
update_door_leds(1);
door_timer_counter = DOOR_OPEN_TIME / MAIN_LOOP_DELAY;
// Clear request for this floor (already cleared for external calls at current
floor)
clear_floor_request(current_floor);
}
break;
case STATE_DOOR_OPEN:
// Open LED ON, Close LED OFF
update_door_leds(1);
if(door_timer_counter > 0)
{
door_timer_counter--;
}
else
{
// Time to close door
begin_door_closing();
}
break;
case STATE_DOOR_CLOSING:
// Close LED ON during closing
update_door_leds(0);
if(door_timer_counter > 0)
{
door_timer_counter--;
}
else
{
// Door is now closed
elevator_state = STATE_IDLE;
current_direction = DIR_IDLE;
update_direction_leds(current_direction);
// Close LED ON, Open LED OFF
update_door_leds(0);
// Check for next request
if(any_requests_pending())
{
int8_t next_floor = find_highest_priority_request();
if(next_floor >= 0)
{
target_floor = next_floor;
if(target_floor > current_floor)
current_direction = DIR_UP;
else if(target_floor < current_floor)
current_direction = DIR_DOWN;
else
{
// Already at requested floor - open door
begin_door_opening();
break;
}
// Start moving
move_to_next_floor();
}
}
}
break;
}
}
// ============ SAFETY FUNCTIONS ============
uint8_t is_safe_to_move(void)
{
// Check if door is closed
if(!is_door_closed())
return 0;
return 1;
}
void emergency_stop(void)
{
// Stop elevator immediately
elevator_state = STATE_IDLE;
current_direction = DIR_IDLE;
update_direction_leds(current_direction);
// If moving, open door at current floor
if(travel_timer_counter > 0)
{
// We're between floors, open door at next floor
begin_door_opening();
}
}
// ============ MAIN FUNCTION ============
int main(void)
{
// ============ INITIALIZE ============
DDRA = 0x0F; // PA0-PA3 outputs for 7-segment (4 bits for 0-4 floors)
DDRB = 0x00; // PORTB inputs for internal buttons
PORTB = 0xFF; // Enable pull-ups
DDRC = 0x00; // PORTC inputs for external buttons
PORTC = 0xFF; // Enable pull-ups
DDRD = (1<<UP_LED_PIN) | (1<<DOWN_LED_PIN) |
(1<<DOOR_OPEN_LED) | (1<<DOOR_CLOSE_LED);
// Initial state
current_floor = 0;
current_direction = DIR_IDLE;
elevator_state = STATE_IDLE;
// Update all indicators
update_display(current_floor);
update_direction_leds(current_direction);
// Start with door closed (Close LED ON)
update_door_leds(0);
// Clear all requests
for(uint8_t i = 0; i < 5; i++)
{
internal_requests[i] = 0;
external_up[i] = 0;
external_down[i] = 0;
}
// Initialize timers
door_timer_counter = 0;
travel_timer_counter = 0;
// ============ MAIN LOOP ============
while(1)
{
// Scan buttons every cycle
scan_buttons();
// Run elevator state machine
elevator_state_machine();
// Safety check - ensure door is closed before moving
if((elevator_state == STATE_MOVING_UP || elevator_state ==
STATE_MOVING_DOWN) &&
!is_door_closed())
{
// Emergency stop if door opens while moving
emergency_stop();
}
// Main loop delay
_delay_ms(MAIN_LOOP_DELAY);
}
return 0;
}
