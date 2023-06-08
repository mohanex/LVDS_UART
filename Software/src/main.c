#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xspi.h"

#define BYTE1				0 /* Byte 1 position */
#define BYTE2				1 /* Byte 2 position */
#define BYTE3				2 /* Byte 3 position */
#define BYTE4				3 /* Byte 4 position */
#define BYTE5				4 /* Byte 5 position */
#define BYTE6				5 /* Byte 5 position */
#define BYTE7				6 /* Byte 5 position */
#define BYTE8				7 /* Byte 5 position */

#define SPI_SELECT		0x01
#define SPI_DEVICE_ID		XPAR_SPI_0_DEVICE_ID
#define PAGE_SIZE		256

#define READ_WRITE_EXTRA_BYTES		4 /* Read/Write extra bytes */


u8 WriteBuffer[PAGE_SIZE + READ_WRITE_EXTRA_BYTES];
/*
 * Buffer used during Read transactions.
 */
u8 ReadBuffer[PAGE_SIZE + READ_WRITE_EXTRA_BYTES];


static XSpi Spi;
u8 FlashID[3];

int FlashReadID(void);

int main()
{
	init_platform();
	int Status;
	u8 *NULLPtr = NULL;
	xil_printf("Enable Writing on QSPI 0x06\n\r");

	/*
	 * Initialize the SPI driver so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h.
	 */
	Status = XSpi_Initialize(&Spi, SPI_DEVICE_ID);

	/* DEBUG ONLY SELF TEST SPI INTERFACE custom axel
	int maVariable = XSpi_SelfTest(&Spi);
	printf("\r\n Axel = %d \n\r", maVariable);
   */
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the SPI device as a master and in manual slave select mode such
	 * that the slave select signal does not toggle for every byte of a
	 * transfer, this must be done before the slave select is set.
	 */
	Status = XSpi_SetOptions(&Spi, XSP_MASTER_OPTION |
			     XSP_MANUAL_SSELECT_OPTION);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Select the flash device on the SPI bus, so that it can be
	 * read and written using the SPI bus.
	 */
	Status = XSpi_SetSlaveSelect(&Spi, SPI_SELECT);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the SPI driver so that interrupts and the device are enabled.
	 */
	XSpi_Start(&Spi);

	XSpi_IntrGlobalDisable(&Spi);

	Status = FlashReadID( );
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	u32 adresse_to_read_from = (u32)0x000002A8;

	xil_printf("Enable Writing on QSPI 0x06\n\r");
	WriteBuffer[BYTE1] = 0x06;
	Status = XSpi_Transfer(&Spi, WriteBuffer, NULLPtr, 1);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	//DEBUG ONLY SELF TEST SPI INTERFACE
	//int maVariable = XSpi_SelfTest(&Spi);
	//printf("\r\n Debugging = %d \n\r", maVariable);

    /*u8 BankRegReadCmd[1] = {0x16};
    u8 BankRegData[2] = {0x00, 0x00};
    XSpi_Transfer(&Spi, BankRegReadCmd, BankRegData, 2);
    xil_printf("SPI Flash Bank Register Data: %x %x\r\n", BankRegData[0], BankRegData[1]);*/

	xil_printf("Switch to 4byte adresse\n\r");
	WriteBuffer[BYTE1] = 0x17; //use 17h command for BRWR Bank register Write
	WriteBuffer[BYTE2] = 0x80; // use 80h to put EXTADD to 1 (1000 0000)b
	Status = XSpi_Transfer(&Spi, WriteBuffer, WriteBuffer, 2);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

    /*XSpi_Transfer(&Spi, BankRegReadCmd, BankRegData, 2);
    xil_printf("SPI Flash Bank Register Data after update: %x %x\r\n", BankRegData[0], BankRegData[1]);*/

	/*
	WriteBuffer[BYTE1] = 0x01; //use 01h command for Write registers WRR
	WriteBuffer[BYTE2] = 0x02; // use 02H FOR STATUS REGISTER page 52 for WEL = 1b
	WriteBuffer[BYTE3] = 0xC2; // use 02H FOR CONF REGISTER page 53 for LC=11b and QUAD=1b
	Status = XSpi_Transfer(&Spi, WriteBuffer, WriteBuffer, 3);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	*/
    u8 AdresseReadCmd[5] = {
    		0x13,
			(u8) (adresse_to_read_from >> 24),
			(u8) (adresse_to_read_from >> 16),
			(u8) (adresse_to_read_from >> 8),
			(u8) (adresse_to_read_from)
    };
    u8 AdresseReadData[20] = {0x00};


	xil_printf("Trying to read at address 0x%x\n\r", adresse_to_read_from);

	Status = XSpi_Transfer(&Spi, AdresseReadCmd, AdresseReadData, 20);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	int c=0;

	while(c!=20){
		xil_printf("ReadBuffer[%d] = 0x%x\n\r", c,AdresseReadData[c]);
		c++;
	}
	cleanup_platform();
	return 0;
}

int FlashReadID(void)
{
	//Modified code by AFASSI Mohamed
	int Status;
	int i;

	/* Read ID in Auto mode.*/
	WriteBuffer[BYTE1] = 0x90;
	WriteBuffer[BYTE2] = 0x00;		/* 3 dummy bytes */
	WriteBuffer[BYTE3] = 0x00;
	WriteBuffer[BYTE4] = 0x00;
	WriteBuffer[BYTE5] = 0xff;		/* 3 dummy reading bytes */
	WriteBuffer[BYTE6] = 0xff;

	Status = XSpi_Transfer(&Spi, WriteBuffer, ReadBuffer, 6);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	for(i = 0; i < 2; i++)
		FlashID[i] = ReadBuffer[i + 4];

	xil_printf("FlashID=0x%x 0x%x\n\r", ReadBuffer[BYTE5], ReadBuffer[BYTE6]);

	/*FOR S25FL256S
	if(ReadBuffer[BYTE5]==0x01&&ReadBuffer[BYTE6]==0x18){
		xil_printf("Spansion S25FL256S Flash detected\n\r");
		return XST_SUCCESS;
	}*/
	return XST_SUCCESS;
}
