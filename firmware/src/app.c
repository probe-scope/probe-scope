/*******************************************************************************
 End of File
 */


/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include <stdio.h>
#include <string.h>


// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

uint8_t CACHE_ALIGN cdcReadBuffer[APP_READ_BUFFER_SIZE];
uint8_t CACHE_ALIGN cdcWriteBuffer[APP_READ_BUFFER_SIZE];


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;


// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

static void
APP_ISR_Blink (uintptr_t context)
{
	APP_DATA * appData = (APP_DATA *) context;
	
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
	
	if (!PIC_SW_Get())
	{
		LED1_Set();
	}
	else
	{
		LED1_Clear();
	}
}

static void
APP_ISR_Tick (uintptr_t context)
{
	APP_DATA * appData = (APP_DATA *) context;
	
	appData->comms_tick = true;
}



// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize(void)

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize(void)
{
    // Place the App state machine in its initial state.
    appData.state = APP_STATE_INIT;
	
	// Set up USB comms
	comms_init(&(appData.comms), USB_DEVICE_INDEX_0, cdcReadBuffer, APP_READ_BUFFER_SIZE, cdcWriteBuffer, APP_READ_BUFFER_SIZE);
	
	appData.blink_tick = false;
	appData.comms_tick = false;
	
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


/******************************************************************************
  Function:
    void APP_Tasks(void)

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks(void)
{   
    switch(appData.state)
    {
        case APP_STATE_INIT:
			
			SYS_TIME_CallbackRegisterMS(APP_ISR_Blink, (uintptr_t) &appData, 500, SYS_TIME_PERIODIC);
			SYS_TIME_CallbackRegisterMS(APP_ISR_Tick, (uintptr_t) &appData, 50, SYS_TIME_PERIODIC);
			
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
			FPP_DREQ_Set();
			
			appData.state = APP_STATE_WAIT_SAMPLE;
			// no break to enable immediate check for response
		
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
				
				appData.last_sample = (int8_t) inter;
				appData.state = APP_STATE_SEND_SAMPLE;
			}
			break;
		
		case APP_STATE_SEND_SAMPLE:
			if (RXTX_IDLE == appData.comms.transmitState && appData.comms_tick)
			{
				uint32_t bytes = sprintf((char *) cdcWriteBuffer, "%d\r\n", appData.last_sample);
				if(comms_transmit(&(appData.comms), bytes))
				{
					appData.state = APP_STATE_WAIT_USB;
					appData.comms_tick = false;
				}
			}
			break;
		
		case APP_STATE_WAIT_USB:
			if (RXTX_IDLE == appData.comms.transmitState)
			{
				appData.state = APP_STATE_GET_SAMPLE;
			}
			break;
		
        case APP_STATE_ERROR:
        default:
            
            break;
    }
	
	comms_task(&(appData.comms));
}

/*******************************************************************************
 End of File
 */
