/*
 * finalproject.c
 *
 * Author : Katie Fukuda
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <avr/eeprom.h>

#include "timer.h"
#include "scheduler.h"
#include "lcd.c"
#include "joystick.c"
#include "nokia5110.c"
#include "nokia5110.h"
#include "queue.h"

//*******GLOBAL VARIABLES********
unsigned char stats[3] = {0,0,0};
unsigned char right_press = 0;
unsigned char left_press = 0;
unsigned char down_press = 0;
unsigned char button = 0;
unsigned lcd_wait = 0;
Queue choice;

//*******USER DEFINED STATE MACHINES***********
enum lcd_states {HAPPY,  CLEAN, HUNGER, RESET};
int lcd_sm(int state) {
	//local variables

	//transitions  
	switch(state) {
		case HAPPY:
 			if(right_press) { state = HAPPY; }
			else if(left_press) { state = HUNGER; }
			else {state = HAPPY; }
			break;
		case CLEAN:
 			if(right_press) { state = CLEAN; }
			else if (left_press) { state = RESET; }
			else { state = CLEAN; }
			break;
		case HUNGER:
 			if(right_press) { state = HUNGER;}
			else if (left_press) { state = CLEAN; }
			else { state = HUNGER; }
			break;
		case RESET:
			if(right_press) { state = RESET; }
			else if (left_press) {state = HAPPY;}
			else {state = RESET;}
			break;
		default:
			state = HAPPY;
			break;
	}
	//actions 
	switch(state) {
		case HAPPY:
			LCD_ClearScreen();
			LCD_DisplayString(1, "Happy");
			LCD_Cursor(17);			
			for(int j = 0; j < stats[0]; j++) {
				LCD_heart();
			}
			if(button) {
				QueueEnqueue(choice, 0); 
			}
			lcd_wait = 0;
			break;
		case CLEAN:
			lcd_wait = 0;
			LCD_ClearScreen();
			LCD_DisplayString(1, "Clean");
			LCD_Cursor(17);
			for(int j = 0; j < stats[1]; j++) {
				LCD_heart();
			}
			if(button) {
				QueueEnqueue(choice, 1);
			}
			break;
		case HUNGER:
			lcd_wait = 0;
			LCD_ClearScreen();
			LCD_DisplayString(1, "Hunger");
			LCD_Cursor(17);
			for(int j = 0; j < stats[2]; j++) {
				LCD_heart();
			}
			if(button) {
				QueueEnqueue(choice, 2);
			}
			break;
		case  RESET:
			lcd_wait = 0;
			LCD_ClearScreen();
			LCD_DisplayString(1, "Reset?");
			if(button) {
				eeprom_update_byte ((uint8_t *) 1, 0);
				eeprom_update_byte ((uint8_t *) 4, 0);
				eeprom_update_byte ((uint8_t *) 8, 0);
				for(int j = 0; j < 3; j++) {
					stats[j] = 0;
				}
				LCD_ClearScreen();
				LCD_DisplayString(17, "Done.");
			}
			break;
		default:
			break;
	}
	return state;
};

enum joystick_state {NEUTRAL, R_MOVE, L_MOVE, D_MOVE, BUTTON};
int joystick_sm(int state) {
	//adc_init();
	//local variables
	uint16_t mid = 512;
	uint16_t vertArrow = adc_read(0);
	uint16_t horizArrow = adc_read(1);
	//transitions
	switch(state){
		case NEUTRAL:
			if (vertArrow < mid - 100) { //UP
				state = BUTTON;
				break;
			} else if (vertArrow > mid + 100){ //DOWN
				state = BUTTON;
				break;
			} else if (horizArrow < mid - 100) { //LEFT
				state = L_MOVE;
				break;
			} else if (horizArrow > mid + 100) { //RIGHT
				state = R_MOVE;
				break;
			} else {
				state = NEUTRAL;
				break;
			}
			state = NEUTRAL;
			break;
		case R_MOVE:		
			state = NEUTRAL;
			break;
		case L_MOVE:
			state = NEUTRAL;
			break;
		case  D_MOVE:
			state = NEUTRAL;
			break;
		case BUTTON:
			state = NEUTRAL;
			break;
		default:
			state = NEUTRAL;
			break;
	}
	//actions
	switch(state){
		case NEUTRAL:
			right_press = 0;
			left_press = 0;
			down_press = 0;
			button = 0;
			break;
		case R_MOVE:
			right_press = 1;
			break;
		case L_MOVE:
			left_press = 1;
			break;
		case D_MOVE:
			down_press = 1;
			break;
		case BUTTON:
			button = 1;
			break;
		default:
			break;
	}
	return state;
};

enum nokia_states {HAPPY0, HAPPY1, CLEAN0, CLEAN1, HUNGRY0, HUNGRY1, /*EAT, PET, BATH*/};
int nokia_sm(int state) {
	//local variables
	unsigned char help = 0;
	for(int m = 0; m < 3; ++m) {		//finds stat with lowest hearts
		if(stats[m] < stats[help]) {
			help = m;
		}
	}
	//transitions
	switch(state) {
		case HAPPY0:
			if(help == 0) {
				if(stats[0] < 3) { state = HAPPY0; break; }
				else{
					state = HAPPY1;
				}
				break;
			}
			if(help == 1) {
				if(stats[1] < 3) { state = CLEAN0; break; }
				state = CLEAN1;
				break;
			}
			if(help == 2) {
				if(stats[2] < 3) { state = HUNGRY0; break; }
				state = HUNGRY1;
				break;
			}
			state = HAPPY0;
			break;
		case HAPPY1:
			if(help == 0) {
				if(stats[0] < 3) { state = HAPPY0; break; }
				state = HAPPY1;
				break;
			}
			if(help == 1) {
				if(stats[1] < 3) { state = CLEAN0; break; }
				state = CLEAN1;
				break;
			}
			if(help == 2) {
				if(stats[2] < 3) { state = HUNGRY0; break; }
				state = HUNGRY1;
				break;
			}
			state = HAPPY1;
			break;
		case CLEAN0:
			if(help == 0) { 
				if(stats[0] < 3) { state = HAPPY0; break; }
				state = HAPPY1;
				break;
			}
			if(help == 1) {
				if(stats[1] < 3) { state = CLEAN0; break; }
				state = CLEAN1;
				break;
			}
			if(help == 2) {
				if(stats[2] < 3) { state = HUNGRY0; break; }
				state = HUNGRY1;
				break;
			}
			state = CLEAN0;
			break;
		case CLEAN1:
			if(help == 0) {
				if(stats[0] < 3) { state = HAPPY0; break; }
				state = HAPPY1;
				break;
			}
			if(help == 1) {
				if(stats[1] < 3) { state = CLEAN0; break; }
				state = CLEAN1;
				break;
			}
			if(help == 2) {
				if(stats[2] < 3) { state = HUNGRY0; break; }
				state = HUNGRY1;
				break;
			}
			state = CLEAN1;
			break;
		case HUNGRY0:
			if(help == 0) {
				if(stats[0] < 3) { state = HAPPY0; break; }
				state = HAPPY1;
				break;
			}
			if(help == 1) {
				if(stats[1] < 3) { state = CLEAN0; break; }
				state = CLEAN1;
				break;
			}
			if(help == 2) {
				if(stats[2] < 3) { state = HUNGRY0; break; }
				state = HUNGRY1;
				break;
			}
			state = HUNGRY0;
			break;
		case HUNGRY1:
			if(help == 0) {
				if(stats[0] < 3) { state = HAPPY0; break; }
				state = HAPPY1;
				break;
			}
			if(help == 1) {
				if(stats[1] < 3) { state = CLEAN0; break; }
				state = CLEAN1;
				break;
			}
			if(help == 2) {
				if(stats[2] < 3) { state = HUNGRY0; break; }
				state = HUNGRY1;
				break;
			}
			state = HUNGRY1;
			break;
		default:
			state = HAPPY0;
			break;
	}
	//actions
	nokia_lcd_clear();
	nokia_lcd_write_string("Your pet is...",1);
	nokia_lcd_set_cursor(0, 10);
	
	
	switch(state) {
		case HAPPY0:
			nokia_lcd_write_string("Sad :(", 2);
			break;
		case HAPPY1:
			nokia_lcd_write_string("Happy :)", 2);
			break;
		case CLEAN0:
			nokia_lcd_write_string("Dirty :(", 2);
			break;
		case CLEAN1:
			nokia_lcd_write_string("Clean :)", 2);
			break;
		case HUNGRY0:
			nokia_lcd_write_string("Hungry :(", 2);
			break;
		case HUNGRY1:
			nokia_lcd_write_string("Full :)", 2);
			break;
		default:
			nokia_lcd_write_string("error" , 3);
			break;
	}
	//if(button) {nokia_lcd_write_string("button press", 1); }
	//if(QueueIsEmpty(choice)) {nokia_lcd_write_string("empty Q", 1); }
	nokia_lcd_render();
	return state;
};

