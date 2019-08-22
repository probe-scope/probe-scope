/*
 * Main Probe-Scope Application
 * 
 * @Company
 *   Probe-Scope Team
 * 
 * @File Name
 *   app.c
 * 
 * @Summary
 *   FPGA Interface and Sample Memory
*/

#include "app.h"
#include <stdio.h>
#include <string.h>


uint8_t CACHE_ALIGN text_buffer[20];

app_data_t appData;
cdc_comms_t * gp_comms;


/// Callbacks

static void
APP_ISR_Blink (uintptr_t context)
{
	app_data_t * appData = (app_data_t *) context;
	
	if (appData->blink_tick)
	{
		LED0_Clear();
		appData->blink_tick = false;
	}
	else	
	{
		LED0_Set();
		appData->blink_tick = true;
	}
}

static void
APP_ISR_Tick (uintptr_t context)
{
	app_data_t * appData = (app_data_t *) context;
	
	appData->comms_tick = true;
}


/// Main Functions

void APP_Initialize(void)
{
    // Place the App state machine in its initial state.
    appData.state = APP_STATE_INIT;
	
	// Set up USB comms
	gp_comms = &(appData.comms);
	comms_init(&(appData.comms), USB_DEVICE_INDEX_0);
	if_init();
	
	appData.blink_tick = false;
	appData.comms_tick = false;
	appData.stop_acq = false;
	
	appData.buf.first = appData.buf.data;
	appData.buf.last = appData.buf.data;
	appData.buf.end = appData.buf.data + SAMPLES;
	
	FPP_D0_InputEnable();
	FPP_D1_InputEnable();
	FPP_D2_InputEnable();
	FPP_D3_InputEnable();
	FPP_D4_InputEnable();
	FPP_D5_InputEnable();
	FPP_D6_InputEnable();
	FPP_D7_InputEnable();
	FPP_DRDY_InputEnable();
	FPP_DREQ_OutputEnable();
	FPP_DREQ_Clear();
	FPIO0_InputEnable(); // trigger
	FPIO1_OutputEnable(); // pic ready
	FPIO2_InputEnable(); // fpga ready
}

void APP_Tasks(void)
{
	if_task();
    switch(appData.state)
    {
        case APP_STATE_INIT:
			
			SYS_TIME_CallbackRegisterMS(APP_ISR_Blink, (uintptr_t) &appData, 500, SYS_TIME_PERIODIC);
			SYS_TIME_CallbackRegisterMS(APP_ISR_Tick, (uintptr_t) &appData, 1, SYS_TIME_PERIODIC);
			
			appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;
			appData.triggered = false;
			
            break;
		
        case APP_STATE_WAIT_FOR_CONFIGURATION:
			
            // Check if the device was configured
            if(appData.comms.isConfigured)
            {
                appData.state = APP_STATE_WAIT_TRIGGER;
            }
            
            break;
		
		case APP_STATE_WAIT_TRIGGER:
			LED1_Clear();
			if (FPIO0_Get()) // FPGA trigger out
			{
				LED1_Set();
				FPIO1_Set(); // indicate ready to receive
				appData.buf.first = appData.buf.data;
				appData.buf.last = appData.buf.data;
				appData.state = APP_STATE_WAIT_BUFFER;
			}
			break;
		
		case APP_STATE_WAIT_BUFFER:
			if (FPIO2_Get()) // FPGA second half full
			{
				appData.state = APP_STATE_GET_SAMPLE;
			}
			break;
		
		case APP_STATE_GET_SAMPLE:
			if (!appData.stop_acq)
			{
				if (FPIO2_Get())
				{
					FPP_DREQ_Set();
					LED2_Toggle();
			
					appData.state = APP_STATE_WAIT_SAMPLE;
					// no break to enable immediate check for response
				}
				else
				{
					appData.state = APP_STATE_TRIGGERED;
					FPIO1_Clear();
					break;
				}
			}
		
		case APP_STATE_WAIT_SAMPLE:
			if (FPP_DRDY_Get())
			{
				uint8_t inter = 0;
				inter |= (FPP_D0_Get() << 0);
				inter |= (FPP_D1_Get() << 1);
				inter |= (FPP_D2_Get() << 2);
				inter |= (FPP_D3_Get() << 3);
				inter |= (FPP_D4_Get() << 4);
				inter |= (FPP_D5_Get() << 5);
				inter |= (FPP_D6_Get() << 6);
				inter |= (FPP_D7_Get() << 7);
				
				FPP_DREQ_Clear();
				
				appData.state = APP_STATE_GET_SAMPLE;
				
				*(appData.buf.last) = inter;
				appData.buf.last++;
				if (appData.buf.last >= appData.buf.end)
				{
					appData.buf.last = appData.buf.end;
					appData.state = APP_STATE_TRIGGERED;
					FPIO1_Clear();
				}
			}
			break;
		
		case APP_STATE_TRIGGERED:
			break;
		
        case APP_STATE_ERROR:
        default:
            
            break;
    }
	
	comms_task(&(appData.comms));
}
