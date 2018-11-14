#include <lpc17xx.h>
#include "stdio.h"
#include "uart.h"
#include "GLCD.h"
#include <cmsis_os.h>
#include "timer.h"

// Yann Roberge yroberge & Edward Hou ehou
// Wednesday Group 4
// MTE 241 Fall 2018

const char* joystickRead(void const *arg)
{
    static char* directions[] = {"up", "right", "down", "left", "none"};

    int GPIOAStatus = LPC_GPIO1->FIOPIN;
		if(!(GPIOAStatus & (1 << 23) ) )
			return directions[0];
		else if(!(GPIOAStatus & (1 << 24) ) )
			return directions[1];
		else if(!(GPIOAStatus & (1 << 25) ) )
			return directions[2];
		else if(!(GPIOAStatus & (1 << 26) ) )
			return directions[3];
		else
			return directions[4];
}


void joystick(void const *arg) {
	// Setup
	GLCD_Init();
	GLCD_Clear(0x226E);
	GLCD_SetBackColor(0x226E);
	GLCD_SetTextColor(0xE77D);

float previousTime = 0;
float time = timer_read()/1E6;
float deltaTime = time - previoustime;
previousTime = time;
//MovePlayer(player_speed * delta_time, joystick_read()); n

	// Loop
	while(1) {
	   GLCD_DisplayString(4, 3, 1, joystickRead());
	}
}



int main(void) {
	printf("Initializing...");

  timer_setup();

	osThreadDef(joystick, osPriorityNormal, 1, 0);

	osKernelInitialize();
	osKernelStart();

	osThreadCreate(osThread(joystick), NULL);

}