enum game_states {NO_ACTION, ADD_HAPPY, ADD_CLEAN, ADD_HUNGER};
int game_sm(int state) {
	//transitions
	switch(state) {
		case NO_ACTION:
			if(QueueIsEmpty(choice)) { state = NO_ACTION; break;}
			else{
				unsigned char option = QueueDequeue(choice);
				if(option == 0) { state = ADD_HAPPY; break; }
				else if(option == 1) { state = ADD_CLEAN; break;}
				else if (option == 2) { state = ADD_HUNGER; break;}
			}
			break;
		case ADD_HAPPY:
			if(QueueIsEmpty(choice)) { state = NO_ACTION; break;}
			state = ADD_HAPPY;
			break;
		case ADD_CLEAN:
			if(QueueIsEmpty(choice)) { state = NO_ACTION; break;}
			state = ADD_CLEAN;
			break;
		case ADD_HUNGER:
			if(QueueIsEmpty(choice)) { state = NO_ACTION; break;}
			state = ADD_HUNGER;
			break;
		default:
			state = NO_ACTION;
			break;
	}
	//actions
	switch(state) {
		case  NO_ACTION:
			QueueDequeue(choice);
			break;
		case ADD_HAPPY:
			if(stats[0] <= 10) {
				stats[0] = stats[0] +1;
			}
			QueueDequeue(choice);
			break;
		case ADD_CLEAN:
			if(stats[1] <= 10) {
				stats[1] = stats[1] +1;
			}
			QueueDequeue(choice);
			break;
		case ADD_HUNGER:
			if(stats[2] <= 10) {
				stats[2] = stats[2] +1;
			}
			QueueDequeue(choice);
			break;
		default:
			break;
	}
	eeprom_update_byte((uint8_t*) 1, (uint8_t) stats[0]);
	eeprom_update_byte((uint8_t*) 4, (uint8_t) stats[1]);
	eeprom_update_byte((uint8_t*) 8, (uint8_t) stats[2]);
	return state;
};

