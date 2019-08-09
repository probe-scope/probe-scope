/*
 * Communications Interface
 * 
 * @Company
 *   Probe-Scope Team
 * 
 * @File Name
 *   comms.c
 * 
 * @Summary
 *   Operates the USB CDC library
*/

#include "comms.h"
#include "interface.h"
#include "usb/src/usb_device_cdc_local.h"
#include "usb/usb_device_cdc.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>


static uint8_t __attribute__((coherent, aligned(16))) read_buffer[CDC_READ_BUFFER_SIZE];


/*******************************************************
 * USB CDC Device Events - Application Event Handler
 *******************************************************/

static USB_DEVICE_CDC_EVENT_RESPONSE COMMS_USBDeviceCDCEventHandler
(
    USB_DEVICE_CDC_INDEX index,
    USB_DEVICE_CDC_EVENT event,
    void * pData,
    uintptr_t userData
)
{
    cdc_comms_t * cdc_comms;
    USB_CDC_CONTROL_LINE_STATE * controlLineStateData;
    USB_DEVICE_CDC_EVENT_DATA_READ_COMPLETE * eventDataRead;
    
    cdc_comms = (cdc_comms_t *)userData;

    switch(event)
    {
        case USB_DEVICE_CDC_EVENT_GET_LINE_CODING:

            /* This means the host wants to know the current line
             * coding. This is a control transfer request. Use the
             * USB_DEVICE_ControlSend() function to send the data to
             * host.  */

            USB_DEVICE_ControlSend(cdc_comms->deviceHandle,
                    &cdc_comms->getLineCodingData, sizeof(USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_LINE_CODING:

            /* This means the host wants to set the line coding.
             * This is a control transfer request. Use the
             * USB_DEVICE_ControlReceive() function to receive the
             * data from the host */

            USB_DEVICE_ControlReceive(cdc_comms->deviceHandle,
                    &cdc_comms->setLineCodingData, sizeof(USB_CDC_LINE_CODING));

            break;

        case USB_DEVICE_CDC_EVENT_SET_CONTROL_LINE_STATE:

            /* This means the host is setting the control line state.
             * Read the control line state. We will accept this request
             * for now. */

            controlLineStateData = (USB_CDC_CONTROL_LINE_STATE *)pData;
            cdc_comms->controlLineStateData.dtr = controlLineStateData->dtr;
            cdc_comms->controlLineStateData.carrier = controlLineStateData->carrier;

            USB_DEVICE_ControlStatus(cdc_comms->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);

            break;

        case USB_DEVICE_CDC_EVENT_SEND_BREAK:

            /* This means that the host is requesting that a break of the
             * specified duration be sent. Read the break duration */

            cdc_comms->breakData = ((USB_DEVICE_CDC_EVENT_DATA_SEND_BREAK *)pData)->breakDuration;
            
            /* Complete the control transfer by sending a ZLP  */
            USB_DEVICE_ControlStatus(cdc_comms->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);
            
            break;

        case USB_DEVICE_CDC_EVENT_READ_COMPLETE:

            /* This means that the host has sent some data*/
            eventDataRead = (USB_DEVICE_CDC_EVENT_DATA_READ_COMPLETE *)pData;
            cdc_comms->receiveState = RXTX_IDLE;
            cdc_comms->receiveBytes = eventDataRead->length; 
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_RECEIVED:

            /* The data stage of the last control transfer is
             * complete. For now we accept all the data */

            USB_DEVICE_ControlStatus(cdc_comms->deviceHandle, USB_DEVICE_CONTROL_STATUS_OK);
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_SENT:

            /* This means the GET LINE CODING function data is valid. We don't
             * do much with this data in this demo. */
            break;

        case USB_DEVICE_CDC_EVENT_WRITE_COMPLETE:

            cdc_comms->writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
            cdc_comms->transmitState = RXTX_IDLE;
            break;

        default:
            break;
    }

    return USB_DEVICE_CDC_EVENT_RESPONSE_NONE;
}

/***********************************************
 * Application USB Device Layer Event Handler.
 ***********************************************/
static void COMMS_USBDeviceEventHandler 
(
    USB_DEVICE_EVENT event, 
    void * eventData, 
    uintptr_t context 
)
{
    USB_DEVICE_EVENT_DATA_CONFIGURED *configuredEventData;
	cdc_comms_t * cdc_comms = (cdc_comms_t *) context;

    switch(event)
    {
        case USB_DEVICE_EVENT_SOF:

            cdc_comms->sofEventHasOccurred = true;
            
            break;

        case USB_DEVICE_EVENT_RESET:

            /* Update LED to show reset state */
            LED2_Clear();

            cdc_comms->isConfigured = false;

            break;

        case USB_DEVICE_EVENT_CONFIGURED:

            /* Check the configuration. We only support configuration 1 */
            configuredEventData = (USB_DEVICE_EVENT_DATA_CONFIGURED*)eventData;
            
            if ( configuredEventData->configurationValue == 1)
            {
                /* Update LED to show configured state */
                LED2_Set();
                
                /* Register the CDC Device application event handler here.
                 * Note how the appData object pointer is passed as the
                 * user data */

                USB_DEVICE_CDC_EventHandlerSet(USB_DEVICE_CDC_INDEX_0, COMMS_USBDeviceCDCEventHandler, (uintptr_t)cdc_comms);

                /* Mark that the device is now configured */
                cdc_comms->isConfigured = true;
            }
            
            break;

        case USB_DEVICE_EVENT_POWER_DETECTED:

            /* VBUS was detected. We can attach the device */
            USB_DEVICE_Attach(cdc_comms->deviceHandle);
            
            break;

        case USB_DEVICE_EVENT_POWER_REMOVED:

            /* VBUS is not available any more. Detach the device. */
            USB_DEVICE_Detach(cdc_comms->deviceHandle);
            
            LED2_Clear();
            
            break;

        case USB_DEVICE_EVENT_SUSPENDED:

            LED2_Clear();
            
            break;

        case USB_DEVICE_EVENT_ERROR:
            LED2_Clear();
        case USB_DEVICE_EVENT_RESUMED:
        default:
            
            break;
    }
}

static bool comms_state_reset(cdc_comms_t * cdc_comms)
{
    // returns true if the device was reset

    bool retVal;

    if(cdc_comms->isConfigured == false)
    {
        cdc_comms->state = CC_STATE_WAIT_FOR_CONFIGURATION;
        cdc_comms->readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        cdc_comms->writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
        cdc_comms->transmitState = RXTX_READY;
        cdc_comms->receiveState = RXTX_READY;
        retVal = true;
    }
    else
    {
        retVal = false;
    }

    return(retVal);
}

void comms_init (
	cdc_comms_t *    cdc_comms,
	SYS_MODULE_INDEX deviceIndex
)
{
    // Place the comms state machine in its initial state.
    cdc_comms->state = CC_STATE_INIT;
	
    // Device Layer Handle  */
    cdc_comms->deviceHandle = USB_DEVICE_HANDLE_INVALID ;
	
    // Device configured status */
    cdc_comms->isConfigured = false;
	
    // Initial get line coding state */
    cdc_comms->getLineCodingData.dwDTERate = 115200;
    cdc_comms->getLineCodingData.bParityType = 0;
    cdc_comms->getLineCodingData.bParityType = 0;
    cdc_comms->getLineCodingData.bDataBits = 8;
	
    // Read Transfer Handle */
    cdc_comms->readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
	
    // Write Transfer Handle */
    cdc_comms->writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
	
    // Initialize internal states
    cdc_comms->transmitState = RXTX_IDLE;
    cdc_comms->receiveState = RXTX_IDLE;
	cdc_comms->receiveBytes = 0;
	cdc_comms->receiveBytesTotal = 0;
	
    // Reset other flags
    cdc_comms->sofEventHasOccurred = false;
	
	
	cdc_comms->deviceIndex = deviceIndex;
	
    // Set up the read buffer
    cdc_comms->cdcReadBuffer = (uint8_t *)malloc(CDC_READ_BUFFER_SIZE);
	cdc_comms->cdcReadBufferSize = CDC_READ_BUFFER_SIZE;
	
	cdc_comms->rx_auto = false;
	cdc_comms->rx_auto_fail = false;
	cdc_comms->rx_auto_terminator = '\0';
	cdc_comms->rx_target_bytes = 0;
	cdc_comms->rx_out = NULL;
}

void comms_task (cdc_comms_t * cdc_comms)
{
    /* Update the comms state machine based
     * on the current state */
    
    switch(cdc_comms->state)
    {
        case CC_STATE_INIT:
            /* Open the device layer */
            cdc_comms->deviceHandle = USB_DEVICE_Open( cdc_comms->deviceIndex, DRV_IO_INTENT_READWRITE );

            if(cdc_comms->deviceHandle != USB_DEVICE_HANDLE_INVALID)
            {
                /* Register a callback with device layer to get event notification */
                USB_DEVICE_EventHandlerSet(cdc_comms->deviceHandle, COMMS_USBDeviceEventHandler, (uintptr_t) cdc_comms);

                cdc_comms->state = CC_STATE_WAIT_FOR_CONFIGURATION;
            }
            else
            {
                /* The Device Layer is not ready to be opened. We should try
                 * again later. */
            }

            break;

		case CC_STATE_WAIT_FOR_CONFIGURATION:
			
			/* Check if the device was configured */
			if(cdc_comms->isConfigured)
			{
				cdc_comms->cdcReadBufferSize = CDC_READ_BUFFER_SIZE;
				cdc_comms->cdcReadBuffer = read_buffer;
				
				if (NULL != cdc_comms->cdcReadBuffer)
				{
					cdc_comms->transmitState = RXTX_READY;
					cdc_comms->receiveState = RXTX_READY;
					cdc_comms->readTransferHandle =
						USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
					cdc_comms->writeTransferHandle =
						USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
					cdc_comms->state = CC_STATE_TRANSACT;
				}
				else
				{
					cdc_comms->state = CC_STATE_ERROR;
				}
			}
			
			break;
		
		case CC_STATE_TRANSACT:
			if (comms_state_reset(cdc_comms))
			{
				break; // USB got reset
			}
			
			if (RXTX_IDLE == cdc_comms->transmitState)
			{
				cdc_comms->transmitState = RXTX_READY;
			}
			
			if (cdc_comms->writeTransferHandle == USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID)
			{
				cdc_comms->transmitState = RXTX_READY;
			}
			
			if (RXTX_IDLE == cdc_comms->receiveState
				&& cdc_comms->receiveBytes > 0)
			{
				if (cdc_comms->rx_auto)
				{
					// If an overrun would happen, reduce apparent bytes read
					cdc_comms->receiveBytesTotal += cdc_comms->receiveBytes;
					if (cdc_comms->receiveBytesTotal > cdc_comms->rx_target_bytes)
					{
						uint32_t diff = cdc_comms->receiveBytesTotal
							- cdc_comms->rx_target_bytes;
						cdc_comms->receiveBytesTotal -= diff;
						cdc_comms->receiveBytes -= diff;
					}
					
					// Try to detect auto terminator character
					uint32_t i;
					for (i = 0;	i < cdc_comms->receiveBytes; i++)
					{
						uint8_t last_char = '\0';
						if (i)
						{
							last_char = cdc_comms->cdcReadBuffer[i - 1];
						}
						else if (cdc_comms->receiveBytesTotal
							> cdc_comms->receiveBytes)
						{
							last_char = cdc_comms->rx_out[
								cdc_comms->receiveBytesTotal
								- cdc_comms->receiveBytes - 1];
						}
						
						if (cdc_comms->cdcReadBuffer[i]
								== cdc_comms->rx_auto_terminator
								&& IF_ESCAPE != last_char)
						{
							// Found it so we're done
							cdc_comms->rx_auto = false;
							cdc_comms->receiveState = RXTX_READY;
							
							// If it's not the last character, make it so
							uint32_t diff = (cdc_comms->receiveBytes - i) - 1;
							cdc_comms->receiveBytesTotal -= diff;
							cdc_comms->receiveBytes -= diff;
							break;
						}
					}
					
					// Go ahead and copy received data to output buffer
					memcpy(&(cdc_comms->rx_out[cdc_comms->receiveBytesTotal
						- cdc_comms->receiveBytes]), cdc_comms->cdcReadBuffer,
						cdc_comms->receiveBytes);
					
					// If we're not done, schedule another read
					if (RXTX_IDLE == cdc_comms->receiveState)
					{
						if (cdc_comms->receiveBytesTotal
							< cdc_comms->rx_target_bytes)
						{
							USB_DEVICE_CDC_Read (cdc_comms->deviceIndex,
								&(cdc_comms->readTransferHandle),
								cdc_comms->cdcReadBuffer,
								cdc_comms->cdcReadBufferSize);
							cdc_comms->receiveState = RXTX_BUSY;
						}
						else
						{
							// We're at the byte limit so just call it off
							cdc_comms->rx_auto = false;
							cdc_comms->rx_auto_fail = true;
							cdc_comms->receiveState = RXTX_READY;
						}
					}
				}
				else
				{
					// If an overrun would happen, reduce apparent bytes read
					cdc_comms->receiveBytesTotal += cdc_comms->receiveBytes;
					if (cdc_comms->receiveBytesTotal > cdc_comms->rx_target_bytes)
					{
						uint32_t diff = cdc_comms->receiveBytesTotal
								- cdc_comms->rx_target_bytes;
						cdc_comms->receiveBytesTotal -= diff;
						cdc_comms->receiveBytes -= diff;
					}
					
					// Go ahead and copy received data to output buffer
					memcpy(&(cdc_comms->rx_out[cdc_comms->receiveBytesTotal
						- cdc_comms->receiveBytes]), cdc_comms->cdcReadBuffer,
						cdc_comms->receiveBytes);
					
					// If we're not done, schedule another read
					if (cdc_comms->receiveBytes >= cdc_comms->rx_target_bytes)
					{
						cdc_comms->receiveState = RXTX_READY;
					}
					else if (cdc_comms->receiveBytes
							< cdc_comms->cdcReadBufferSize)
					{
						USB_DEVICE_CDC_Read (cdc_comms->deviceIndex,
							&(cdc_comms->readTransferHandle),
							cdc_comms->cdcReadBuffer,
							cdc_comms->cdcReadBufferSize);
						cdc_comms->receiveState = RXTX_BUSY;
					}
				}
			}
			break;

        case CC_STATE_ERROR:
        default:
            
            break;
    }
}

bool comms_transmit (cdc_comms_t * cdc_comms, uint8_t * buffer, uint32_t bytes)
{
	if (RXTX_READY != cdc_comms->transmitState)
	{
		return false;
	}
	
	cdc_comms->writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
	
	if (USB_DEVICE_CDC_RESULT_OK == USB_DEVICE_CDC_Write(cdc_comms->deviceIndex,
		&(cdc_comms->writeTransferHandle), buffer,
		bytes, USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE))
	{
		if (cdc_comms->writeTransferHandle != USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID)
		{
			// USB can go so fast it finished before we even get here, so the
			// handler advances state too early and then the state gets stuck
			cdc_comms->transmitState = RXTX_BUSY;
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool
comms_receive (cdc_comms_t * cdc_comms, uint8_t * buffer, uint32_t bytes)
{
	if (RXTX_READY != cdc_comms->receiveState)
	{
		return false;
	}
	
	cdc_comms->rx_auto = false;
	cdc_comms->rx_target_bytes = bytes;
	cdc_comms->rx_out = buffer;
	cdc_comms->receiveBytesTotal = 0;
	cdc_comms->readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
	
	if (USB_DEVICE_CDC_RESULT_OK == USB_DEVICE_CDC_Read (cdc_comms->deviceIndex,
		&(cdc_comms->readTransferHandle), cdc_comms->cdcReadBuffer,
		cdc_comms->cdcReadBufferSize))
	{
		cdc_comms->receiveState = RXTX_BUSY;
		return true;
	}
	else
	{
		return false;
	}
	return true;
}

bool
comms_receive_auto (cdc_comms_t * cdc_comms, uint8_t * buffer,
					uint32_t bytes, uint8_t terminator)
{
	if (RXTX_READY != cdc_comms->receiveState)
	{
		return false;
	}
	
	cdc_comms->rx_auto = true;
	cdc_comms->rx_target_bytes = bytes;
	cdc_comms->rx_auto_terminator = terminator;
	cdc_comms->rx_out = buffer;
	cdc_comms->receiveBytesTotal = 0;
	cdc_comms->readTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
	
	if (USB_DEVICE_CDC_RESULT_OK == USB_DEVICE_CDC_Read (cdc_comms->deviceIndex,
		&(cdc_comms->readTransferHandle), cdc_comms->cdcReadBuffer,
		cdc_comms->cdcReadBufferSize))
	{
		cdc_comms->receiveState = RXTX_BUSY;
		return true;
	}
	else
	{
		cdc_comms->rx_auto = false;
		return false;
	}
	return true;
}

