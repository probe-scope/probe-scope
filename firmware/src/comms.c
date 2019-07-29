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

            /* This means that the data write got completed. We can schedule
             * the next read. */

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

        case USB_DEVICE_EVENT_RESUMED:
        case USB_DEVICE_EVENT_ERROR:
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
        cdc_comms->transmitState = RXTX_IDLE;
        cdc_comms->receiveState = RXTX_IDLE;
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
	SYS_MODULE_INDEX deviceIndex,
	uint8_t *        readBuffer,
	uint32_t         readBufferSize,
	uint8_t *        writeBuffer,
	uint32_t         writeBufferSize
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
	cdc_comms->transmitBytes = 0;
	cdc_comms->receiveBytes = 0;
	
    // Reset other flags
    cdc_comms->sofEventHasOccurred = false;
	
	
	cdc_comms->deviceIndex = deviceIndex;
	
    // Set up the read buffer
    cdc_comms->cdcReadBuffer = readBuffer;
	cdc_comms->cdcReadBufferSize = readBufferSize;
	
    // Set up the read buffer
    cdc_comms->cdcWriteBuffer = writeBuffer;
	cdc_comms->cdcWriteBufferSize = writeBufferSize;
}

void comms_task (cdc_comms_t * cdc_comms)
{
    /* Update the comms state machine based
     * on the current state */
    //int i;
    
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
                /* If the device is configured then lets start reading */
                cdc_comms->state = CC_STATE_TRANSACT;
            }
            
            break;
		
		case CC_STATE_TRANSACT:
			comms_state_reset(cdc_comms);
			
			switch (cdc_comms->transmitState)
			{
				case RXTX_READY:
					cdc_comms->writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
					cdc_comms->transmitState = RXTX_BUSY;
					
					USB_DEVICE_CDC_Write(cdc_comms->deviceIndex,
							&(cdc_comms->writeTransferHandle),
							cdc_comms->cdcWriteBuffer, cdc_comms->transmitBytes,
							USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);
					break;
				
				// Other state transfers are accomplished elsewhere
				case RXTX_BUSY:
				case RXTX_IDLE:
				default:
					break;
			}
			break;

		/*
        case CC_STATE_SCHEDULE_READ:

            if(comms_state_reset(cdc_comms))
            {
                break;
            }

            // If a read is complete, then schedule a read
            // else wait for the current read to complete

            cdc_comms->state = CC_STATE_WAIT_FOR_READ_COMPLETE;
            if(cdc_comms->isReadComplete == true)
            {
                cdc_comms->isReadComplete = false;
                cdc_comms->readTransferHandle =  USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;

                USB_DEVICE_CDC_Read (cdc_comms->deviceIndex,
                        &(cdc_comms->readTransferHandle), cdc_comms->cdcReadBuffer,
                        cdc_comms->cdcReadBufferSize);
                
                if(cdc_comms->readTransferHandle == USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID)
                {
                    cdc_comms->state = CC_STATE_ERROR;
                    break;
                }
            }

            break;

        case CC_STATE_WAIT_FOR_READ_COMPLETE:

            if(comms_state_reset(cdc_comms))
            {
                break;
            }

            // Check if a character was received the isReadComplete flag gets 
            // updated in the CDC event handler.

            if(cdc_comms->isReadComplete)
            {
                cdc_comms->state = CC_STATE_SCHEDULE_WRITE;
            }

            break;


        case CC_STATE_SCHEDULE_WRITE:

            if(comms_state_reset(cdc_comms))
            {
                break;
            }

            // Setup the write

            cdc_comms->writeTransferHandle = USB_DEVICE_CDC_TRANSFER_HANDLE_INVALID;
            cdc_comms->isWriteComplete = false;
            cdc_comms->state = CC_STATE_WAIT_FOR_WRITE_COMPLETE;

            // Else echo each received character by adding 1
            for(i = 0; i < cdc_comms->numBytesRead; i++)
            {
                if((cdc_comms->cdcReadBuffer[i] != 0x0A) && (cdc_comms->cdcReadBuffer[i] != 0x0D))
                {
                    cdc_comms->cdcWriteBuffer[i] = cdc_comms->cdcReadBuffer[i] + 1;
                }
            }
            USB_DEVICE_CDC_Write(cdc_comms->deviceIndex,
                    &(cdc_comms->writeTransferHandle),
                    cdc_comms->cdcWriteBuffer, cdc_comms->numBytesRead,
                    USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);

            break;

        case CC_STATE_WAIT_FOR_WRITE_COMPLETE:

            if(comms_state_reset(cdc_comms))
            {
                break;
            }

            // Check if a character was sent. The isWriteComplete
            // flag gets updated in the CDC event handler

            if(cdc_comms->isWriteComplete == true)
            {
                cdc_comms->state = CC_STATE_SCHEDULE_READ;
            }

            break;
		*/

        case CC_STATE_ERROR:
        default:
            
            break;
    }
}

bool comms_transmit (cdc_comms_t * cdc_comms, uint32_t bytes)
{
	if (RXTX_IDLE != cdc_comms->transmitState)
	{
		return false;
	}
	
	cdc_comms->transmitBytes = bytes;
	cdc_comms->transmitState = RXTX_READY;
	return true;
}