int main(void)
{
    DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	choice = QueueInit(3);
	stats[0] = eeprom_read_byte((uint8_t*)1);
	stats[1] = eeprom_read_byte((uint8_t*)4);
	stats[2] = eeprom_read_byte((uint8_t*)8);

	// Period for the tasks
	unsigned long int lcd_calc = 500;
	unsigned long int joystick_calc = 500;
	unsigned long int nokia_calc = 600;
	unsigned long int game_calc = 500;

	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(lcd_calc, joystick_calc);
	tmpGCD = findGCD(tmpGCD, nokia_calc);
	tmpGCD = findGCD(tmpGCD, game_calc);

	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;

	//Recalculate GCD periods for scheduler
	unsigned long int lcd_period = lcd_calc/GCD;
	unsigned long int joysstick_period = joystick_calc/GCD;
	unsigned long int nokia_period = nokia_calc/GCD;
	unsigned long int game_period = game_calc/GCD;
	//unsigned long int button_period = button_calc/GCD;

	//Declare an array of tasks
	static task lcd_task, joystick_task, nokia_task, game_task;
	task *tasks[] = { &lcd_task, &joystick_task, &nokia_task, &game_task};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	//******TASK NUMBER 1: LCD_TASK *******
	lcd_task.state = -1;//Task initial state.
	lcd_task.period = lcd_period;//Task Period.
	lcd_task.elapsedTime = lcd_period;//Task current elapsed time.
	lcd_task.TickFct = &lcd_sm;//Function pointer for the tick.

	//******TASK NUMBER 2: JOYSTICK_TASK *******
	joystick_task.state = -1;//Task initial state.
	joystick_task.period = joysstick_period;//Task Period.
	joystick_task.elapsedTime = joysstick_period;//Task current elapsed time.
	joystick_task.TickFct = &joystick_sm;//Function pointer for the tick.

	//******TASK NUMBER 3: NOKIA_TASK *******
	nokia_task.state = -1;//Task initial state.
	nokia_task.period = nokia_period;//Task Period.
	nokia_task.elapsedTime = joysstick_period;//Task current elapsed time.
	nokia_task.TickFct = &nokia_sm;//Function pointer for the tick.

	//******TASK NUMBER 4: GAME_TASK *******
	game_task.state = -1;//Task initial state.
	game_task.period = game_period;//Task Period.
	game_task.elapsedTime = game_period;//Task current elapsed time.
	game_task.TickFct = &game_sm;//Function pointer for the tick.
/*
	//******TASK NUMBER 5: BUTTON_TASK *******
	button_task.state = -1;//Task initial state.
	button_task.period = button_period;//Task Period.
	button_task.elapsedTime = button_period;//Task current elapsed time.
	button_task.TickFct = &button_sm;//Function pointer for the tick.

*/
	//timer, lcd, and adc init
	adc_init();
	nokia_lcd_init();
	LCD_init();
	TimerSet(GCD);
	TimerOn();

	unsigned short i;
    while (1) 
    {

		// Scheduler code
		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
    }
}

