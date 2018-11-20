#include <lpc17xx.h>
#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include <cmsis_os.h>
#include "random.h"
#include "GLCD.h"
//#include "crosshair.h"
#include "crosshair_232.h"
#include "gamebackground.h"
#include "duck.h"
#include "timer.h"
#include <string.h>

// Yann Roberge - yroberge
// Edward Hou - ehou

// Screen dimensions
int WIDTH = 240;
int HEIGHT = 320;
unsigned short BACKCOL = 0x041f;
unsigned short EARTHCOL = 0x8b08;
unsigned int BACKGNDH = 100;

// Sprite type
int duckW = 50;
int duckH = 41;

typedef struct {
	uint32_t x,y; // position
	int visible; // 0: hidden ; 1: displayed
	int toClear; // flag indicating whether the duck image needs to be erased from the display
} duck_t;

// Ducks on display
const int NDUCKS = 30; // number of ducks the will spawn during game time
duck_t ducks[NDUCKS];
const int DUCKFREQ = 60/NDUCKS; // rounds a lot

// Crosshair position
unsigned char* crosshair_map;
unsigned int xCross=60;
unsigned int yCross=60;

// Misc. data
uint8_t inProgress = 0;
uint32_t time = 60;
uint8_t fireEnable = 1; //1 = player can fire; 0 = player can't fire
uint8_t shotFired = 0;
char timeDisp[6];
uint32_t score = 0; 
unsigned char scoreDisp[6];
enum Direction crossDirection = None;
uint32_t crossPixelsToMove = 0;

// Define mutexes
osMutexDef(newFrame); // waits for background to update sprite positions
osMutexId(newFrameID);

osMutexDef(crosshair); // protects crosshair parameters
osMutexId(crosshairID);

osMutexDef(duckParam); // protects duck array paramaters
osMutexId(duckParamID);

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

// Function to end game and show score
void showResult(void){
	osDelay(3000);
	GLCD_SetBackColor(0x6DEB);
	GLCD_SetTextColor(0x2A49);
	GLCD_Clear(0x6DEB);
	
	GLCD_DisplayString(3, 3, 1, "Game Over");	
	GLCD_DisplayString(6, 2, 1, "Your Score:");	
	sprintf(scoreDisp, "%d", score);
	GLCD_DisplayString(8, 7, 1, scoreDisp);	
	GLCD_DisplayString(10, 1, 1, "Press RESTART");	
	GLCD_DisplayString(11, 1, 1, "to Duck Again");	
}

// Function to restart game and show score
void restartGame(void){
	osDelay(3000);
	GLCD_Clear(BACKCOL);
	GLCD_SetBackColor(BACKCOL);
	GLCD_SetTextColor(0xE77D);
	
	osDelay(2500); //delay to allow screen to clear 
	
	inProgress = 1;
	fireEnable = 1;
	time = 15;
	
	GLCD_DisplayString(1, 1, 1, "Score: ");
	GLCD_DisplayString(2, 1, 1, "Time: ");
}

// Define performance 
const uint32_t TICKSPEED = 200; // 200 ticks per second
const uint32_t SECOND = 10000; // 1000 ms in a second - needs 10000 to be close to a second
const uint32_t FPS = 1000;
const uint32_t FRAMERATE = SECOND/FPS;
const uint32_t delayLED = SECOND/10;
const uint32_t crosshairSpeed = 5;

//// Number of ducks created per minute
//#define DUCKSPERMIN (15)
//const int DUCKGENSPEED = DUCKSPERMIN*SECOND/60; // ducks per second

unsigned int generateDuckStartHeight(void) {
	// Generates a pseudorandom starting height for a duck
	// Reference: https://www.tutorialspoint.com/c_standard_library/c_function_srand.htm
	srand( timer_read() );
	unsigned int max = HEIGHT-BACKGNDH-duckH;
	return rand()%max;
}

// Function for collision detection between crosshair and ducks for score count
// Unused: now embedded within Background task
void shoot(void){
	int i = 0;

	osMutexWait(duckParamID, osWaitForever);
	
	// Iterate through the ducks
	for (i=0; i<NDUCKS; i++) {
		if (ducks[i].visible) {
			// Clear ducks that have been shot
			osMutexWait(crosshairID, osWaitForever);
			if(((yCross+30) < (ducks[i].y + duckH) ) && (yCross+30 > ducks[i].y) && (xCross+30 > ducks[i].x) && (xCross+30 < (ducks[i].x + duckW))){
					printf("QUACK");
					ducks[i].visible = 0;
					ducks[i].toClear = 1;
					score++;
			}
			osMutexRelease(crosshairID);
		}
	}
	shotFired = 0; // reset flag
	
	osMutexRelease(duckParamID);
	
}
				
