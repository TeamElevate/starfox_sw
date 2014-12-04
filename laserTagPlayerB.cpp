// PLAYER 0 CODE

#include <stdlib.h>                                  
#include <stdio.h>                                   
#include <unistd.h>                                  
#include <mraa/pwm.h>                                
#include <mraa.hpp>                                  
#include <time.h>                                    
// Controller Includes
#include "ds4_data.h"
#include "client.h"
#include "ds4.h"     

        
// 0 if game is running                                     
int running = 0;

// Block Interrupt
bool blockInterrupt = false;

// Buffer ready to read
volatile bool bufferLoaded = false;

// For Interrupts
clock_t manEncode;
volatile int bufSpot = 0;                                    
volatile bool fall = true;

//Buffer
volatile int buf[16];
                 
// Functions                                     
mraa_result_t writeOne(mraa::Pwm* dev);              
mraa_result_t writeZero(mraa::Pwm* dev);             
void writeByte (bool playera, bool playerb, bool shoot, mraa::Pwm* pwm);
void readBuffer(mraa::Gpio* hitLED, ds4_client_t* controller);
void edge(void * args); 
mraa_result_t finish(mraa::Pwm* dev); 
                                            
int main () { 
    // Right Trigger Data
	bool rightTrigger;
    // Initialize Controller
	ds4_client_t* controller = ds4_client_new();
 	const ds4_controls_t* controllerData;       
    // HIT LED SETUP
   mraa::Gpio* hitLED = new mraa::Gpio(47); 
   if (hitLED == NULL) {
	fprintf(stdout, "ERROR WITH HITLED\n");
	return MRAA_ERROR_UNSPECIFIED;
    }	
    hitLED->dir(mraa::DIR_OUT);
    // INITIALIZING INTERRUPTS
    mraa_gpio_context x;
    x = mraa_gpio_init(46);
    if (x == NULL) {
	fprintf(stdout, "ERROR WITH GPIO\n");
	return MRAA_ERROR_UNSPECIFIED;
    }
    mraa_gpio_dir(x,MRAA_GPIO_IN);
    gpio_edge_t rising = MRAA_GPIO_EDGE_FALLING;

    mraa_gpio_isr(x, rising, &edge, &x);
   // Initializing PWM's
    mraa::Pwm* pwm;                                  
    pwm = new mraa::Pwm(20);                         
    fprintf(stdout, "Starting ...\n");               
    if (pwm == NULL) {                               
        return MRAA_ERROR_UNSPECIFIED;                
    }                                                
    while (pwm->period_us(18) != MRAA_SUCCESS);      
    fprintf(stdout, "Cycling PWM on IO20 (pwm3) \n");
    pwm->enable(true); 
    pwm->write(0.0);
    fall = true;
    //ds4_client_rgb(controller,255,0,0);
    while (running == 0){
	if (bufferLoaded) { 
		readBuffer(hitLED, controller);
	}
	else {
        	// Ping every 0.1 seconds
		usleep(1000);
		// Check if right Trigger is pressed
		if (ds4_client_connected(controller)) {
			controllerData = ds4_client_controls(controller);
			rightTrigger = controllerData->r1;
		}	
		hitLED->write(0);
		if (rightTrigger){
			fprintf(stdout, "SHOOTING\n");
			writeByte(0,1,1,pwm);
		}
	}
    } 
 //   delete pwm;                                      
} 

void readBuffer(mraa::Gpio* hitLED,ds4_client_t* controller) {
	if (ds4_client_connected(controller))
		ds4_client_rumble(controller);
	bufferLoaded = false;
	hitLED->write(1);
	blockInterrupt=true;	
	fprintf(stdout, "HITTTT\n");
	usleep(1000000);
	fprintf(stdout, "RESET\n");
	bufSpot = 0;
	hitLED->write(0);
	usleep(1000000);
	blockInterrupt = false;
	
}
                                                   
void writeByte(bool playera, bool playerb, bool shoot, mraa::Pwm* pwm) {
	for (int i = 0; i < 15; i++) {
		writeOne(pwm);
	}
}	

mraa_result_t finish(mraa::Pwm* dev) {
	dev->write(0.0);
	return MRAA_SUCCESS;	
}

mraa_result_t writeOne(mraa::Pwm* dev) {             
	clock_t t;                                   
        t = clock();                                 
        dev->write(0.5);                             
        while (clock() - t < 1500);                   
        dev->write(0.0);                             
        while (clock() - t < 2000);                  
        return MRAA_SUCCESS;                         
}    

mraa_result_t writeZero(mraa::Pwm* dev) {             
        clock_t t;                                   
        t = clock();                                 
        dev->write(0.5);                             
        while (clock() - t < 500);                   
        dev->write(0.0);                             
        while (clock() - t < 2000);                  
        return MRAA_SUCCESS;                         
}

void edge (void * args) {
   if(!blockInterrupt){
	usleep(700);
	mraa_gpio_context* dev = (mraa_gpio_context*) args;
	if (mraa_gpio_read(*dev) == 0){
	    fprintf(stdout, "Wrote a One to buffer spot %d\n", bufSpot);
	    buf[bufSpot] = 1;
	    if (bufSpot >= 15)
		bufSpot = 0;
	    else
		bufSpot++;
	}
	else{
	    fprintf(stdout, "Self Shot", bufSpot);
	}
	if (bufSpot%8 == 0)
		bufferLoaded = true;
   }
}






