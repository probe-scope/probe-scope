/*
 * Main Probe-Scope Application
 * 
 * @Company
 *   Probe-Scope Team
 * 
 * @File Name
 *   app.h
 * 
 * @Summary
 *   FPGA Interface and Sample Memory
*/

#ifndef _APP_H
#define _APP_H


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "definitions.h"
#include "comms.h"
#include "interface.h"


#define SAMPLES 8192


#ifdef	__cplusplus
extern "C"
{
#endif


typedef enum
{
	// Application's state machine's initial state.
	APP_STATE_INIT=0,
	
	// Application waits for device configuration
	APP_STATE_WAIT_FOR_CONFIGURATION,
	
	// Wait for trigger from FPGA
	APP_STATE_WAIT_TRIGGER,
	
	// Wait for FPGA ready signal
	APP_STATE_WAIT_BUFFER,
	
	// Request ADC sample from FPGA
	APP_STATE_GET_SAMPLE,
	
	// Read ADC sample from FPGA
	APP_STATE_WAIT_SAMPLE,
	
	// Trigger gotten, wait for IF module to reset it
	APP_STATE_TRIGGERED,
	
	// Application Error state
	APP_STATE_ERROR
} app_states_t;


typedef struct
{
	uint8_t * first;
	uint8_t * last;
	uint8_t * end;
	uint8_t data[SAMPLES];
} rudimentary_buffer_t;


typedef struct
{
	app_states_t state;
	
	cdc_comms_t comms;
	
	bool blink_tick;
	bool comms_tick;
	
	int8_t last_sample;
	
	SYS_TIME_HANDLE usb_timeout;
	
	rudimentary_buffer_t buf;
	
	bool stop_acq;
	bool triggered;
} app_data_t;


/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the 
    application in its initial state and prepares it to run so that its 
    APP_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/
void APP_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_Tasks ( void )

  Summary:
    MPLAB Harmony application tasks function

  Description:
    This routine is the Harmony application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_Tasks ( void );


extern cdc_comms_t * gp_comms;
extern app_data_t appData;



#ifdef	__cplusplus
}
#endif

#endif /* _APP_H */
