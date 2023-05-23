/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "blconfig.h"
#include "portab.h"
#include "errors.h"
#include "srec.h"
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "PmodGPIO.h"
#include "xspi.h"
#include "xuartlite.h"
#include "xbram.h"
#include "xclk_wiz.h"

#define GPIO_REG_TRI		0x04
#define GPIO_REG_DATA		0x00
#define GPIO_CHANNEL        1
#define GPIO_INTERRUPT_PIN  1

#define UARTlite_DEVICE_ID XPAR_AXI_UARTLITE_0_DEVICE_ID
#define QSPI_DEVICE_ID	   XPAR_SPI_0_DEVICE_ID
#define GPIO_DEVICE_ID     XPAR_PMODGPIO_0_DEVICE_ID


#define BRAM_MATRIX_BASEADDR       0x000FFCD1
#define QSPI_FLASH_SIZE            65536  // Size of QSPI flash in bytes (64KB)
#define BUFFER_SIZE                32      // Buffer size for reading data
#define BRAM_SIZE                  1048576     // Size of BRAM in bytes (1MB)

u8 qspi_read_buffer[BUFFER_SIZE];
u8 bram_buffer[BRAM_SIZE];


//IPs Instanciations
PmodGPIO *GPIO_input;

XSpi_Stats *QSPI_stats;
XSpi_Config *QSPI_config;
XSpi *QSPI;

XUartLite_Config *UART_config;
XUartLite_Buffer *UART_Buffer;
XUartLite_Stats *UART_Stats;
XUartLite *UART;

XBram *XBRAM;
XBram_Config *XBRAM_config;


XClk_Wiz *CLK_wiz;
XClk_Wiz_Config *CLK_wiz_config;









int main()
{
    init_platform();
    
    cleanup_platform();
    return 0;
}

int init_qspi() {
    int status;

    QSPI_config = XSpi_LookupConfig(QSPI_DEVICE_ID);
    if (QSPI_config == NULL) {
        return XST_FAILURE;
    }

    status = XSpi_CfgInitialize(&QSPI, QSPI_config, QSPI_config->BaseAddress);
    if (status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    status = XSpi_SetOptions(&QSPI, XSP_MASTER_OPTION | XSP_MANUAL_SSELECT_OPTION);
	if(status != XST_SUCCESS) {
		return XST_FAILURE;
	}

    status = XSpi_Start(&QSPI);
    if (status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    XSpi_IntrGlobalDisable(&QSPI);

    return XST_SUCCESS;
}

int init_uart() {
    int status;

    status = XUartLite_Initialize(&UART, UARTlite_DEVICE_ID);
    if (status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

int init_gpio() {
    int status;

    status = XGpio_Initialize(&gpio, GPIO_DEVICE_ID);
    if (status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    // Set GPIO channel direction
    XGpio_SetDataDirection(&gpio, GPIO_CHANNEL, 0xFFFFFFFF);

    // Enable interrupt for GPIO channel
    XGpio_InterruptEnable(&gpio, XGPIO_IR_CH1_MASK);
    XGpio_InterruptGlobalEnable(&gpio);

    // Register interrupt handler
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XGpio_InterruptHandler, &gpio);
    Xil_ExceptionEnable();

    return XST_SUCCESS;
}

void read_qspi_flash(u32 start_address, u32 size) {
    u32 bytes_read = 0;
    u32 bytes_to_read;
    int status;

    while (bytes_read < size) {
        bytes_to_read = size - bytes_read;
        if (bytes_to_read > BUFFER_SIZE) {
            bytes_to_read = BUFFER_SIZE;
        }

        status = XSpi_Transfer(&QSPI, qspi_read_buffer, NULL, bytes_to_read);
        if (status != XST_SUCCESS) {
            xil_printf("QSPI read failed!\r\n");
            break;
        }

        for (int i = 0; i < bytes_to_read; i++) {
            XUartLite_SendByte(XPAR_AXI_UARTLITE_0_BASEADDR, qspi_read_buffer[i]);
        }

        bytes_read += bytes_to_read;
    }
}