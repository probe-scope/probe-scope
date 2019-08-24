/*
 * Host Interface
 * 
 * @Company
 *   Probe-Scope Team
 * 
 * @File Name
 *   interface.c
 * 
 * @Summary
 *   Interface according to Probe-Scope CDC Interface Spec v1.0.docx
*/

#include "interface.h"

#include "app.h"
#include "config/default/driver/i2c/drv_i2c.h"
#include "config/default/driver/i2c/drv_i2c_definitions.h"
#include "config/default/peripheral/i2c/plib_i2c4.h"
#include "config/default/peripheral/i2c/plib_i2c_master.h"
#include "FreeRTOS.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static if_data_t if_data;

static uint8_t __attribute__((coherent, aligned(16))) out_buffer[MAX_OUTGOING_MESSAGE];
static uint8_t in_buffer[MAX_INCOMING_MESSAGE];


static void if_rx_task (void);
static void if_tx_task (void);

static void if_send_samp_data (void);
static void if_send_trigger (void);

static void if_vm_dispatch (void);
static void if_vm_pic_op   (uint32_t address, uint32_t length);
static void if_vm_dac_op   (uint32_t address, uint32_t length);
static void if_vm_fpga_op  (uint32_t address, uint32_t length);
static void if_vm_afe_op   (uint32_t address, uint32_t length);
static void if_vm_respstp  (uint32_t length);


void
if_init (void)
{
	
	if_data.state = IF_STATE_INIT;
	
	if_data.tx_state = IF_TX_STATE_WAIT;
	if_data.tx_error_count = 0;
	if_data.tx_error_flag = false;
	
	if_data.rx_state = IF_RX_STATE_WAIT;
	if_data.rx_error_count = 0;
	if_data.rx_error_flag = false;
	
	memset(&(if_data.rx_msg), 0, sizeof(if_data.rx_msg));
	
	if_data.h_spi_fpga = DRV_HANDLE_INVALID;
	if_data.h_spi_afe = DRV_HANDLE_INVALID;
	if_data.h_i2c_dac = DRV_HANDLE_INVALID;
}

void
if_task (void)
{
	switch (if_data.state)
	{
		case IF_STATE_INIT:
			if_data.state = IF_STATE_WAIT;
			
			// init fpga spi
			if (DRV_HANDLE_INVALID == if_data.h_spi_fpga)
			{
				if_data.h_spi_fpga =
					DRV_SPI_Open(DRV_SPI_INDEX_0, DRV_IO_INTENT_EXCLUSIVE);
			}
			if (DRV_HANDLE_INVALID == if_data.h_spi_fpga)
			{
				if_data.state = IF_STATE_INIT;
			}
			else
			{
				// bare minimum configuration for now
			}
			
			// init afe (filter/pga) spi
			if (DRV_HANDLE_INVALID == if_data.h_spi_afe)
			{
				if_data.h_spi_afe =
					DRV_SPI_Open(DRV_SPI_INDEX_1, DRV_IO_INTENT_EXCLUSIVE);
			}
			if (DRV_HANDLE_INVALID == if_data.h_spi_afe)
			{
				if_data.state = IF_STATE_INIT;
			}
			else
			{
				// bare minimum configuration for now
			}
			
			// init dac i2c
			if (DRV_HANDLE_INVALID == if_data.h_i2c_dac)
			{
				if_data.h_i2c_dac =
					DRV_I2C_Open(DRV_I2C_INDEX_0, DRV_IO_INTENT_EXCLUSIVE);
			}
			if (DRV_HANDLE_INVALID == if_data.h_i2c_dac)
			{
				if_data.state = IF_STATE_INIT;
			}
			else
			{
				// bare minimum configuration for now
			}
			
			break;
		
		case IF_STATE_WAIT:
			if_rx_task();

			if (IF_RX_STATE_MSG_READY == if_data.rx_state)
			{
				if_data.state = IF_STATE_PROCESS_HOST_MSG;
			}
			
			if (APP_STATE_TRIGGERED == appData.state)
			{
				appData.state = APP_STATE_WAIT_TRIGGER;
				
				if (if_data.wait_trigger)
				{
					if_send_samp_data();
					if_data.wait_trigger = false;
				}
				else
				{
					//if_send_trigger();
				}
				
				if_data.state = IF_STATE_SEND_HW_MSG;
			}
			break;
		
		case IF_STATE_PROCESS_HOST_MSG:
			switch (if_data.rx_msg.command)
			{
				case IF_CMD_REQ_SAMP_DATA:
					if_data.wait_trigger = true;
					break;
				
				case IF_CMD_WRITE_REGS:
				case IF_CMD_READ_REGS:
					if_vm_dispatch();
					break;
				
				default:
					// real bad
					break;
			}
			
			if_data.rx_state = IF_RX_STATE_WAIT;
			if_data.state = IF_STATE_SEND_HW_MSG;
			break;
		
		case IF_STATE_SEND_HW_MSG:
			if (IF_TX_STATE_WAIT == if_data.tx_state)
			{
				if_data.state = IF_STATE_WAIT;
			}
			else
			{
				if_tx_task();
			}
			break;
		
		default:
			if_data.state = IF_STATE_INIT;
			break;
	}
}