void monitor(void const *arg) {	
	
	newFrameID = osMutexCreate(osMutex(newFrame));
	osMutexWait(newFrameID, osWaitForever);
	
	// Convert the crosshair graphics bitmap from R2 G3 B2 format to R5 G6 B6 format used by GLCD functions
	// This is done to reduce the size of the executable below 32kB.
	crosshair_map = (unsigned char*)GLCD_Convert_232_565(60,60,crosshair_232_map);
	unsigned int i=0;
	
	free(crosshair_232_map);
	GLCD_WindowMax();
	
	// Display grass and dirt background
	unsigned int backGndH = HEIGHT-BACKGNDH;
	for(i=0; i<240; i+=10) {
		GLCD_Bitmap_Move(&i,&backGndH,10,BACKGNDH,gamebackground_map,0,Left);
	}
	
	const int scoreX = 255, scoreY = 15, scoreNumY = scoreY+95;
	const int timeX=255+26, timeY = 15, timeNumY = timeY+95;
	
	GLCD_SetBackColor(EARTHCOL);
	GLCD_DisplayStringPrecise(scoreX, scoreY, 1, "Score: ");
	GLCD_DisplayStringPrecise(timeX, timeY, 1, "Time: ");
	
	osMutexRelease(newFrameID);
	
	// Render frames
	unsigned int x1=60;
	unsigned int y1=60;
	
	while(1){
	while(inProgress == 1) {
		
		osMutexWait(newFrameID, osWaitForever);
		
		// Update score and time
		GLCD_SetBackColor(EARTHCOL);
		sprintf(scoreDisp, " %d", score);
		GLCD_DisplayStringPrecise(scoreX, scoreNumY, 1, scoreDisp);	
		
		sprintf(timeDisp, "%d", time);
		if(time < 10) {
			timeDisp[1] = timeDisp[0];
			timeDisp[0] = ' ';
		}
			
		//GLCD_DisplayStringPrecise(timeX, timeNumY, 1, " ");	
		GLCD_DisplayStringPrecise(timeX, timeNumY, 1, timeDisp);	
		GLCD_SetBackColor(BACKCOL);
		
		// Draw sprites
		
		// Draw ducks
		osMutexWait(duckParamID, osWaitForever);
		for (i=0; i<NDUCKS; i++) {
			if (ducks[i].visible) { // move visible ducks
				GLCD_Bitmap_Move(&(ducks[i].x),&(ducks[i].y),duckW,duckH,duck_map,2,Right);
			}
			else if (ducks[i].toClear) { // clear ducks that need to be cleared
				GLCD_Fill(ducks[i].x, ducks[i].y,duckW,duckH,BACKCOL);
				ducks[i].toClear = 0; // clear flag
			}
		}
		osMutexRelease(duckParamID);
		
		// Draw + move crosshair
		osMutexWait(crosshairID, osWaitForever);
		if(!( (yCross > 160) && (crossDirection == Down) )){
			GLCD_Bitmap_Move(&xCross,&yCross,60,60,crosshair_map,5,crossDirection);
		}
		
		osMutexRelease(crosshairID);
		osMutexRelease(newFrameID);
		
		// Wait until the next frame
		osDelay(FRAMERATE);
	}
}
	
}

void background(void const* arg) {
	
	duckParamID = osMutexCreate(osMutex(duckParam));
	osMutexWait(duckParamID, osWaitForever);
	
	timer_setup();
	
	// Create ducks	
	int i =0;
	int duckIndex = 0;
	for(i=0; i<NDUCKS; i++) {
		ducks[i].x = 0;
		ducks[i].y = generateDuckStartHeight();
		ducks[i].visible = 0;
	}
	
	int addDuckNow = 0;
	const int MAX_X_DUCK = WIDTH - duckW;
	
	osMutexRelease(duckParamID);
	
	while(1){
	while(inProgress == 1) {
		
		osMutexWait(duckParamID, osWaitForever);
		
		if(addDuckNow == 0) {
			ducks[duckIndex++].visible = 1;
		}
		addDuckNow = (++addDuckNow)%DUCKFREQ;
		
		// Iterate through the ducks
		for (i=0; i<NDUCKS; i++) {
			if (ducks[i].visible) {
				osMutexWait(crosshairID, osWaitForever);
				// Clear ducks that reach edge of screen
				if (ducks[i].x == MAX_X_DUCK){
					ducks[i].visible = 0;
					ducks[i].toClear = 1;
				}
			// Clear ducks that have been shot
				else if(shotFired == 1 && ((yCross+30) < (ducks[i].y + duckH) ) && (yCross+30 > ducks[i].y) && (xCross+30 > ducks[i].x) && (xCross+30 < (ducks[i].x + duckW))){
					printf("QUACK");
					ducks[i].visible = 0;
					ducks[i].toClear = 1;
					score++;
			}
			osMutexRelease(crosshairID);
			}
		}
		shotFired = 0; // reset flag
			
		osMutexRelease(duckParamID);
		
		// update game timer
		
		osDelay(SECOND);
		time--;
		if (time == 0){ // check for end game condition
			inProgress = 0;
			fireEnable = 0;
			showResult();
		}
	}
	
}
}

