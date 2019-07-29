/*
 * Communications Interface
 * 
 * @Company
 *   Probe-Scope Team
 * 
 * @File Name
 *   comms.h
 * 
 * @Summary
 *   Operates the USB CDC library
*/

#ifndef COMMS_H
#define	COMMS_H


#include "definitions.h"


#ifdef	__cplusplus
extern "C"
{
#endif


typedef enum
{
    // State machine's initial state.
    CC_STATE_INIT=0,
	
    // Wait for device configuration
    CC_STATE_WAIT_FOR_CONFIGURATION,
	
	// Main transaction processor
	CC_STATE_TRANSACT,
	
	/*
    // Wait for a character receive
    CC_STATE_SCHEDULE_READ,
	
    // A character is received from host
    CC_STATE_WAIT_FOR_READ_COMPLETE,
	
    // Wait for the TX to get completed
    CC_STATE_SCHEDULE_WRITE,
	
    // Wait for the write to complete
    CC_STATE_WAIT_FOR_WRITE_COMPLETE,
	*/
	
    // Error state
    CC_STATE_ERROR
} cdc_comms_states_t;

typedef enum
{
	RXTX_IDLE,
	RXTX_READY,
	RXTX_BUSY
} cdc_rxtx_state_t;

typedef struct
{
	SYS_MODULE_INDEX deviceIndex;
	
    /* Device layer handle returned by device layer open function */
    USB_DEVICE_HANDLE deviceHandle;

    /* Application's current state*/
    cdc_comms_states_t state;

    /* Set Line Coding Data */
    USB_CDC_LINE_CODING setLineCodingData;

    /* Device configured state */
    bool isConfigured;

    /* Get Line Coding Data */
    USB_CDC_LINE_CODING getLineCodingData;

    /* Control Line State */
    USB_CDC_CONTROL_LINE_STATE controlLineStateData;

    /* Read transfer handle */
    USB_DEVICE_CDC_TRANSFER_HANDLE readTransferHandle;

    /* Write transfer handle */
    USB_DEVICE_CDC_TRANSFER_HANDLE writeTransferHandle;

    cdc_rxtx_state_t transmitState;
	cdc_rxtx_state_t receiveState;
	
	uint32_t transmitBytes;
	uint32_t receiveBytes;

    /* Flag determines SOF event occurrence */
    bool sofEventHasOccurred;

    /* Break data */
    uint16_t breakData;

    /* Application CDC read buffer */
    uint8_t * cdcReadBuffer;
	uint32_t  cdcReadBufferSize;

    /* Application CDC Write buffer */
    uint8_t * cdcWriteBuffer;
	uint32_t  cdcWriteBufferSize;
} cdc_comms_t;

void comms_init (
	cdc_comms_t *    cdc_comms,
	SYS_MODULE_INDEX deviceIndex,
	uint8_t *        readBuffer,
	uint32_t         readBufferSize,
	uint8_t *        writeBuffer,
	uint32_t         writeBufferSize
);
void comms_task (cdc_comms_t * cdc_comms);

bool comms_transmit (cdc_comms_t * cdc_comms, uint32_t bytes);


#ifdef	__cplusplus
}
#endif

#endif	/* COMMS_H */

