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
void readBuffer();
void edge(void * args); 
                                             
int main () { 
    // Right Trigger Data
	bool rightTrigger;
    // Initialize Controller
	ds4_client_t* controller = ds4_client_new();
 	const ds4_controls_t* controllerData;       
    // Initializing Interrupts                                       
    mraa_init();
    mraa_gpio_context x;
    x = mraa_gpio_init(8);
    if (x == NULL) {
	fprintf(stdout, "ERROR WITH GPIO\n");
	return MRAA_ERROR_UNSPECIFIED;
    }
    mraa_gpio_dir(x,MRAA_GPIO_IN);
    gpio_edge_t rising = MRAA_GPIO_EDGE_RISING;

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
    pwm->write(0.5);
    fall = true;
    //ds4_client_rgb(controller,255,0,0);
    while (running == 0) {
	if (bufferLoaded) { 
		readBuffer();
	}
	else {
        	// Ping every 0.1 seconds
		usleep(10000);
		// Check if right Trigger is pressed
		//controllerData = ds4_client_controls(controller);
		//rightTrigger = controllerData->r2;	
	
		writeByte(0,1,1,pwm);
	}
    }                                                
    delete pwm;                                      
} 

void readBuffer() {
	bufferLoaded = false;                
        // Read the buffer                   
	if (buf[3] != buf[6])
		buf[3] = 0; 
	fprintf(stdout, "Player %d ", buf[1]*2+buf[2]);
        if (buf[3] == 1)                                        
   	     fprintf(stdout, "shot you\n");                          
        else                                           
             fprintf(stdout, "pinged you\n");                        
                                                               
        // If its a ping, send wireless signal to vibrate opponents cont
                                                                        
        // If its a hit record the score                                
}
                                                   
void writeByte(bool playera, bool playerb, bool shoot, mraa::Pwm* pwm) {
	writeZero(pwm);
	for (int i = 0; i < 2; i++) {
		if (playera)
			writeOne(pwm);
		else
			writeZero(pwm);
		if (playerb)
			writeOne(pwm);
		else
			writeZero(pwm);
		if (shoot)
			writeOne(pwm);
		else
			writeZero(pwm);
	}
	writeOne(pwm);	
}	


mraa_result_t writeOne(mraa::Pwm* dev) {             
	clock_t t;                                   
        t = clock();                                 
        dev->write(0.0);                             
        while (clock() - t < 500);                   
        dev->write(0.5);                             
        while (clock() - t < 2000);                  
        return MRAA_SUCCESS;                         
}    

mraa_result_t writeZero(mraa::Pwm* dev) {             
        clock_t t;                                   
        t = clock();                                 
        dev->write(0.0);                             
        while (clock() - t < 1500);                   
        dev->write(0.5);                             
        while (clock() - t < 2000);                  
        return MRAA_SUCCESS;                         
}

void edge (void * args) {
	usleep(1000);
	mraa_gpio_context* dev = (mraa_gpio_context*) args;
	if (mraa_gpio_read(*dev) == 0){
	    fprintf(stdout, "Wrote a Zero to buffer spot %d\n", bufSpot);
	    buf[bufSpot] = 0;
	}
	else{
	    fprintf(stdout, "Wrote a One to buffer spot %d\n", bufSpot);
	    buf[bufSpot] = 1;
	}
	if (bufSpot >= 15)
	    bufSpot = 0;
	else
	    bufSpot++;
	if (bufSpot %8 == 0)
	    bufferLoaded = true;	    
}
