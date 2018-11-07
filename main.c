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

// Define counters

uint32_t q1sent, q1recv, q1over, q2sent, q2recv, q2over, monitorTime;

// Define performance 

uint32_t TICKSPEED = 200; // 200 ticks per second
uint32_t SECOND = 10000; // 1000 ms in a second - needs 10000 to be close to a second
int RATE_ARRIVAL = 9; //Hz, client
int RATE_SERVICE = 10; //Hz, server

void monitor(void const *arg) {
	
	// Initalize LCD
	GLCD_Init();
	GLCD_Clear(0x226E);
	GLCD_SetBackColor(0x226E);
	GLCD_SetTextColor(0xE77D);
	
	GLCD_DisplayString(1, 7, 1, "q1");	
	GLCD_DisplayString(1, 13, 1, "q2");	
	GLCD_DisplayString(2, 1, 1, "sent");	
	GLCD_DisplayString(3, 1, 1, "recv");	
	GLCD_DisplayString(4, 1, 1, "over");	
	GLCD_DisplayString(6, 1, 1, "time");
	
	GLCD_WindowMax();
	
	int i=0;
	int j=0;
	for (i=0; i<20;i++) {
		for (j=0; j<20;j++)
			GLCD_PutColouredPixel(i,j,Magenta);
	}
	
	GLCD_PutPixel(0,0);
	GLCD_PutPixel(1,1);
	GLCD_PutPixel(2,2);
	GLCD_PutPixel(3,3);
	GLCD_PutPixel(4,4);
	GLCD_PutPixel(5,5);
	
	// Comments comments comments
	// Refresh LCD screen
	while(1) {
		
		//convert each counter to a string, then display the string
//		unsigned char q1sentStr[6];
//		sprintf(q1sentStr, "%d", q1sent);
//		GLCD_DisplayString(2, 7, 1, q1sentStr);
//		
//		unsigned char q2sentStr[6];
//		sprintf(q2sentStr, "%d", q2sent);
//		GLCD_DisplayString(2, 13, 1, q2sentStr);		
//		
//		unsigned char q1recvStr[6];
//		sprintf(q1recvStr, "%d", q1recv);
//		GLCD_DisplayString(3, 7, 1, q1recvStr);		
//		
//		unsigned char q2recvStr[6];
//		sprintf(q2recvStr, "%d", q2recv);
//		GLCD_DisplayString(3, 13, 1, q2recvStr);		
//		
//		unsigned char q1overStr[6];
//		sprintf(q1overStr, "%d", q1over);
//		GLCD_DisplayString(4, 7, 1, q1overStr);		
//		
//		unsigned char q2overStr[6];
//		sprintf(q2overStr, "%d", q2over);
//		GLCD_DisplayString(4, 13, 1, q2overStr);		
//		
//		unsigned char monitorTimerStr[6];
//		sprintf(monitorTimerStr, "%d", monitorTime);
//		GLCD_DisplayString(6, 7, 1, monitorTimerStr);		
//		
		// increment the timer
		monitorTime++;
		
		// wait for one second
		osDelay(SECOND);
	}
}

void server(void const *arg) {
	
	// Read which mailbox the thread is for
	osMailQId* servMBox = (osMailQId*)arg;
	
	// Set the appropriate counters 
	uint32_t* qrecv;
	if (servMBox == &q1_id)
		qrecv = &q1recv;
	else
		qrecv = &q2recv;
	
	uint32_t serverDelay;
	
	while(1) {
			
		// Calculate the next delay time
		serverDelay = next_event()*SECOND/RATE_SERVICE;
		osDelay( (serverDelay) >> 16);
		
		// Grab the message from the mailbox
		osEvent evt = osMailGet(*servMBox, osWaitForever); // receive message
		if(evt.status == osEventMail) {
			osMailFree(*servMBox, evt.value.p); // free mailbox
			(*qrecv)++; // Update received counts
	}
		
	}
}

void client(void const *arg) {
	
	int *mptr; // Define pointer
	osMailQId* currMBox = &q1_id; // points to mailbox currently in use
	
	// Define counters to be used
	uint32_t* qsent = &q1sent;
	uint32_t* qover = &q1over;
	
	uint32_t clientDelay;
	
	while(1) {
		
		// Calculate the next delay time
		clientDelay = next_event()*SECOND/(RATE_ARRIVAL*2); //divide 2 to get 9 Hz per queue
		osDelay( (clientDelay) >> 16);
		
		// Create and send the message to the mailbox
		mptr = osMailAlloc(*currMBox, 0);
		
		// If message creation succeeded, increment sent counter; else increment overflow counter
		if(!(mptr == NULL)){
			osMailPut(*currMBox, mptr);
			(*qsent)++;
		}
		else
			(*qover)++;
		
		
		// Switch to the other mailbox
		if (currMBox == &q1_id) {
			currMBox = &q2_id;
			qsent = &q2sent;
			qover = &q2over;
		}
		else {
			currMBox = &q1_id;
			qsent = &q1sent;
			qover = &q1over;
		}
		
	}
}

int main(void) {
	// Mailbox initialization
	q1_id = osMailCreate(osMailQ(q1), NULL);
	q2_id = osMailCreate(osMailQ(q2), NULL);
	
	// Counter initalization
	q1sent = 0;
	q1recv = 0;
	q1over = 0;
	q2sent = 0;
	q2recv = 0;
	q2over = 0;
	monitorTime = 0;
	
	// Thread definitions
	osThreadDef(monitor, osPriorityNormal, 1, 0); 
	osThreadDef(server, osPriorityNormal, 2, 0); 
	osThreadDef(client, osPriorityNormal, 1, 0);
	
	// Initialize multi-threading
	osKernelInitialize(); 
	osKernelStart();
	osThreadCreate(osThread(monitor), NULL);
	osThreadCreate(osThread(server), &q1_id);
	osThreadCreate(osThread(server), &q2_id);
	osThreadCreate(osThread(client), NULL);
}
