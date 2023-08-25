#include <stdio.h>
#include <string.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "uart_axifull.h"
#include "xil_io.h"
#include "xspi.h"
#include "sleep.h"
#include "xgpio.h"

#define BYTE1				0 /* Byte 1 position */
#define BYTE2				1 /* Byte 2 position */
#define BYTE3				2 /* Byte 3 position */
#define BYTE4				3 /* Byte 4 position */
#define BYTE5				4 /* Byte 5 position */
#define BYTE6				5 /* Byte 5 position */
#define BYTE7				6 /* Byte 5 position */
#define BYTE8				7 /* Byte 5 position */

#define SPI_DEVICE_ID	XPAR_SPI_0_DEVICE_ID	//ID De l'IP généré par vivado dans xparameters


int QUAD_Configured = 1;

u8 WriteBuffer[256];
u8 ReadBuffer[256];
u8 FlashID[10];

u8 File_In[25600];
u8 AdresseReadCmd[5]={0};

XSpi Spi;

int FlashReadID(void);
int ReadFromFLASH(int mode, u32 addr);
void shiftAndDelete(uint8_t* data, size_t size, size_t shiftAmount, size_t deleteCount);

int main()
{
    init_platform();
    int Status; /*Status variable to save the return values of SPI configurations*/

    u32 i,loop,j;
    u32 adresse_to_read = 0x0000e000; /*Adreese where the combined file is saved in the QSPI*/

    /**
    * Initialize the SPI instance provided by the caller based on the
    * given DeviceID.
    *
    * Nothing is done except to initialize the InstancePtr.
    */
    Status = XSpi_Initialize(&Spi, SPI_DEVICE_ID);
    if(Status != XST_SUCCESS) {
    	return XST_FAILURE;
    }

    	/*
    	 * Set the SPI device as a master and in manual slave select mode such
    	 * that the slave select signal does not toggle for every byte of a
    	 * transfer, this must be done before the slave select is set.
    	 */
    Status = XSpi_SetOptions(&Spi, XSP_MASTER_OPTION | XSP_MANUAL_SSELECT_OPTION);
    if(Status != XST_SUCCESS) {
    	return XST_FAILURE;
    }

    	/*
    	 * Select the flash device on the SPI bus, so that it can be
    	 * read and written using the SPI bus.
    	 */
    Status = XSpi_SetSlaveSelect(&Spi, 0x01);
    if(Status != XST_SUCCESS) {
    	return XST_FAILURE;
    }

    	/*
    	 * Start the SPI driver so that interrupts and the device are enabled.
    	 */
    XSpi_Start(&Spi);
    XSpi_IntrGlobalDisable(&Spi);

    /*
     * Read flash ID to make sure the FLASH is detected
     */
    Status = FlashReadID();
    if(Status != XST_SUCCESS) {
    	return XST_FAILURE;
    }

	u32 new_adr = adresse_to_read + (0x00006400*loop);
	Status = ReadFromFLASH(4, new_adr);
	if(Status != XST_SUCCESS) {
			return XST_FAILURE;
	}
		
    //}
    cleanup_platform();
    return 0;
}

int FlashReadID(void)
{
	int Status,i;

	/* Read ID in Auto mode.*/
	WriteBuffer[BYTE1] = 0x9F;
	WriteBuffer[BYTE2] = 0x00;		/* 3 dummy bytes for reading Manufacturer identification and device identification*/
	WriteBuffer[BYTE3] = 0x00;
	WriteBuffer[BYTE4] = 0x00;

	Status = XSpi_Transfer(&Spi, WriteBuffer, ReadBuffer, 6);/*Send reading command*/
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*Extracting the ID from the Readbuffer variable*/
	/*En effet pour controler la QSPI il faut toujours donner une variable de lecture et pendant que
	 * la QSPI lis les commandes qu'on lui envoi, elle envoi aussi des données au hasard.
	 * La QSPI est programmé d'une sorte d'être obligée de lire et écrire en même temps
	 * */
	for(i = 0; i < 3; i++)
		FlashID[i] = ReadBuffer[i + 1];

	if(FlashID[BYTE1]==0xC2||ReadBuffer[BYTE2]==0x20||ReadBuffer[BYTE3]==0x16){ /*Check if ID is the same as the one described in the datasheet*/
		return XST_SUCCESS;
	}
	else return XST_FAILURE;
}
	/*	Function to read from the FLASH QSPI
	 * 	This function handles different mode of reading
	 * 	Simplereading, Doublereading, Quadreading
	 *
	 * */
