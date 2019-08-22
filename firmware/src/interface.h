/*
 * Host Interface
 * 
 * @Company
 *   Probe-Scope Team
 * 
 * @File Name
 *   interface.h
 * 
 * @Summary
 *   Interface according to Probe-Scope CDC Interface Spec v1.0.docx
*/

#ifndef INTERFACE_H
#define	INTERFACE_H


#include "app.h"

#include <stdbool.h>
#include <stdint.h>


#define IF_START_MESSAGE  0x1E
#define IF_END_MESSAGE    0x04
#define IF_END_BLOCK      0x17
#define IF_ESCAPE         0x1A

#define IF_VM_PIC_START   0x00001000
#define IF_VM_PIC_END     0x00001007
#define IF_VM_PIC_LENGTH  (IF_VM_PIC_END - IF_VM_PIC_START + 1)
#define IF_VM_FPGA_START  0x00002000
#define IF_VM_FPGA_END    0x00002001
#define IF_VM_FPGA_LENGTH (IF_VM_FPGA_END - IF_VM_FPGA_START + 1)
#define IF_VM_PGA_START   0x00003000
#define IF_VM_PGA_END     0x00003001
#define IF_VM_PGA_LENGTH  (IF_VM_PGA_END - IF_VM_PGA_START + 1)
#define IF_VM_DAC_START   0x00004000
#define IF_VM_DAC_END     0x00004010
#define IF_VM_DAC_LENGTH  (IF_VM_DAC_END - IF_VM_DAC_START + 1)

#define SAMPLE_DATA_LENGTH_FI 'L'
#define SAMPLE_DATA_FI        'D'
#define WRITE_ADDRESS_FI      'A'
#define WRITE_LENGTH_FI       'L'
#define WRITE_DATA_FI         'D'
#define READ_ADDRESS_FI       'A'
#define READ_LENGTH_FI        'L'
#define READ_DATA_FI          'D'

#define BUFFER_MARGIN 100U
#define MAX_INCOMING_MESSAGE (256U * 2U + 15U + BUFFER_MARGIN)
#define MAX_OUTGOING_MESSAGE (SAMPLES * 2 + BUFFER_MARGIN)


#ifdef	__cplusplus
extern "C"
{
#endif


typedef enum
{
	IF_STATE_INIT = 0,
	IF_STATE_WAIT,
	IF_STATE_PROCESS_HOST_MSG,
	IF_STATE_SEND_HW_MSG
} if_states_t;

typedef enum
{
	IF_TX_STATE_WAIT = 0,
	IF_TX_STATE_ENCODE,
	IF_TX_STATE_TRANSMIT,
	IF_TX_STATE_TRANSMITTING		
} if_tx_states_t;

typedef enum
{
	IF_RX_STATE_WAIT = 0,
	IF_RX_STATE_RECEIVING,
	IF_RX_STATE_DECODE,
	IF_RX_STATE_MSG_READY
} if_rx_states_t;

typedef enum
{
	IF_MSG_COMMAND = 0x43, // 'C'
	IF_MSG_RESULT  = 0x52  // 'R'
} if_msg_type_t;

typedef enum
{
	IF_CMD_REQ_SAMP_DATA = 0x73, // 's'
	IF_CMD_TRIGGERED =     0x74, // 't'
	IF_CMD_WRITE_REGS =    0x77, // 'w'
	IF_CMD_READ_REGS =     0x72  // 'r'
} if_commands_t;


typedef struct __attribute__((__packed__))
{
	char     sample_data_length_fi;
	uint32_t sample_data_length;
	char     sample_data_fi;
} if_res_data_req_samp_data_t;

typedef struct __attribute__((__packed__))
{
	char     write_address_fi;
	uint32_t write_address;
	char     write_length_fi;
	uint32_t write_length;
	char     write_data_fi;
} if_cmd_data_write_regs_t;

typedef struct __attribute__((__packed__))
{
	char     write_length_fi;
	uint32_t write_length;
} if_res_data_write_regs_t;

typedef struct __attribute__((__packed__))
{
	char     read_address_fi;
	uint32_t read_address;
	char     read_length_fi;
	uint32_t read_length;
} if_cmd_data_read_regs_t;

typedef struct __attribute__((__packed__))
{
	char     read_length_fi;
	uint32_t read_length;
	char     read_data_fi;
} if_res_data_read_regs_t;

typedef union
{
	if_res_data_req_samp_data_t res_data_req_samp_data;
	if_cmd_data_write_regs_t    cmd_data_write_regs;
	if_res_data_write_regs_t    res_data_write_regs;
	if_cmd_data_read_regs_t     cmd_data_read_regs;
	if_res_data_read_regs_t     res_data_read_regs;
	uint8_t                     bytes[1];
} if_msg_data_t;

typedef struct
{
	if_msg_type_t   type;
	if_commands_t   command;
	if_msg_data_t   data;
	unsigned char * var_data;
	unsigned int    length;
} if_message_t;

typedef struct
{
	if_states_t     state;
	
	if_tx_states_t  tx_state;
	unsigned int    tx_error_count;
	bool            tx_error_flag;
	
	if_rx_states_t  rx_state;
	unsigned int    rx_error_count;
	bool            rx_error_flag;
	
	if_message_t    tx_msg;
	if_message_t    rx_msg;
	
	bool            wait_trigger;
	
	// ext if
	DRV_HANDLE      h_i2c;
	DRV_HANDLE      h_spi;
} if_data_t;


void if_init (void);
void if_task (void);


#ifdef	__cplusplus
}
#endif

#endif	/* INTERFACE_H */

