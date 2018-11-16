#include <lpc17xx.h>
#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include <cmsis_os.h>
#include "random.h"
#include "GLCD.h"
#include "crosshair.h"
#include "timer.h"

// Yann Roberge - yroberge
// Edward Hou - ehou

// Screen dimensions
int WIDTH = 240;
int HEIGHT = 320;
unsigned short BACKCOL = 0x041f;

// Sprite type
int duckW = 70;
int duckH = 70;

typedef struct {
	uint32_t x,y; // position
} duck_t;

// Ducks on display
const int NDUCKS = 6;
duck_t ducks[NDUCKS];

// Misc. data
uint32_t time;
uint32_t score;
enum Direction crossDirection = None;

// Define mutex
osMutexDef(newFrame); // waits for background to update sprite positions
osMutexId(newFrameID);

// Function to get joystick position
enum Direction joystickRead(void) {
	//Get joystick status
	int GPIOAStatus = LPC_GPIO1->FIOPIN;
	
	//Determine position
	if(!(GPIOAStatus & (1 << 23) ) )
		return Left;
	else if(!(GPIOAStatus & (1 << 24) ) )
		return Up;
	else if(!(GPIOAStatus & (1 << 25) ) )
		return Right;
	else if(!(GPIOAStatus & (1 << 26) ) )
		return Down;
	else
		return None;
}


// Define performance 
const uint32_t TICKSPEED = 200; // 200 ticks per second
const uint32_t SECOND = 1000; // 1000 ms in a second - needs 7000 to be close to a second
const uint32_t FPS = 60;
const uint32_t FRAMERATE = SECOND/FPS;
const uint32_t delayLED = 5000;


void monitor(void const *arg) {
	
	GLCD_DisplayString(300, 1, 1, "Score: ");
	GLCD_DisplayString(300, 10, 1, "Time: ");
	
	GLCD_WindowMax();
	
	//osMutexWait(newFrameID, osWaitForever);
	
	// Render frames
	unsigned int x1=60;
	unsigned int y1=60;
	unsigned int x2=120;
	unsigned int y2=60;
	unsigned int x3=150;
	unsigned int y3=60;
	unsigned int x4=60;
	unsigned int y4=60;
	
	while(1) {
		
		// Use mutex to pull array of sprites
		
		// Draw background
		
		// Draw sprites
		//GLCD_DisplayString(5, 3, 1, direction);
		//GLCD_Fill((100+x)%edge,100,60,60,BACKCOL);

		GLCD_Bitmap_Move1px(&x1,&y1,60,60,crosshair_map, crossDirection);
		//GLCD_Bitmap_Move1px(&x2,&y2,60,60,crosshair_map, Right);
		//GLCD_Bitmap_Move1px(&x3,&y3,60,60,crosshair_map, Up);
		//GLCD_Bitmap_Move1px(&x4,&y4,60,60,crosshair_map, Down);
		
		// Wait until the next frame
		osDelay(FRAMERATE);
	}
}

void background(void const* arg) {
	//newFrameID = osMutexCreate(osMutex(newFrame));
	//osMutexWait(newFrameID, osWaitForever);
	
	// Create crosshair
	
	// Create ducks	
	int i =0;
	for(i=0; i<NDUCKS; i++) {
		ducks[i].x = 0;
		ducks[i].y = 0;
	}
	
	// Create dead duck
	
	//osMutexRelease(newFrameID);
	
	while(1) {
		osDelay(1000);
	}
}

void aim(void const* arg) {
	
	float previousTime = 0;
	float time = timer_read()/1E6;
	float deltaTime = time - previousTime;
	previousTime = time;

	// Loop
	while(1) {
		
		time = timer_read()/1E6;
		deltaTime = time - previousTime;
		previousTime = time;
		
		crossDirection = joystickRead();
		
		//MovePlayer(player_speed * deltaTime, joystickRead()); 
		
		//direction = joystickRead();
		
	}
}

