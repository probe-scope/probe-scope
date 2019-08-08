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

static void
APP_ISR_UnfuckUSB (uintptr_t context)
{
	app_data_t * appData = (app_data_t *) context;
	
	appData->comms.transmitState = RXTX_READY;
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
}

void APP_Tasks(void)
{
	if_task();
    switch(appData.state)
    {
        case APP_STATE_INIT:
			
			SYS_TIME_CallbackRegisterMS(APP_ISR_Blink, (uintptr_t) &appData, 500, SYS_TIME_PERIODIC);
			SYS_TIME_CallbackRegisterMS(APP_ISR_Tick, (uintptr_t) &appData, 1, SYS_TIME_PERIODIC);
			
			appData.usb_timeout = SYS_TIME_TimerCreate(0,
					SYS_TIME_MSToCount(200), APP_ISR_UnfuckUSB,
					(uintptr_t) &appData, SYS_TIME_PERIODIC);
			//SYS_TIME_TimerStart(appData.usb_timeout);
			
			appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;
			
            break;
		
        case APP_STATE_WAIT_FOR_CONFIGURATION:
			
            // Check if the device was configured
            if(appData.comms.isConfigured)
            {
                // If the device is configured then lets start samplin'
                appData.state = APP_STATE_GET_SAMPLE;
            }
            
            break;
		
		case APP_STATE_GET_SAMPLE:
			if (!appData.stop_acq)
			{
				FPP_DREQ_Set();
			
				appData.state = APP_STATE_WAIT_SAMPLE;
				// no break to enable immediate check for response
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
				
				appData.buf.last++;
				if (appData.buf.last >= appData.buf.end)
				{
					appData.buf.last = appData.buf.data;
				}
				if (appData.buf.first == appData.buf.last)
				{
					appData.buf.first++;
					if (appData.buf.first >= appData.buf.end)
					{
						appData.buf.first = appData.buf.data;
					}
				}
				*(appData.buf.last) = inter;
				
				appData.state = APP_STATE_GET_SAMPLE;
			}
			break;
		
		case APP_STATE_SEND_SAMPLE:
			if (RXTX_READY == appData.comms.transmitState && appData.comms_tick)
			{
				uint32_t bytes = sprintf((char *) text_buffer, "%d\r\n", appData.last_sample);
				if(comms_transmit(&(appData.comms), text_buffer, bytes))
				{
					appData.state = APP_STATE_WAIT_USB;
					appData.comms_tick = false;
				}
			}
			break;
		
		case APP_STATE_WAIT_USB:
			if (RXTX_READY == appData.comms.transmitState)
			{
				SYS_TIME_TimerStop(appData.usb_timeout);
				SYS_TIME_TimerStart(appData.usb_timeout);
				appData.state = APP_STATE_GET_SAMPLE;
			}
			break;
		
        case APP_STATE_ERROR:
        default:
            
            break;
    }
	
	comms_task(&(appData.comms));
}
