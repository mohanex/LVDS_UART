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

#define SPI_SELECT		0x01
#define SPI_DEVICE_ID		XPAR_SPI_0_DEVICE_ID
/*
 * Number of bytes per page in the flash device.
 */
#define PAGE_SIZE		256

/*
 * Byte Positions.
 */
#define BYTE1				0 /* Byte 1 position */
#define BYTE2				1 /* Byte 2 position */
#define BYTE3				2 /* Byte 3 position */
#define BYTE4				3 /* Byte 4 position */
#define BYTE5				4 /* Byte 5 position */

#define READ_WRITE_EXTRA_BYTES		4 /* Read/Write extra bytes */
#define	READ_WRITE_EXTRA_BYTES_4BYTE_MODE	5 /**< Command extra bytes */

#define RD_ID_SIZE					4

#define ISSI_ID_BYTE0			0x9D
#define MICRON_ID_BYTE0			0x20

#define ENTER_4B_ADDR_MODE		0xb7 /* Enter 4Byte Mode command */
#define EXIT_4B_ADDR_MODE		0xe9 /* Exit 4Byte Mode command */
#define EXIT_4B_ADDR_MODE_ISSI	0x29
#define	WRITE_ENABLE			0x06 /* Write Enable command */

#define ENTER_4B	1
#define EXIT_4B		0

#define	FLASH_16_MB	0x18
#define FLASH_MAKE		0
#define	FLASH_SIZE		2

#define	READ_CMD	0x03



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
    //gpio initialization and tristate set
    GPIO_begin(&GPIO_input,XPAR_PMODGPIO_0_AXI_LITE_GPIO_BASEADDR,1);
    //QSPI initialization 
    XSpi_CfgInitialize(&QSPI,&QSPI_config,XPAR_AXI_QUAD_SPI_0_BASEADDR);
    //UART initialization
    XUartLite_CfgInitialize(&UART,&UART_config,XPAR_AXI_UARTLITE_0_BASEADDR);
    //XBRAM Initialization
    XBram_CfgInitialize(&XBRAM,&XBRAM_config,XPAR_MICROBLAZE_0_LOCAL_MEMORY_DLMB_BRAM_IF_CNTLR_BASEADDR);
    //Clk_wiz Initialization
    XClk_Wiz_CfgInitialize(&CLK_wiz,&CLK_wiz_config,XPAR_CLK_WIZ_0_BASEADDR);

    init_platform();

    
    cleanup_platform();
    return 0;
}
