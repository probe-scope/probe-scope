/*******************************************************************************
  GPIO PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_gpio.h

  Summary:
    GPIO PLIB Header File

  Description:
    This library provides an interface to control and interact with Parallel
    Input/Output controller (GPIO) module.

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
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

#ifndef PLIB_GPIO_H
#define PLIB_GPIO_H

#include <device.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data types and constants
// *****************************************************************************
// *****************************************************************************

/*** Macros for FPP_D5 pin ***/
#define FPP_D5_Set()               (LATESET = (1<<5))
#define FPP_D5_Clear()             (LATECLR = (1<<5))
#define FPP_D5_Toggle()            (LATEINV= (1<<5))
#define FPP_D5_Get()               ((PORTE >> 5) & 0x1)
#define FPP_D5_OutputEnable()      (TRISECLR = (1<<5))
#define FPP_D5_InputEnable()       (TRISESET = (1<<5))
#define FPP_D5_PIN                  GPIO_PIN_RE5
/*** Macros for FPP_D6 pin ***/
#define FPP_D6_Set()               (LATESET = (1<<6))
#define FPP_D6_Clear()             (LATECLR = (1<<6))
#define FPP_D6_Toggle()            (LATEINV= (1<<6))
#define FPP_D6_Get()               ((PORTE >> 6) & 0x1)
#define FPP_D6_OutputEnable()      (TRISECLR = (1<<6))
#define FPP_D6_InputEnable()       (TRISESET = (1<<6))
#define FPP_D6_PIN                  GPIO_PIN_RE6
/*** Macros for FPP_D7 pin ***/
#define FPP_D7_Set()               (LATESET = (1<<7))
#define FPP_D7_Clear()             (LATECLR = (1<<7))
#define FPP_D7_Toggle()            (LATEINV= (1<<7))
#define FPP_D7_Get()               ((PORTE >> 7) & 0x1)
#define FPP_D7_OutputEnable()      (TRISECLR = (1<<7))
#define FPP_D7_InputEnable()       (TRISESET = (1<<7))
#define FPP_D7_PIN                  GPIO_PIN_RE7
/*** Macros for FPIO3 pin ***/
#define FPIO3_Set()               (LATGSET = (1<<6))
#define FPIO3_Clear()             (LATGCLR = (1<<6))
#define FPIO3_Toggle()            (LATGINV= (1<<6))
#define FPIO3_Get()               ((PORTG >> 6) & 0x1)
#define FPIO3_OutputEnable()      (TRISGCLR = (1<<6))
#define FPIO3_InputEnable()       (TRISGSET = (1<<6))
#define FPIO3_PIN                  GPIO_PIN_RG6
/*** Macros for FPIO4 pin ***/
#define FPIO4_Set()               (LATGSET = (1<<9))
#define FPIO4_Clear()             (LATGCLR = (1<<9))
#define FPIO4_Toggle()            (LATGINV= (1<<9))
#define FPIO4_Get()               ((PORTG >> 9) & 0x1)
#define FPIO4_OutputEnable()      (TRISGCLR = (1<<9))
#define FPIO4_InputEnable()       (TRISGSET = (1<<9))
#define FPIO4_PIN                  GPIO_PIN_RG9
/*** Macros for EXIO0 pin ***/
#define EXIO0_Set()               (LATBSET = (1<<5))
#define EXIO0_Clear()             (LATBCLR = (1<<5))
#define EXIO0_Toggle()            (LATBINV= (1<<5))
#define EXIO0_Get()               ((PORTB >> 5) & 0x1)
#define EXIO0_OutputEnable()      (TRISBCLR = (1<<5))
#define EXIO0_InputEnable()       (TRISBSET = (1<<5))
#define EXIO0_PIN                  GPIO_PIN_RB5
/*** Macros for FPGA_PROGRAMn pin ***/
#define FPGA_PROGRAMn_Set()               (LATBSET = (1<<4))
#define FPGA_PROGRAMn_Clear()             (LATBCLR = (1<<4))
#define FPGA_PROGRAMn_Toggle()            (LATBINV= (1<<4))
#define FPGA_PROGRAMn_Get()               ((PORTB >> 4) & 0x1)
#define FPGA_PROGRAMn_OutputEnable()      (TRISBCLR = (1<<4))
#define FPGA_PROGRAMn_InputEnable()       (TRISBSET = (1<<4))
#define FPGA_PROGRAMn_PIN                  GPIO_PIN_RB4
/*** Macros for FPGA_DONE pin ***/
#define FPGA_DONE_Set()               (LATBSET = (1<<3))
#define FPGA_DONE_Clear()             (LATBCLR = (1<<3))
#define FPGA_DONE_Toggle()            (LATBINV= (1<<3))
#define FPGA_DONE_Get()               ((PORTB >> 3) & 0x1)
#define FPGA_DONE_OutputEnable()      (TRISBCLR = (1<<3))
#define FPGA_DONE_InputEnable()       (TRISBSET = (1<<3))
#define FPGA_DONE_PIN                  GPIO_PIN_RB3
/*** Macros for FPGA_INITn pin ***/
#define FPGA_INITn_Set()               (LATBSET = (1<<2))
#define FPGA_INITn_Clear()             (LATBCLR = (1<<2))
#define FPGA_INITn_Toggle()            (LATBINV= (1<<2))
#define FPGA_INITn_Get()               ((PORTB >> 2) & 0x1)
#define FPGA_INITn_OutputEnable()      (TRISBCLR = (1<<2))
#define FPGA_INITn_InputEnable()       (TRISBSET = (1<<2))
#define FPGA_INITn_PIN                  GPIO_PIN_RB2
/*** Macros for EXIO1 pin ***/
#define EXIO1_Set()               (LATBSET = (1<<6))
#define EXIO1_Clear()             (LATBCLR = (1<<6))
#define EXIO1_Toggle()            (LATBINV= (1<<6))
#define EXIO1_Get()               ((PORTB >> 6) & 0x1)
#define EXIO1_OutputEnable()      (TRISBCLR = (1<<6))
#define EXIO1_InputEnable()       (TRISBSET = (1<<6))
#define EXIO1_PIN                  GPIO_PIN_RB6
/*** Macros for EXIO2 pin ***/
#define EXIO2_Set()               (LATBSET = (1<<9))
#define EXIO2_Clear()             (LATBCLR = (1<<9))
#define EXIO2_Toggle()            (LATBINV= (1<<9))
#define EXIO2_Get()               ((PORTB >> 9) & 0x1)
#define EXIO2_OutputEnable()      (TRISBCLR = (1<<9))
#define EXIO2_InputEnable()       (TRISBSET = (1<<9))
#define EXIO2_PIN                  GPIO_PIN_RB9
/*** Macros for PIC_SW pin ***/
#define PIC_SW_Set()               (LATBSET = (1<<10))
#define PIC_SW_Clear()             (LATBCLR = (1<<10))
#define PIC_SW_Toggle()            (LATBINV= (1<<10))
#define PIC_SW_Get()               ((PORTB >> 10) & 0x1)
#define PIC_SW_OutputEnable()      (TRISBCLR = (1<<10))
#define PIC_SW_InputEnable()       (TRISBSET = (1<<10))
#define PIC_SW_PIN                  GPIO_PIN_RB10
/*** Macros for FPIO0 pin ***/
#define FPIO0_Set()               (LATBSET = (1<<11))
#define FPIO0_Clear()             (LATBCLR = (1<<11))
#define FPIO0_Toggle()            (LATBINV= (1<<11))
#define FPIO0_Get()               ((PORTB >> 11) & 0x1)
#define FPIO0_OutputEnable()      (TRISBCLR = (1<<11))
#define FPIO0_InputEnable()       (TRISBSET = (1<<11))
#define FPIO0_PIN                  GPIO_PIN_RB11
/*** Macros for FPIO1 pin ***/
#define FPIO1_Set()               (LATBSET = (1<<12))
#define FPIO1_Clear()             (LATBCLR = (1<<12))
#define FPIO1_Toggle()            (LATBINV= (1<<12))
#define FPIO1_Get()               ((PORTB >> 12) & 0x1)
#define FPIO1_OutputEnable()      (TRISBCLR = (1<<12))
#define FPIO1_InputEnable()       (TRISBSET = (1<<12))
#define FPIO1_PIN                  GPIO_PIN_RB12
/*** Macros for EXIO3 pin ***/
#define EXIO3_Set()               (LATBSET = (1<<13))
#define EXIO3_Clear()             (LATBCLR = (1<<13))
#define EXIO3_Toggle()            (LATBINV= (1<<13))
#define EXIO3_Get()               ((PORTB >> 13) & 0x1)
#define EXIO3_OutputEnable()      (TRISBCLR = (1<<13))
#define EXIO3_InputEnable()       (TRISBSET = (1<<13))
#define EXIO3_PIN                  GPIO_PIN_RB13
/*** Macros for DAC_RDY pin ***/
#define DAC_RDY_Set()               (LATBSET = (1<<15))
#define DAC_RDY_Clear()             (LATBCLR = (1<<15))
#define DAC_RDY_Toggle()            (LATBINV= (1<<15))
#define DAC_RDY_Get()               ((PORTB >> 15) & 0x1)
#define DAC_RDY_OutputEnable()      (TRISBCLR = (1<<15))
#define DAC_RDY_InputEnable()       (TRISBSET = (1<<15))
#define DAC_RDY_PIN                  GPIO_PIN_RB15
/*** Macros for LED2 pin ***/
#define LED2_Set()               (LATCSET = (1<<15))
#define LED2_Clear()             (LATCCLR = (1<<15))
#define LED2_Toggle()            (LATCINV= (1<<15))
#define LED2_Get()               ((PORTC >> 15) & 0x1)
#define LED2_OutputEnable()      (TRISCCLR = (1<<15))
#define LED2_InputEnable()       (TRISCSET = (1<<15))
#define LED2_PIN                  GPIO_PIN_RC15
/*** Macros for FPIO2 pin ***/
#define FPIO2_Set()               (LATFSET = (1<<4))
#define FPIO2_Clear()             (LATFCLR = (1<<4))
#define FPIO2_Toggle()            (LATFINV= (1<<4))
#define FPIO2_Get()               ((PORTF >> 4) & 0x1)
#define FPIO2_OutputEnable()      (TRISFCLR = (1<<4))
#define FPIO2_InputEnable()       (TRISFSET = (1<<4))
#define FPIO2_PIN                  GPIO_PIN_RF4
/*** Macros for FPP_DREQ pin ***/
#define FPP_DREQ_Set()               (LATDSET = (1<<9))
#define FPP_DREQ_Clear()             (LATDCLR = (1<<9))
#define FPP_DREQ_Toggle()            (LATDINV= (1<<9))
#define FPP_DREQ_Get()               ((PORTD >> 9) & 0x1)
#define FPP_DREQ_OutputEnable()      (TRISDCLR = (1<<9))
#define FPP_DREQ_InputEnable()       (TRISDSET = (1<<9))
#define FPP_DREQ_PIN                  GPIO_PIN_RD9
/*** Macros for FPP_DRDY pin ***/
#define FPP_DRDY_Set()               (LATDSET = (1<<11))
#define FPP_DRDY_Clear()             (LATDCLR = (1<<11))
#define FPP_DRDY_Toggle()            (LATDINV= (1<<11))
#define FPP_DRDY_Get()               ((PORTD >> 11) & 0x1)
#define FPP_DRDY_OutputEnable()      (TRISDCLR = (1<<11))
#define FPP_DRDY_InputEnable()       (TRISDSET = (1<<11))
#define FPP_DRDY_PIN                  GPIO_PIN_RD11
/*** Macros for LED0 pin ***/
#define LED0_Set()               (LATCSET = (1<<13))
#define LED0_Clear()             (LATCCLR = (1<<13))
#define LED0_Toggle()            (LATCINV= (1<<13))
#define LED0_Get()               ((PORTC >> 13) & 0x1)
#define LED0_OutputEnable()      (TRISCCLR = (1<<13))
#define LED0_InputEnable()       (TRISCSET = (1<<13))
#define LED0_PIN                  GPIO_PIN_RC13
/*** Macros for LED1 pin ***/
#define LED1_Set()               (LATCSET = (1<<14))
#define LED1_Clear()             (LATCCLR = (1<<14))
#define LED1_Toggle()            (LATCINV= (1<<14))
#define LED1_Get()               ((PORTC >> 14) & 0x1)
#define LED1_OutputEnable()      (TRISCCLR = (1<<14))
#define LED1_InputEnable()       (TRISCSET = (1<<14))
#define LED1_PIN                  GPIO_PIN_RC14
/*** Macros for LDACn pin ***/
#define LDACn_Set()               (LATDSET = (1<<2))
#define LDACn_Clear()             (LATDCLR = (1<<2))
#define LDACn_Toggle()            (LATDINV= (1<<2))
#define LDACn_Get()               ((PORTD >> 2) & 0x1)
#define LDACn_OutputEnable()      (TRISDCLR = (1<<2))
#define LDACn_InputEnable()       (TRISDSET = (1<<2))
#define LDACn_PIN                  GPIO_PIN_RD2
/*** Macros for FPP_NC0 pin ***/
#define FPP_NC0_Set()               (LATDSET = (1<<4))
#define FPP_NC0_Clear()             (LATDCLR = (1<<4))
#define FPP_NC0_Toggle()            (LATDINV= (1<<4))
#define FPP_NC0_Get()               ((PORTD >> 4) & 0x1)
#define FPP_NC0_OutputEnable()      (TRISDCLR = (1<<4))
#define FPP_NC0_InputEnable()       (TRISDSET = (1<<4))
#define FPP_NC0_PIN                  GPIO_PIN_RD4
/*** Macros for FPP_NC1 pin ***/
#define FPP_NC1_Set()               (LATDSET = (1<<5))
#define FPP_NC1_Clear()             (LATDCLR = (1<<5))
#define FPP_NC1_Toggle()            (LATDINV= (1<<5))
#define FPP_NC1_Get()               ((PORTD >> 5) & 0x1)
#define FPP_NC1_OutputEnable()      (TRISDCLR = (1<<5))
#define FPP_NC1_InputEnable()       (TRISDSET = (1<<5))
#define FPP_NC1_PIN                  GPIO_PIN_RD5
/*** Macros for FPP_D0 pin ***/
#define FPP_D0_Set()               (LATESET = (1<<0))
#define FPP_D0_Clear()             (LATECLR = (1<<0))
#define FPP_D0_Toggle()            (LATEINV= (1<<0))
#define FPP_D0_Get()               ((PORTE >> 0) & 0x1)
#define FPP_D0_OutputEnable()      (TRISECLR = (1<<0))
#define FPP_D0_InputEnable()       (TRISESET = (1<<0))
#define FPP_D0_PIN                  GPIO_PIN_RE0
/*** Macros for FPP_D1 pin ***/
#define FPP_D1_Set()               (LATESET = (1<<1))
#define FPP_D1_Clear()             (LATECLR = (1<<1))
#define FPP_D1_Toggle()            (LATEINV= (1<<1))
#define FPP_D1_Get()               ((PORTE >> 1) & 0x1)
#define FPP_D1_OutputEnable()      (TRISECLR = (1<<1))
#define FPP_D1_InputEnable()       (TRISESET = (1<<1))
#define FPP_D1_PIN                  GPIO_PIN_RE1
/*** Macros for FPP_D2 pin ***/
#define FPP_D2_Set()               (LATESET = (1<<2))
#define FPP_D2_Clear()             (LATECLR = (1<<2))
#define FPP_D2_Toggle()            (LATEINV= (1<<2))
#define FPP_D2_Get()               ((PORTE >> 2) & 0x1)
#define FPP_D2_OutputEnable()      (TRISECLR = (1<<2))
#define FPP_D2_InputEnable()       (TRISESET = (1<<2))
#define FPP_D2_PIN                  GPIO_PIN_RE2
/*** Macros for FPP_D3 pin ***/
#define FPP_D3_Set()               (LATESET = (1<<3))
#define FPP_D3_Clear()             (LATECLR = (1<<3))
#define FPP_D3_Toggle()            (LATEINV= (1<<3))
#define FPP_D3_Get()               ((PORTE >> 3) & 0x1)
#define FPP_D3_OutputEnable()      (TRISECLR = (1<<3))
#define FPP_D3_InputEnable()       (TRISESET = (1<<3))
#define FPP_D3_PIN                  GPIO_PIN_RE3
/*** Macros for FPP_D4 pin ***/
#define FPP_D4_Set()               (LATESET = (1<<4))
#define FPP_D4_Clear()             (LATECLR = (1<<4))
#define FPP_D4_Toggle()            (LATEINV= (1<<4))
#define FPP_D4_Get()               ((PORTE >> 4) & 0x1)
#define FPP_D4_OutputEnable()      (TRISECLR = (1<<4))
#define FPP_D4_InputEnable()       (TRISESET = (1<<4))
#define FPP_D4_PIN                  GPIO_PIN_RE4


