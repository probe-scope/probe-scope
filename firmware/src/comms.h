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


#define CDC_READ_BUFFER_SIZE 0x200


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
	
	uint32_t receiveBytes;
	uint32_t receiveBytesTotal;

    /* Flag determines SOF event occurrence */
    bool sofEventHasOccurred;

    /* Break data */
    uint16_t breakData;

    /* Application CDC read buffer */
    uint8_t * cdcReadBuffer;
	uint32_t  cdcReadBufferSize;
	
	bool rx_auto;
	bool rx_auto_fail;
	uint8_t rx_auto_terminator;
	uint32_t rx_target_bytes;
	uint8_t * rx_out;
} cdc_comms_t;

void comms_init (cdc_comms_t * cdc_comms, SYS_MODULE_INDEX deviceIndex);
void comms_task (cdc_comms_t * cdc_comms);

bool comms_transmit (cdc_comms_t * cdc_comms, uint8_t * buffer, uint32_t bytes);
bool comms_receive (cdc_comms_t * cdc_comms, uint8_t * buffer, uint32_t bytes);
bool comms_receive_auto (cdc_comms_t * cdc_comms, uint8_t * buffer,
						 uint32_t bytes, uint8_t terminator);


#ifdef	__cplusplus
}
#endif

#endif	/* COMMS_H */

