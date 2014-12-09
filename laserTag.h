// Basic Includes
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#inclue <mraa/pwm.h>
#include <mraa.hpp>
#include <time.h>
// Controller Includes
#include "ds4_data.h"
#include "client.h"
#include "ds4.h"


/*
 * Initializes all of the GPIO outputs and the controller for the code
 */

void initGame(ds4_client_t* controller, mraa::Gpio hitLED, mraa_gpio_context* x, mraa::Pwm* pwm); 

/*
 * Sends a Manchester encoded one by having a 56kHz wave at a 50% duty cycle
 * as a 1 for 75% of the time and a 0% duty cycle as a 0 for 25% of the time
 *

mraa_result_t writeOne(mraa::Pwm* dev);

/*
 * Sends a Manchester encoded one by having a 56kHz wave at a 50% duty cycle
 * as a 1 for 25% of the time and a 0% duty cycle as a 0 for 75% of the time
 */

mraa_result_t writeZero(mraa::Pwm* dev);

/*
 * Sends a byte of data at a time
 *
 */

void writeByte (bool playera);

/*
 * Reads in the buffer generated from the interrupt
 */

void readBuffer(mraa::Gpio* hitLED, ds4_cleint_t controller);


/*
 * Interrupt that occurs when ever a falling edge occurs on the input
 */

void edge(void * args);

/*
 * Returns wave to 0% duty cycle following a byte transmission
 */

mraa_result_t finish(mraa::Pwm* dev);