static int
get_msg_data_len (if_message_t msg)
{
	int data_len = 0;
	switch (msg.type)
	{
		case IF_MSG_COMMAND:
			switch (msg.command)
			{
				case IF_CMD_REQ_SAMP_DATA:
					data_len = 0;
					break;

				case IF_CMD_WRITE_REGS:
					data_len = 11;
					break;

				case IF_CMD_READ_REGS:
					data_len = 10;
					break;
				
				case IF_CMD_TRIGGERED:
					data_len = 0;
					break;

				default:
					data_len = -1;
					break;
			}
			break;
		
		case IF_MSG_RESULT:
			switch (msg.command)
			{
				case IF_CMD_REQ_SAMP_DATA:
					data_len = 6;
					break;

				case IF_CMD_WRITE_REGS:
					data_len = 5;
					break;

				case IF_CMD_READ_REGS:
					data_len = 6;
					break;

				default:
					data_len = -1;
					break;
			}
			break;
		
		default:
			data_len = -1;
			break;
	}
	return data_len;
}

static int
get_msg_var_data_len (if_message_t msg)
{
	int var_data_len = 0;
	switch (msg.type)
	{
		case IF_MSG_COMMAND:
			switch (msg.command)
			{
				case IF_CMD_REQ_SAMP_DATA:
					var_data_len = 0;
					break;

				case IF_CMD_WRITE_REGS:
					var_data_len = msg.data.cmd_data_write_regs.write_length;
					break;

				case IF_CMD_READ_REGS:
					var_data_len = 0;
					break;
				
				case IF_CMD_TRIGGERED:
					var_data_len = 0;
					break;

				default:
					var_data_len = -1;
					break;
			}
			break;
		
		case IF_MSG_RESULT:
			switch (msg.command)
			{
				case IF_CMD_REQ_SAMP_DATA:
					var_data_len =
						msg.data.res_data_req_samp_data.sample_data_length;
					break;

				case IF_CMD_WRITE_REGS:
					var_data_len = 0;
					break;

				case IF_CMD_READ_REGS:
					var_data_len = msg.data.res_data_read_regs.read_length;
					break;

				default:
					var_data_len = -1;
					break;
			}
			break;
		
		default:
			var_data_len = -1;
			break;
	}
	return var_data_len;
}


static void
if_rx_task (void)
{
	switch (if_data.rx_state)
	{
		case IF_RX_STATE_WAIT:
			if (true == comms_receive_auto(gp_comms, in_buffer,
				MAX_INCOMING_MESSAGE, IF_END_MESSAGE))
			{
				if_data.rx_state = IF_RX_STATE_RECEIVING;
			}
			break;
		
		case IF_RX_STATE_RECEIVING:
			if (!gp_comms->rx_auto)
			{
				if (gp_comms->rx_auto_fail)
				{
					if_data.rx_error_count++;
					if_data.rx_error_flag = true;
					if_data.rx_state = IF_RX_STATE_WAIT;
				}
				else
				{
					if_data.rx_state = IF_RX_STATE_DECODE;
				}
			}
			break;
		
		case IF_RX_STATE_DECODE:
			if (in_buffer[0] != IF_START_MESSAGE)
			{
				if_data.rx_error_count++;
				if_data.rx_error_flag = true;
				if_data.rx_state = IF_RX_STATE_WAIT;
				break;
			}
			
			if_data.rx_msg.type = in_buffer[1];
			if (IF_MSG_COMMAND != if_data.rx_msg.type)
			{
				if_data.rx_error_count++;
				if_data.rx_error_flag = true;
				if_data.rx_state = IF_RX_STATE_WAIT;
				break;
			}
			
			if_data.rx_msg.command = in_buffer[2];
			
			int data_len = get_msg_data_len(if_data.rx_msg);
			if (data_len < 0)
			{
				if_data.rx_error_count++;
				if_data.rx_error_flag = true;
				if_data.rx_state = IF_RX_STATE_WAIT;
				break;
			}
			
			unsigned int i, o = 0;
			for (i = 0; i < data_len; i++)
			{
				if (IF_ESCAPE == in_buffer[3 + i + o])
				{
					o++;
				}
				
				if_data.rx_msg.data.bytes[i] = in_buffer[3 + i + o];
			}
			
			
			int var_data_len = get_msg_var_data_len(if_data.rx_msg);
			if (var_data_len < 0 || var_data_len > 256)
			{
				if_data.rx_error_count++;
				if_data.rx_error_flag = true;
				if_data.rx_state = IF_RX_STATE_WAIT;
				break; 
			}
			
			for (i = 0; i < var_data_len; i++)
			{
				if (IF_ESCAPE == in_buffer[3 + data_len + i + o])
				{
					o++;
				}
				
				in_buffer[i] = in_buffer[3 + data_len + i + o];
			}
			if_data.rx_msg.var_data = in_buffer;
			if_data.rx_msg.length = var_data_len;
			
			if_data.rx_state = IF_RX_STATE_MSG_READY;
			break;
		
		case IF_RX_STATE_MSG_READY:
		default:
			break;
	}
}

