#include <lpc17xx.h>
#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include <cmsis_os.h>
#include "random.h"
#include "GLCD.h"

// Yann Roberge - yroberge
// Edward Hou - ehou

// Define mailboxes
osMailQId q1_id;
osMailQId q2_id;

osMailQDef(q1, 10, int);
osMailQDef(q2, 10, int);

// Define variables passed between tasks

// Define mutex

// Define performance 
const uint32_t TICKSPEED = 200; // 200 ticks per second
const uint32_t SECOND = 10000; // 1000 ms in a second - needs 10000 to be close to a second
const uint32_t FPS = 30;
const uint32_t FRAMERATE = FPS/SECOND;

void monitor(void const *arg) {
	
	// Initalize LCD
	GLCD_Init();
	GLCD_Clear(0x226E);
	GLCD_SetBackColor(0x226E);
	GLCD_SetTextColor(0xE77D);
		
	GLCD_DisplayString(6, 1, 1, "Score: ");
	GLCD_DisplayString(1, 13, 1, "Time: ");
	
	GLCD_WindowMax();
	
//	int i=0;
//	int j=0;
//	for (i=0; i<20;i++) {
//		for (j=0; j<20;j++)
//			GLCD_PutColouredPixel(i,j,Magenta);
//	}
	
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
	
}

void aim(void const* arg) {
	
	while(1) {
		
	}
}

void fire(void const* arg) {
	
	while(1) {
	
	}
}

int main(void) {
	// Mailbox initialization

	// Sprite type
	typedef struct {
		uint32_t x,y; // position
		uint16_t* bitmap; // array of coloured pixels (image)
	} sprite_t;
	
	// Thread definitions
	osThreadDef(monitor, osPriorityNormal, 1, 0); 
	osThreadDef(background, osPriorityNormal, 1, 0); 
	osThreadDef(aim, osPriorityHigh, 1, 0);
	osThreadDef(fire, osPriorityHigh, 1, 0);
	
	// Initialize multi-threading
	osKernelInitialize(); 
	osKernelStart();
	osThreadCreate(osThread(monitor), NULL);
	osThreadCreate(osThread(background), NULL);
	osThreadCreate(osThread(aim), NULL);
	osThreadCreate(osThread(fire), NULL);
}
