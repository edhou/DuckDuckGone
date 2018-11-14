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
typedef struct {
	uint32_t x,y; // position
	uint16_t colours[3]; // 5-6-5 16-bit colours. Array size must be 3. Color 4 is transparency
	uint32_t width,height;
	unsigned char* pixels; // Each pixel has four possible colours, 1 byte per pixel
	uint8_t* pixels; // Each pixel has four possible colours, 1 byte per pixel
	int visible; // 0: hidden 1: shown
} sprite_t;

// Sprites on display
sprite_t* ducks;
sprite_t crosshair;
sprite_t deadDuck; // only one dead duck can be displayed at a time

// Misc. data
const int NDUCKS = 6;
uint32_t time;
uint32_t score;

// Define mutex
osMutexDef(newFrame); // waits for background to update sprite positions
osMutexId(newFrameID);

// Function to get joystick position
unsigned char* joystickRead(void) {
    static unsigned char* directions[] = {"left", "up", "right", "down", "none"};

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


// Define performance 
const uint32_t TICKSPEED = 200; // 200 ticks per second
const uint32_t FPS = 60;
const uint32_t FRAMERATE = SECOND/FPS;
const uint32_t delayLED = 5000;


void monitor(void const *arg) {
	
	GLCD_DisplayString(6, 1, 1, "Score: ");
	GLCD_DisplayString(1, 13, 1, "Time: ");
	
	GLCD_WindowMax();
	
	osMutexWait(newFrameID, osWaitForever);
	
	// Render frames
	while(1) {
		
		// Use mutex to pull array of sprites
		
		// Draw background
		
		// Draw sprites
		
		// Wait until the next frame
		osDelay(FRAMERATE);
	}
}

void background(void const* arg) {
	newFrameID = osMutexCreate(osMutex(newFrame));
	osMutexWait(newFrameID, osWaitForever);
	
	// Create crosshair
	crosshair.x = WIDTH/2;
	crosshair.y = HEIGHT/2;
	crosshair.colours[0] = Black;
	crosshair.colours[1] = Black;
	crosshair.colours[2] = Black;
	crosshair.width = 70;
	crosshair.height = 70;
	crosshair.visible = 1;
	crosshair.pixels = malloc(sizeof(uint8_t) * (crosshair.width*crosshair.height)/16 );
	
	// Create ducks
	int i =0;
	int duckW = 70;
	int duckH = 70;
	uint8_t* duckPixels = malloc(sizeof(uint8_t)*duckW*duckH/16);
	
	for(i=0; i<NDUCKS; i++) {
		ducks[i].x = 0;
		ducks[i].y = 0;
		ducks[i].colours[0] = Black;
		ducks[i].colours[1] = Maroon;
		ducks[i].colours[2] = White;
		ducks[i].width = duckW;
		ducks[i].height = duckH;
		ducks[i].visible = 0;
		ducks[i].pixels = duckPixels; // all ducks share the same image data
	}
	
	// Create dead duck
	deadDuck.x = 0;
	deadDuck.y = 0;
	deadDuck.colours[0] = Black;
	deadDuck.colours[1] = Maroon;
	deadDuck.colours[2] = White;
	deadDuck.width = 70;
	deadDuck.height = 70;
	deadDuck.visible = 0;
	deadDuck.pixels = malloc(sizeof(uint8_t) * (ducks[i].width*ducks[i].height) );
	
	osMutexRelease(newFrameID);
	
	while(1) {
		
	}
}

void aim(void const* arg) {
	
	float previousTime = 0;
	float time = timer_read()/1E6;
	float deltaTime = time - previousTime;
	previousTime = time;
	//MovePlayer(player_speed * delta_time, joystick_read()); 

	// Loop
	while(1) {
	   //GLCD_DisplayString(5, 3, 1, joystickRead()); //debug
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
	
	
	GLCD_DisplayString(5, 2, 1, "DuckDuckGone");
	GLCD_DisplayString(9, 1, 1, "INT0 to Begin");
	
	// if button not pressed, wait for it to be pressed
	while (LPC_GPIO2->FIOPIN & (1 << 10));
	// if button pressed, wait for it to be released
	while (!(LPC_GPIO2->FIOPIN & (1 << 10)));
	
	GLCD_Clear(0x226E);
	GLCD_SetBackColor(0x226E);
	GLCD_SetTextColor(0xE77D);
	
	// Thread definitions
	//osThreadDef(monitor, osPriorityNormal, 1, 0); 
	//osThreadDef(background, osPriorityNormal, 1, 0); 
	osThreadDef(aim, osPriorityNormal, 1, 0);
	osThreadDef(fire, osPriorityNormal, 1, 0);
	
	// Initialize multi-threading
	osKernelInitialize(); 
	osKernelStart();
	
	//osThreadCreate(osThread(monitor), NULL);
	//osThreadCreate(osThread(background), NULL);
	osThreadCreate(osThread(aim), NULL);
	osThreadCreate(osThread(fire), NULL);
	
	
	
}