void aim(void const* arg) {
	
	// Create crosshair mutex
	crosshairID = osMutexCreate(osMutex(crosshair));
	
	// Initialize movement calculation variables
	float previousTime = 0;
	float time = timer_read()/1E6;
	float deltaTime = time - previousTime;
	previousTime = time;

	// Loop
	while(1){
	while(inProgress == 1) {
		
		// Movement calculations
		time = timer_read()/1E6;
		deltaTime = time - previousTime;
		previousTime = time;
				
		osMutexWait(crosshairID, osWaitForever);
			
			if(!( (yCross > 160) && (joystickRead()==Down) )){ // restrict crosshair to within the game screen
				crossDirection = joystickRead();
				crossPixelsToMove = crosshairSpeed * deltaTime;
			}
			else{
				crossDirection = None;
			}
			
		osMutexRelease(crosshairID);
		
	}
}
}

void fire(void const* arg) {
	
	// Configure pins as outputs
	LPC_GPIO1->FIODIR |= ((1 << 28) | (1 << 29) | (1 << 31)); // bits 28, 29, 31 set as outputs
	LPC_GPIO2->FIODIR |= 0x0000007C; // bits 2 to 6 set as outputs
	
	
	fireEnable = 1; //1 = player can fire; 0 = player can't fire
	
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
	if(inProgress == 1){	
		
		// wait for button press
		// if button not pressed, wait for it to be pressed
		while (LPC_GPIO2->FIOPIN & (1 << 10));
		// if button pressed, wait for it to be released
		while (!(LPC_GPIO2->FIOPIN & (1 << 10)));
		// do stuff on release
		
		if(fireEnable == 1){ // only fire if we are allowed to
									
			if(inProgress == 1){ //confirm game in progress
				
				shotFired = 1;
				
				shoot();
				//GLCD_DisplayString(4, 3, 1, "fire"); //debug
				printf("FIRED"); // Used to play sound
				
				fireEnable = 0;
				
				// clear all LEDs	
				LPC_GPIO1->FIOCLR |= (1 << 28);
				LPC_GPIO1->FIOCLR |= (1 << 29);
				LPC_GPIO1->FIOCLR |= (1 << 31);

				LPC_GPIO2->FIOCLR |= (1 << 2);
				LPC_GPIO2->FIOCLR |= (1 << 3);
				LPC_GPIO2->FIOCLR |= (1 << 4);
				LPC_GPIO2->FIOCLR |= (1 << 5);
				LPC_GPIO2->FIOCLR |= (1 << 6);
				
				// Re-enable LEDs one by one
				osDelay(delayLED);
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
	
	if(inProgress == 0){	
			// if button not pressed, wait for it to be pressed
			while (LPC_GPIO2->FIOPIN & (1 << 10));
			// if button pressed, wait for it to be released
			while (!(LPC_GPIO2->FIOPIN & (1 << 10)));
			
			//Restart game via RESET button
			//restartGame();
			//inProgress = 1;
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
	
	// Display start screen
	GLCD_DisplayString(3, 2, 1, "Duck");
	GLCD_DisplayString(4, 5, 1, "Duck");
	GLCD_DisplayString(5, 8, 1, "Gone");
	GLCD_DisplayString(9, 0, 1, "===============");
	GLCD_DisplayString(11, 1, 1, "INT0 to Begin");
	
	// if button not pressed, wait for it to be pressed
	while (LPC_GPIO2->FIOPIN & (1 << 10));
	// if button pressed, wait for it to be released
	while (!(LPC_GPIO2->FIOPIN & (1 << 10)));
	
	// Display game screen
	GLCD_Clear(BACKCOL);
	GLCD_SetBackColor(BACKCOL);
	GLCD_SetTextColor(0xE77D);
	
	osDelay(2500); //delay to allow screen to clear 
	inProgress = 1;
	
	// Thread definitions
	osThreadDef(monitor, osPriorityNormal, 1, 0); 
	osThreadDef(background, osPriorityNormal, 1, 0); 
	osThreadDef(aim, osPriorityNormal, 1, 0);
	osThreadDef(fire, osPriorityNormal, 1, 0);
	
	// Initialize multi-threading
	osKernelInitialize(); 
	osKernelStart();
	
	// Start Threads	
	osThreadCreate(osThread(monitor), NULL);
	osThreadCreate(osThread(background), NULL);
	osThreadCreate(osThread(aim), NULL);
	osThreadCreate(osThread(fire), NULL);
	
	
}