// *****************************************************************************
/* GPIO Port

  Summary:
    Identifies the available GPIO Ports.

  Description:
    This enumeration identifies the available GPIO Ports.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all ports are available on all devices.  Refer to the specific
    device data sheet to determine which ports are supported.
*/

typedef enum
{
    GPIO_PORT_B = 0,
    GPIO_PORT_C = 1,
    GPIO_PORT_D = 2,
    GPIO_PORT_E = 3,
    GPIO_PORT_F = 4,
    GPIO_PORT_G = 5,
} GPIO_PORT;

// *****************************************************************************
/* GPIO Port Pins

  Summary:
    Identifies the available GPIO port pins.

  Description:
    This enumeration identifies the available GPIO port pins.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all pins are available on all devices.  Refer to the specific
    device data sheet to determine which pins are supported.
*/

typedef enum
{
    GPIO_PIN_RB0 = 0,
    GPIO_PIN_RB1 = 1,
    GPIO_PIN_RB2 = 2,
    GPIO_PIN_RB3 = 3,
    GPIO_PIN_RB4 = 4,
    GPIO_PIN_RB5 = 5,
    GPIO_PIN_RB6 = 6,
    GPIO_PIN_RB7 = 7,
    GPIO_PIN_RB8 = 8,
    GPIO_PIN_RB9 = 9,
    GPIO_PIN_RB10 = 10,
    GPIO_PIN_RB11 = 11,
    GPIO_PIN_RB12 = 12,
    GPIO_PIN_RB13 = 13,
    GPIO_PIN_RB14 = 14,
    GPIO_PIN_RB15 = 15,
    GPIO_PIN_RC12 = 28,
    GPIO_PIN_RC13 = 29,
    GPIO_PIN_RC14 = 30,
    GPIO_PIN_RC15 = 31,
    GPIO_PIN_RD0 = 32,
    GPIO_PIN_RD1 = 33,
    GPIO_PIN_RD2 = 34,
    GPIO_PIN_RD3 = 35,
    GPIO_PIN_RD4 = 36,
    GPIO_PIN_RD5 = 37,
    GPIO_PIN_RD9 = 41,
    GPIO_PIN_RD10 = 42,
    GPIO_PIN_RD11 = 43,
    GPIO_PIN_RE0 = 48,
    GPIO_PIN_RE1 = 49,
    GPIO_PIN_RE2 = 50,
    GPIO_PIN_RE3 = 51,
    GPIO_PIN_RE4 = 52,
    GPIO_PIN_RE5 = 53,
    GPIO_PIN_RE6 = 54,
    GPIO_PIN_RE7 = 55,
    GPIO_PIN_RF0 = 64,
    GPIO_PIN_RF1 = 65,
    GPIO_PIN_RF3 = 67,
    GPIO_PIN_RF4 = 68,
    GPIO_PIN_RF5 = 69,
    GPIO_PIN_RG6 = 86,
    GPIO_PIN_RG7 = 87,
    GPIO_PIN_RG8 = 88,
    GPIO_PIN_RG9 = 89,

    /* This element should not be used in any of the GPIO APIs.
       It will be used by other modules or application to denote that none of the GPIO Pin is used */
    GPIO_PIN_NONE = -1

} GPIO_PIN;