static void
if_tx_task (void)
{
	switch (if_data.tx_state)
	{
		case IF_TX_STATE_ENCODE:
			out_buffer[0] = IF_START_MESSAGE;
			out_buffer[1] = if_data.tx_msg.type;
			out_buffer[2] = if_data.tx_msg.command;
			
			int data_len = get_msg_data_len(if_data.tx_msg);
			int var_data_len = get_msg_var_data_len(if_data.tx_msg);
			
			unsigned int i, o = 0;
			for (i = 0; i < data_len; i++)
			{
				uint8_t c = if_data.tx_msg.data.bytes[i];
				if (IF_ESCAPE == c || IF_END_MESSAGE == c
					|| IF_END_BLOCK == c || IF_START_MESSAGE == c)
				{
					out_buffer[3 + i + o] = IF_ESCAPE;
					o++;
				}
				
				out_buffer[3 + i + o] = if_data.tx_msg.data.bytes[i];
			}
			for (i = 0; i < var_data_len; i++)
			{
				uint8_t c = if_data.tx_msg.var_data[i];
				if (IF_ESCAPE == c || IF_END_MESSAGE == c
					|| IF_END_BLOCK == c || IF_START_MESSAGE == c)
				{
					out_buffer[3 + data_len + i + o] = IF_ESCAPE;
					o++;
				}
				
				out_buffer[3 + data_len + i + o] = if_data.tx_msg.var_data[i];
			}
			out_buffer[3 + data_len + var_data_len + o] = IF_END_MESSAGE;
			if_data.tx_msg.length = 3 + data_len + var_data_len + o + 1;
			
			if_data.tx_state = IF_TX_STATE_TRANSMIT;
			break;
		
		case IF_TX_STATE_TRANSMIT:
			if(comms_transmit(gp_comms, out_buffer, if_data.tx_msg.length))
			{
				if_data.tx_state = IF_TX_STATE_TRANSMITTING;
			}
		
		case IF_TX_STATE_TRANSMITTING:
			if (gp_comms->transmitState == RXTX_READY)
			{
				if_data.tx_state = IF_TX_STATE_WAIT;
			}
			break;
		
		case IF_TX_STATE_WAIT:
		default:
			break;
	}
}


static void
if_send_samp_data (void)
{
	if_data.tx_msg.type = IF_MSG_RESULT;
	if_data.tx_msg.command = IF_CMD_REQ_SAMP_DATA;
	
	if_data.tx_msg.data.res_data_req_samp_data.sample_data_fi =
		SAMPLE_DATA_FI;
	if_data.tx_msg.data.res_data_req_samp_data.sample_data_length_fi =
		SAMPLE_DATA_LENGTH_FI;
	if_data.tx_msg.data.res_data_req_samp_data.sample_data_length = SAMPLES;
	
	if_data.tx_msg.var_data = appData.buf.data;
	
	if_data.tx_state = IF_TX_STATE_ENCODE;
}

static void
if_send_trigger (void)
{
	if_data.tx_msg.type = IF_MSG_COMMAND;
	if_data.tx_msg.command = IF_CMD_TRIGGERED;
	if_data.tx_state = IF_TX_STATE_ENCODE;
}


