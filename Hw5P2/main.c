//*****************************************************************************
//
// uart_echo.c - Example for reading data from and writing data to the UART in
//               an interrupt driven fashion.
//
// Copyright (c) 2013-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.4.178 of the DK-TM4C129X Firmware Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "grlib/grlib.h"
#include "drivers/kentec320x240x16_ssd2119.h"
#include "drivers/frame.h"
#include "drivers/pinout.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>UART Echo (uart_echo)</h1>
//!
//! This example application utilizes the UART to echo text.  The first UART
//! (connected to the FTDI virtual serial port on the evaluation board) will be
//! configured in 115,200 baud, 8-n-1 mode.  All characters received on the
//! UART are transmitted back to the UART.
//
//*****************************************************************************

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif


/**
 * @brief Contains lookup table for converting a byte from integer to ASCII
 *
 * @param num Number to convert to ASCII (one byte)
 *
 * @return ASCII character representation of byte
 */

uint8_t ascii_lut(int32_t num)
{
  switch(num)  {
    case 0:
      return '0';
    case 1:
      return '1';
    case 2:
      return '2';
    case 3:
      return '3';
    case 4:
      return '4';
    case 5:
      return '5';
    case 6:
      return '6';
    case 7:
      return '7';
    case 8:
      return '8';
    case 9:
      return '9';
    case 10:
      return 'A';
    case 11:
      return 'B';
    case 12:
      return 'C';
    case 13:
      return 'D';
    case 14:
      return 'E';
    case 15:
      return 'F';
    }

  return 'X'; // if digit falls out of range, return obviously wrong value
}

/**
 * @brief Converts from integer to ASCII, including numerical base conversion
 *
 * @param data Integer number to convert to ASCII
 * @param ptr Pointer to memory location containing converted ASCII string
 * @param base Number between [2, 16] indicating numerical base to convert to
 *
 * @return Length of converted ASCII array, in bytes
 */

uint8_t my_itoa(int32_t data, uint8_t * ptr, uint32_t base)
{
  uint8_t length = 0; // store length of converted ASCII String
  uint8_t tmp = 0;    // store current digit in temp location

  if(base < 2 || base > 16)  // only bases in range [2,16] supported
    return 255;

  while(data != 0)  {  // perform repeated divides by base to perform conversion
    *(ptr+length) = ascii_lut(data % base);
    data /= base;
    length++;  // count length of ASCII string (number of digits)
  }

  uint8_t i;
  for(i=0; i<(length/2); i++) { // reverse ASCII string to correct
    tmp = *(ptr+i);                     // digit order
    *(ptr+i) = *(ptr+length-i-1);
    *(ptr+length-i-1) = tmp;
  }

  *(ptr+length) = '\0';   // terminate with null character

  return length;  // return length of ASCII string
}

//*****************************************************************************
//
// The UART interrupt handler.
//
//*****************************************************************************
void
UARTIntHandler(void)
{
    uint32_t ui32Status;

    //
    // Get the interrrupt status.
    //
    ui32Status = ROM_UARTIntStatus(UART0_BASE, true);

    //
    // Clear the asserted interrupts.
    //
    ROM_UARTIntClear(UART0_BASE, ui32Status);

    //
    // Loop while there are characters in the receive FIFO.
    //
    while(ROM_UARTCharsAvail(UART0_BASE))
    {
        //
        // Read the next character from the UART and write it back to the UART.
        //
        ROM_UARTCharPutNonBlocking(UART0_BASE,
                                   UARTCharGetNonBlocking(UART0_BASE));
    }
}

//*****************************************************************************
//
// Send a string to the UART.
//
//*****************************************************************************
void
UARTSend(const uint8_t *pui8Buffer, uint32_t ui32Count)
{
    //
    // Loop while there are more characters to send.
    //
    while(ui32Count--)
    {
        //
        // Write the next character to the UART.
        //
        ROM_UARTCharPutNonBlocking(UART0_BASE, *pui8Buffer++);
    }
}

//*****************************************************************************
//
// This example demonstrates how to send a string of data to the UART.
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32SysClock;

    //
    // Run from the PLL at 120 MHz.
    //
    ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                           SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
                                           SYSCTL_CFG_VCO_480), 120000000);

    //
    // Enable the GPIO port that is used for the on-board LED.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    //
    // Check if the peripheral access is enabled.
    //
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION))
    {
    }

    //
    // Enable the GPIO pin for the LED (PN0).  Set the direction as output, and
    // enable the GPIO pin for digital function.
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);
    //
    // Enable the (non-GPIO) peripherals used by this example.  PinoutSet()
    // already enabled GPIO Port A.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Enable processor interrupts.
    //
    IntMasterEnable();

    //
    // Configure the UART for 115,200, 8-N-1 operation.
    //
    ROM_UARTConfigSetExpClk(UART0_BASE, ui32SysClock, 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));

    //
    // Enable the UART interrupt.
    //
    ROM_IntEnable(INT_UART0);
    ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);

    //gotta split it into 3 because apparently the
    //uart buffer is slow and the fifo is small
    UARTSend((uint8_t *)"Project for: Zach ", 19);
    ROM_SysCtlDelay(60000);
    UARTSend((uint8_t *)" Farmer 4/4/2018", 18);
    ROM_SysCtlDelay(60000);
    UARTSend((uint8_t *)"\n\r", 2);
    //
    // Loop forever echoing data through the UART.
    //

    //~2Hz loop
    int32_t counter=0;
    uint8_t val[12];
    while(1)
    {
        counter++;
        //LED on
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
        ROM_SysCtlDelay(10000000); //approx 0.25s delay
        //LED off
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x0);
        ROM_SysCtlDelay(10000000); //approx 0.25s delay
        my_itoa(counter, val, 10);
        UARTSend(val, 12);
        UARTSend((uint8_t *)"\n\r", 2);
    }
}