void GPIO_Initialize(void);

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on multiple pins of a port
// *****************************************************************************
// *****************************************************************************

uint32_t GPIO_PortRead(GPIO_PORT port);

void GPIO_PortWrite(GPIO_PORT port, uint32_t mask, uint32_t value);

uint32_t GPIO_PortLatchRead ( GPIO_PORT port );

void GPIO_PortSet(GPIO_PORT port, uint32_t mask);

void GPIO_PortClear(GPIO_PORT port, uint32_t mask);

void GPIO_PortToggle(GPIO_PORT port, uint32_t mask);

void GPIO_PortInputEnable(GPIO_PORT port, uint32_t mask);

void GPIO_PortOutputEnable(GPIO_PORT port, uint32_t mask);

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on one pin at a time
// *****************************************************************************
// *****************************************************************************

static inline void GPIO_PinWrite(GPIO_PIN pin, bool value)
{
    GPIO_PortWrite(pin>>4, (uint32_t)(0x1) << (pin & 0xF), (uint32_t)(value) << (pin & 0xF));
}

static inline bool GPIO_PinRead(GPIO_PIN pin)
{
    return (bool)(((GPIO_PortRead(pin>>4)) >> (pin & 0xF)) & 0x1);
}

static inline bool GPIO_PinLatchRead(GPIO_PIN pin)
{
    return (bool)((GPIO_PortLatchRead(pin>>4) >> (pin & 0xF)) & 0x1);
}

static inline void GPIO_PinToggle(GPIO_PIN pin)
{
    GPIO_PortToggle(pin>>4, 0x1 << (pin & 0xF));
}

static inline void GPIO_PinSet(GPIO_PIN pin)
{
    GPIO_PortSet(pin>>4, 0x1 << (pin & 0xF));
}

static inline void GPIO_PinClear(GPIO_PIN pin)
{
    GPIO_PortClear(pin>>4, 0x1 << (pin & 0xF));
}

static inline void GPIO_PinInputEnable(GPIO_PIN pin)
{
    GPIO_PortInputEnable(pin>>4, 0x1 << (pin & 0xF));
}

static inline void GPIO_PinOutputEnable(GPIO_PIN pin)
{
    GPIO_PortOutputEnable(pin>>4, 0x1 << (pin & 0xF));
}


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END
#endif // PLIB_GPIO_H
