#include <stdlib.h>                                  
#include <stdio.h>                                   
#include <unistd.h>                                  
#include <mraa/pwm.h>                                
#include <mraa.hpp>                                  
#include <time.h>                                    
                
// For sending data (will change later)                                     
int running = 0;
// For Interrupts
clock_t manEncode;
volatile int bufSpot = 0;                                    
volatile bool fall = true;
//Buffer
volatile int buf[16];
                 
// Functions                                     
mraa_result_t writeOne(mraa::Pwm* dev);              
mraa_result_t writeZero(mraa::Pwm* dev);             
void fallEdge(void * args); 
void riseEdge(void * args);       
                                             
int main () { 
    // Initializing Interrupts                                       
    mraa_init();
    mraa_gpio_context x;
    x = mraa_gpio_init(8);
    if (x == NULL) {
	fprintf(stdout, "ERROR WITH GPIO\n");
	return MRAA_ERROR_UNSPECIFIED;
    }
    mraa_gpio_dir(x,MRAA_GPIO_IN);
    gpio_edge_t anyedge = MRAA_GPIO_EDGE_BOTH;

    mraa_gpio_isr(x, anyedge, &fallEdge, NULL);
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
    while (running == 0) {                           
           writeOne(pwm);                            
        //   writeZero(pwm);                         
    }                                                
    delete pwm;                                      
}                                                    
                                                     
mraa_result_t writeOne(mraa::Pwm* dev) {             
        clock_t t;                                   
        t = clock();                                 
        dev->write(0.0);                             
        while (clock() - t < 250);                   
        dev->write(0.5);                             
        while (clock() - t < 1000);                  
        return MRAA_SUCCESS;                         
}    

mraa_result_t writeZero(mraa::Pwm* dev) {             
        clock_t t;                                   
        t = clock();                                 
        dev->write(0.0);                             
        while (clock() - t < 750);                   
        dev->write(0.5);                             
        while (clock() - t < 1000);                  
        return MRAA_SUCCESS;                         
}

void fallEdge (void * args) {
	if (fall) {
		manEncode = clock();
		if (bufSpot > 15)
		   bufSpot = 0;
		else
 	   	   bufSpot++;
	   	fprintf(stdout, "BufSpot: %d manEncode: %d \n", bufSpot, manEncode);
		fall = false;
	}
       	else {
		if (clock() - manEncode > 500)
	  		buf[bufSpot] = 1;
		else
		        buf[bufSpot] = 0;

		fprintf(stdout, "Wrote %d to bufSpot %d: Delay was %d \n", buf[bufSpot], bufSpot, clock()- manEncode);
		fall = true;
	}
	if (bufSpot%8 == 0)
		fprintf(stdout, "\n\n\n");

}  