int ReadFromFLASH(int mode, u32 addr){
	u8 WRITE_ENABLE[2] = {0x06,0x00};/*Command to enable memory conf writing*/
	u8 conf_QUAD[3] ={   /*Command to activate Quad Read Mode*/
			0x01,
			0x43,
			0x00
	};
	int Status,I_mode = mode;
	u32 adresse_to_read_from = addr;

	switch(I_mode){
	case 1: ; //SimpleREADING
    	u8 SimpleRead[5] = {
    			0x0B,
				(u8) (adresse_to_read_from >> 16),
				(u8) (adresse_to_read_from >> 8),
				(u8) (adresse_to_read_from),
				0x00
    	};
    	Status = XSpi_Transfer(&Spi, SimpleRead, File_In, 25605);
    	shiftAndDelete(File_In,sizeof(File_In),5,5);
    	if (Status != XST_SUCCESS) {
    		return XST_FAILURE;
    	}
		break;
	case 2: ; //DoubleREADING
		if (QUAD_Configured){
			Status = XSpi_Transfer(&Spi, WRITE_ENABLE, File_In, 1);
			QUAD_Configured = 0;
		}
		Status = XSpi_Transfer(&Spi, conf_QUAD, File_In, 3);/*Configure Status and Config register on QUAD MODE*/
    	u8 DoubleRead[12]={
    			0x3B,
				(u8) (adresse_to_read_from >> 16),
				(u8) (adresse_to_read_from >> 8),
				(u8) (adresse_to_read_from),
				0x00,0x00,0x00,    /*8 DUMMY Cycles*/
				0x00,0x00,0x00,
				0x00,0x00
    	};
    	Status = XSpi_Transfer(&Spi, DoubleRead, File_In, 25612);
    	shiftAndDelete(File_In,sizeof(File_In),12,12);
    	if (Status != XST_SUCCESS) {
    		return XST_FAILURE;
    	}
			break;
	case 4: ; //QuadREADING
		if (QUAD_Configured){
			Status = XSpi_Transfer(&Spi, WRITE_ENABLE, File_In, 1);
			QUAD_Configured = 0;
		}
		Status = XSpi_Transfer(&Spi, conf_QUAD, File_In, 3);/*Configure Status and Config register on QUAD MODE*/
    	u8 QuadRead[12]={
    			0x6B,
				(u8) (adresse_to_read_from >> 16),
				(u8) (adresse_to_read_from >> 8),
				(u8) (adresse_to_read_from),
				0x00,0x00,0x00,    /*8 DUMMY Cycles*/
				0x00,0x00,0x00,
				0x00,0x00
    	};
    	Status = XSpi_Transfer(&Spi, QuadRead, File_In, 25612);
    	shiftAndDelete(File_In,sizeof(File_In),12,12);
    	if (Status != XST_SUCCESS) {
    		return XST_FAILURE;
    	}
			break;
	}
	return XST_SUCCESS;
}
	/*
	 * shift and Delete function is
	 * a function to delete dummy bytes received and shift the remaining ones
	 * */
void shiftAndDelete(uint8_t* data, size_t size, size_t shiftAmount, size_t deleteCount) {
    // Shift the data by shiftAmount
    for (size_t i = shiftAmount; i < size; ++i) {
        data[i - shiftAmount] = data[i];
    }

    // Reduce the size of the array by deleteCount
    size -= deleteCount;

    // Optional: Clear the remaining elements after reducing the size
    for (size_t i = size; i < size + deleteCount; ++i) {
        data[i] = 0;  // Set deleted elements to zero or any desired value
    }
}
