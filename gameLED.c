#include <lpc17xx.h>
#include "stdio.h"
#include "uart.h"
#include "GLCD.h"
#include <cmsis_os.h>
#include "timer.h"

// Yann Roberge yroberge & Edward Hou ehou
// Wednesday Group 4
// MTE 241 Fall 2018

uint32_t delayLED = 5000;

void fireTest(void const *arg) {
	
	// Setup
	GLCD_Init();
	GLCD_Clear(0x226E);
	GLCD_SetBackColor(0x226E);
	GLCD_SetTextColor(0xE77D);
	
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
			GLCD_DisplayString(4, 3, 1, "    ");
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
	printf("Initializing...");
	
	osThreadDef(fireTest, osPriorityNormal, 1, 0); 
	
	osKernelInitialize(); 
	osKernelStart();
	
	osThreadCreate(osThread(fireTest), NULL); 
	
}