void fire(void const* arg) {
	
	// Configure pins as outputs
	LPC_GPIO1->FIODIR |= ((1 << 28) | (1 << 29) | (1 << 31)); // bits 28, 29, 31 set as outputs
	LPC_GPIO2->FIODIR |= 0x0000007C; // bits 2 to 6 set as outputs
	
	
	uint8_t fireEnable = 1; //1 = player can fire; 0 = player can't fire
	
	
	// Inital condition: All LEDs on
	
	LPC_GPIO1->FIOSET |= (1 << 28);
	LPC_GPIO1->FIOSET |= (1 << 29);
	LPC_GPIO1->FIOSET |= (1 << 31);

	LPC_GPIO2->FIOSET |= (1 << 2);
	LPC_GPIO2->FIOSET |= (1 << 3);
	LPC_GPIO2->FIOSET |= (1 << 4);
	LPC_GPIO2->FIOSET |= (1 << 5);
	LPC_GPIO2->FIOSET |= (1 << 6);	
	
	while(1){	
		
		if(fireEnable == 1){
			// wait for button press
			
			// if button not pressed, wait for it to be pressed
			while (LPC_GPIO2->FIOPIN & (1 << 10));
			// if button pressed, wait for it to be released
			while (!(LPC_GPIO2->FIOPIN & (1 << 10)));
				
			// do stuff on release
			
			//shoot();
			GLCD_DisplayString(4, 3, 1, "fire"); //debug
			
			fireEnable = 0;
			
			LPC_GPIO1->FIOCLR |= (1 << 28);
			LPC_GPIO1->FIOCLR |= (1 << 29);
			LPC_GPIO1->FIOCLR |= (1 << 31);

			LPC_GPIO2->FIOCLR |= (1 << 2);
			LPC_GPIO2->FIOCLR |= (1 << 3);
			LPC_GPIO2->FIOCLR |= (1 << 4);
			LPC_GPIO2->FIOCLR |= (1 << 5);
			LPC_GPIO2->FIOCLR |= (1 << 6);
			
			// clear all LEDs	
			LPC_GPIO1->FIOCLR |= (1 << 28);
			LPC_GPIO1->FIOCLR |= (1 << 29);
			LPC_GPIO1->FIOCLR |= (1 << 31);

			LPC_GPIO2->FIOCLR |= (1 << 2);
			LPC_GPIO2->FIOCLR |= (1 << 3);
			LPC_GPIO2->FIOCLR |= (1 << 4);
			LPC_GPIO2->FIOCLR |= (1 << 5);
			LPC_GPIO2->FIOCLR |= (1 << 6);
			
			osDelay(delayLED);
			GLCD_DisplayString(4, 3, 1, "    "); //debug
			LPC_GPIO1->FIOSET |= (1 << 28);	
			osDelay(delayLED);
			LPC_GPIO1->FIOSET |= (1 << 29);
			osDelay(delayLED);
			LPC_GPIO1->FIOSET |= (1 << 31);
			osDelay(delayLED);
			LPC_GPIO2->FIOSET |= (1 << 2);
			osDelay(delayLED);
			LPC_GPIO2->FIOSET |= (1 << 3);
			osDelay(delayLED);
			LPC_GPIO2->FIOSET |= (1 << 4);
			osDelay(delayLED);
			LPC_GPIO2->FIOSET |= (1 << 5);
			osDelay(delayLED);
			LPC_GPIO2->FIOSET |= (1 << 6);
		
			fireEnable = 1;
		}
	}
}


int main(void) {
	
	// Initalize LCD
	GLCD_Init();
	GLCD_Clear(0xFE40);
	GLCD_SetBackColor(0xFE40);
	GLCD_SetTextColor(0x4A07);
	
	// Initialize Timer
	timer_setup();
	
	
	GLCD_DisplayString(5, 2, 1, "DuckDuckGone");
	GLCD_DisplayString(9, 1, 1, "INT0 to Begin");
	
	// if button not pressed, wait for it to be pressed
	while (LPC_GPIO2->FIOPIN & (1 << 10));
	// if button pressed, wait for it to be released
	while (!(LPC_GPIO2->FIOPIN & (1 << 10)));
	
	GLCD_Clear(BACKCOL);
	GLCD_SetBackColor(BACKCOL);
	GLCD_SetTextColor(0xE77D);
	
	// Thread definitions
	osThreadDef(monitor, osPriorityNormal, 1, 0); 
	osThreadDef(background, osPriorityNormal, 1, 0); 
	osThreadDef(aim, osPriorityNormal, 1, 0);
	osThreadDef(fire, osPriorityNormal, 1, 0);
	
	// Initialize multi-threading
	osKernelInitialize(); 
	osKernelStart();
	
	osThreadCreate(osThread(monitor), NULL);
	osThreadCreate(osThread(background), NULL);
	osThreadCreate(osThread(aim), NULL);
	osThreadCreate(osThread(fire), NULL);
	
	
	
}