static void
if_vm_dispatch (void)
{
	// Dispatch a read or write register command to the correct driver
	
	uint32_t address, length;
	
	if (IF_CMD_WRITE_REGS == if_data.rx_msg.command)
	{
		address = if_data.rx_msg.data.cmd_data_write_regs.write_address;
		length  = if_data.rx_msg.data.cmd_data_write_regs.write_length;
	}
	else if (IF_CMD_READ_REGS == if_data.rx_msg.command)
	{
		address = if_data.rx_msg.data.cmd_data_read_regs.read_address;
		length  = if_data.rx_msg.data.cmd_data_read_regs.read_length;
	}
	else
	{
		// Someone called this at the wrong time, do nothing
		return;
	}
	
	if ((IF_VM_PIC_START <= address)
		&& (IF_VM_PIC_END >= (address + length - 1)))
	{
		if_vm_pic_op(address - IF_VM_PIC_START, length);
	}
	else if ((IF_VM_FPGA_START <= address)
		&& (IF_VM_FPGA_END >= (address + length - 1)))
	{
		if_vm_fpga_op(address - IF_VM_FPGA_START, length);
	}
	else if ((IF_VM_AFE_START <= address)
		&& (IF_VM_AFE_END >= (address + length - 1)))
	{
		if_vm_afe_op(address - IF_VM_AFE_START, length);
	}
	else if ((IF_VM_DAC_START <= address)
		&& (IF_VM_DAC_END >= (address + length - 1)))
	{
		if_vm_dac_op(address - IF_VM_DAC_START, length);
	}
	else
	{
		// Invalid address and/or length
		if_vm_respstp(0);
	}
}

static void
if_vm_pic_op (uint32_t address, uint32_t length)
{
	// Process read/write command that maps to PIC memory.
	// This MUST only be called by if_vm_dispatch, as all the parameter
	// validation is done there.
	
	if (IF_CMD_WRITE_REGS == if_data.rx_msg.command)
	{
		memcpy(&(if_data.vm_pic_buf[address]), if_data.rx_msg.var_data, length);
		if_vm_respstp(length);
	}
	else if (IF_CMD_READ_REGS == if_data.rx_msg.command)
	{
		if_vm_respstp(length);
		if_data.tx_msg.var_data = &(if_data.vm_pic_buf[address]);
	}
}

static void
if_vm_dac_op (uint32_t address, uint32_t length)
{
	// Process read/write command that maps to DAC memory.
	// This MUST only be called by if_vm_dispatch, as all the parameter
	// validation is done there.
	
	if (IF_CMD_WRITE_REGS == if_data.rx_msg.command)
	{
		memcpy(&(if_data.vm_dac_buf[address]), if_data.rx_msg.var_data, length);
		if_vm_respstp(length);
	}
	else if (IF_CMD_READ_REGS == if_data.rx_msg.command)
	{
		if_vm_respstp(length);
		if_data.tx_msg.var_data = &(if_data.vm_dac_buf[address]);
	}
}

static void
if_vm_fpga_op (uint32_t address, uint32_t length)
{
	// Process read/write command that maps to FPGA memory.
	// This MUST only be called by if_vm_dispatch, as all the parameter
	// validation is done there.
	
	if (IF_CMD_WRITE_REGS == if_data.rx_msg.command)
	{
		memcpy(&(if_data.vm_fpga_buf[address]), if_data.rx_msg.var_data,
			length);
		if_vm_respstp(length);
	}
	else if (IF_CMD_READ_REGS == if_data.rx_msg.command)
	{
		if_vm_respstp(length);
		if_data.tx_msg.var_data = &(if_data.vm_fpga_buf[address]);
	}
}

static void
if_vm_afe_op (uint32_t address, uint32_t length)
{
	// Process read/write command that maps to AFE memory.
	// This MUST only be called by if_vm_dispatch, as all the parameter
	// validation is done there.
	
	if (IF_CMD_WRITE_REGS == if_data.rx_msg.command)
	{
		memcpy(&(if_data.vm_afe_buf[address]), if_data.rx_msg.var_data, length);
		if_vm_respstp(length);
	}
	else if (IF_CMD_READ_REGS == if_data.rx_msg.command)
	{
		if_vm_respstp(length);
		if_data.tx_msg.var_data = &(if_data.vm_afe_buf[address]);
	}
}

static void
if_vm_respstp (uint32_t length)
{
	// Set up the appropriate response for a read or write command
	// For write, the user still needs to set the var_data pointer
	
	if (IF_CMD_WRITE_REGS == if_data.rx_msg.command)
	{
		if_data.tx_msg.type = IF_MSG_RESULT;
		if_data.tx_msg.command = IF_CMD_WRITE_REGS;
		if_data.tx_msg.data.res_data_write_regs.write_length_fi =
			WRITE_LENGTH_FI;
		if_data.tx_msg.data.res_data_write_regs.write_length = length;
		
		if_data.tx_state = IF_TX_STATE_ENCODE;
	}
	else if (IF_CMD_READ_REGS == if_data.rx_msg.command)
	{
		if_data.tx_msg.type = IF_MSG_RESULT;
		if_data.tx_msg.command = IF_CMD_READ_REGS;
		if_data.tx_msg.data.res_data_read_regs.read_length_fi =
			READ_LENGTH_FI;
		if_data.tx_msg.data.res_data_read_regs.read_data_fi =
			READ_DATA_FI;
		if_data.tx_msg.data.res_data_read_regs.read_length = length;
		
		if_data.tx_state = IF_TX_STATE_ENCODE;
	}
}
