#include <lpc17xx.h>
#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include <cmsis_os.h>
#include "random.h"
#include "GLCD.h"
#include "crosshair.h"

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

// Define performance 
const uint32_t TICKSPEED = 200; // 200 ticks per second
const uint32_t SECOND = 1000; // 1000 ms in a second - needs 7000 to be close to a second
const uint32_t FPS = 60;
const uint32_t FRAMERATE = SECOND/FPS;

void monitor(void const *arg) {
	
	// Initalize LCD
	GLCD_Init();
	GLCD_Clear(BACKCOL);
	GLCD_SetBackColor(BACKCOL);
	GLCD_SetTextColor(0xE77D);
	
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
		//GLCD_Fill((100+x)%edge,100,60,60,BACKCOL);
		GLCD_Bitmap_Move1px(&x1,&y1,60,60,crosshair_map, Down);
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
	crosshair.x = WIDTH/2;
	crosshair.y = HEIGHT/2;
	crosshair.colours[0] = Black;
	crosshair.colours[1] = Black;
	crosshair.colours[2] = Black;
	crosshair.width = 60;
	crosshair.height = 60;
	crosshair.visible = 1;
	crosshair.pixels = crosshair_map;
	
	// Create ducks
	int i =0;
	int duckW = 70;
	int duckH = 70;
	unsigned char* duckPixels = malloc(sizeof(char)*duckW*duckH/16);
	
	
	for(i=0; i<NDUCKS; i++) {
		ducks[i].x = 0;
		ducks[i].y = 0;
		ducks[i].colours[0] = Black;
		ducks[i].colours[1] = Maroon;
		ducks[i].colours[2] = White;
		ducks[i].width = duckW;
		ducks[i].height = duckH;
		ducks[i].visible = 0;
		ducks[i].pixels = duckPixels; // all ducks share the same image data. This saves memory.
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
	deadDuck.pixels = malloc(sizeof(char) * (ducks[i].width*ducks[i].height) );
	
	//osMutexRelease(newFrameID);
	
	while(1) {
		osDelay(1000);
	}
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
	
	// Thread definitions
	osThreadDef(monitor, osPriorityNormal, 1, 0); 
	//osThreadDef(background, osPriorityNormal, 1, 0); 
	//osThreadDef(aim, osPriorityNormal, 1, 0);
	//osThreadDef(fire, osPriorityNormal, 1, 0);
	
	// Initialize multi-threading
	osKernelInitialize(); 
	osKernelStart();
	osThreadCreate(osThread(monitor), NULL);
	//osThreadCreate(osThread(background), NULL);
	//osThreadCreate(osThread(aim), NULL);
	//osThreadCreate(osThread(fire), NULL);
}
