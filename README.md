# Stellaris_SSD1306_OLED
TI Stellaris library for SSD1306 based OLED screens


This library is a ANSI C adaptation of the Adafruit-SSD1306-Library. This was done specifically to be used with TI's line of Stellaris Launchpad development boards.

# Usage

Declare in main.c:
`
#include <Stellaris_SSD1306.h>
#include <Adafruit_GFX.h>

tAdafruit_GFX sAdafruit_GFX;
tSSD1306 ssd1306;
// Pin setup
// D/C	PA6
// CS	PA3
// MOSI	PA5
// CLK	PA2
// VCC	3.3V

void ssd1306_display(tSSD1306 *psInstSSD);
void ssd1306_command(tSSD1306 *psInstSSD, uint8_t c);
void ssd1306_data(tSSD1306 *psInstSSD, uint8_t c);
void ssd1306_TopNRows(tSSD1306 *psInstSSD, uint8_t N);
`

Within main(void):
`
// Setup Graphics objects
Adafruit_GFX_init(&sAdafruit_GFX, SSD1306_LCDWIDTH, SSD1306_LCDHEIGHT, buffer);
setRotation(&sAdafruit_GFX, 0);
ssd1306_init(&ssd1306, &sAdafruit_GFX, SPI_DC_PIN_BASE, SPI_DC_PIN);
ssd1306.psInstGfx->textcolor = WHITE;
ssd1306.psInstGfx->textbgcolor = BLACK;
`

Define these functions (these are compatable with TIRTOS):
`
//------------------------------------------
// Send a command frame
//------------------------------------------
void ssd1306_command(tSSD1306 *psInstSSD, uint8_t c) {
    SPI_Transaction masterTransaction;
    bool transferOK;

	// Set DC LOW for command
	GPIOPinWrite(psInstSSD->dc_port_base, psInstSSD->dc_pin, 0);

    /* Initialize master SPI transaction structure */
    masterTransaction.count = 1;
    masterTransaction.txBuf = (Ptr)&c;
    masterTransaction.rxBuf = NULL;

    /* Initialize SPI handle as default master */
    masterSpi = SPI_open(Board_SPI0, &spiParams);
    if (masterSpi == NULL) {
    	Log_info0("Error initializing SPI");
    }
    else {
//    	Log_info0("SPI initialized");
    }

    /* Initiate SPI transfer */
    transferOK = SPI_transfer(masterSpi, &masterTransaction);

    if(transferOK) {
        /* Print contents of master receive buffer */
	}
    else {
//    	Log_info0("Unsuccessful master SPI transfer");
    }

    /* Deinitialize SPI */
    SPI_close(masterSpi);

}

//------------------------------------------
// Send a data frame
//------------------------------------------
void ssd1306_data(tSSD1306 *psInstSSD, uint8_t c) {
    SPI_Transaction masterTransaction;
    bool transferOK;

	// Set DC HIGH for data
	GPIOPinWrite(psInstSSD->dc_port_base, psInstSSD->dc_pin, psInstSSD->dc_pin);

    /* Initialize master SPI transaction structure */
    masterTransaction.count = 1;
    masterTransaction.txBuf = (Ptr)&c;
    masterTransaction.rxBuf = NULL;

    /* Initialize SPI handle as default master */
    masterSpi = SPI_open(Board_SPI0, &spiParams);
    if (masterSpi == NULL) {
    	Log_info0("Error initializing SPI");
    }
    else {
//    	Log_info0("SPI initialized");
    }

    /* Initiate SPI transfer */
    transferOK = SPI_transfer(masterSpi, &masterTransaction);

    if(transferOK) {
        /* Print contents of master receive buffer */
    }
    else {
//    	Log_info0("Unsuccessful master SPI transfer");
    }

    /* Deinitialize SPI */
    SPI_close(masterSpi);

}

//------------------------------------------
// Update the entire display
//------------------------------------------
void ssd1306_display(tSSD1306 *psInstSSD) {
    SPI_Transaction masterTransaction;
    bool transferOK;

    // Set DC LOW for command
	GPIOPinWrite(psInstSSD->dc_port_base, psInstSSD->dc_pin, 0);

	ssd1306_command(psInstSSD, SSD1306_COLUMNADDR);
	ssd1306_command(psInstSSD, 0);   // Column start address (0 = reset)
	ssd1306_command(psInstSSD, SSD1306_LCDWIDTH - 1); // Column end address (127 = reset)

	ssd1306_command(psInstSSD, SSD1306_PAGEADDR);
	ssd1306_command(psInstSSD, 0); // Page start address (0 = reset)
#if SSD1306_LCDHEIGHT == 64
	ssd1306_command(psInstSSD, 7); // Page end address
#endif
#if SSD1306_LCDHEIGHT == 32
	ssd1306_command(psInstSSD, 3); // Page end address
#endif
#if SSD1306_LCDHEIGHT == 16
	ssd1306_command(psInstSSD, 1); // Page end address
#endif


//	uint16_t n=0; // this does not work
//	for (n=0; n<SPI_MSG_LENGTH; n++){
//		ssd1306_data(psInstSSD, psInstSSD->psInstGfx->buffer[n]);
//	}

	// Set DC HIGH for data
	GPIOPinWrite(psInstSSD->dc_port_base, psInstSSD->dc_pin, psInstSSD->dc_pin);
	uint16_t n = 0;
	for (n = 0; n < SPI_MSG_LENGTH; n++) {

		/* Initialize master SPI transaction structure */
		masterTransaction.count = 1;
		masterTransaction.txBuf = (Ptr) (psInstSSD->psInstGfx->buffer + n);
		masterTransaction.rxBuf = NULL;

		/* Initialize SPI handle as default master */
		masterSpi = SPI_open(Board_SPI0, &spiParams);
		if (masterSpi == NULL) {
//			Log_info0("Error initializing SPI");
		}

		/* Initiate SPI transfer */
		transferOK = SPI_transfer(masterSpi, &masterTransaction);

		if (!transferOK) {
//			Log_info0("Unsuccessful master SPI transfer");
		}

		/* Deinitialize SPI */
		SPI_close(masterSpi);
	}

    // Set DC LOW for command
	GPIOPinWrite(psInstSSD->dc_port_base, psInstSSD->dc_pin, 0);
}
`

Or for non TIRTOS:

`
#include <driverlib/gpio.h>
#include <driverlib/rom.h>
#include <driverlib/sysctl.h>
#include <driverlib/interrupt.h>
#include <driverlib/pin_map.h>
#include <driverlib/ssi.h>

void ssd1306_command(tSSD1306 *psInstSSD, uint8_t c) {
	// Set DC LOW for command and DC HIGH for data
	ROM_GPIOPinWrite(psInstSSD->gpio_port_base, psInstSSD->dc, 0);

	// Wait until any SPI device is inactive
	while (ROM_SSIBusy(psInstSSD->ssi_base)) {
	}
	// Disable the SSI interface
	ROM_SSIDisable(psInstSSD->ssi_base);
	// Fill the Transmit FIFO
	ROM_SSIDataPutNonBlocking(psInstSSD->ssi_base, c);
	//Enable SSI
	ROM_SSIEnable(psInstSSD->ssi_base);
	while (ROM_SSIBusy(psInstSSD->ssi_base)) {
	}
	ROM_SSIDisable(psInstSSD->ssi_base);
}

void ssd1306_data(tSSD1306 *psInstSSD, uint8_t c) {
	// Set DC LOW for command and DC HIGH for data
	ROM_GPIOPinWrite(psInstSSD->gpio_port_base, psInstSSD->dc, psInstSSD->dc);

	while (ROM_SSIBusy(psInstSSD->ssi_base)) {
	}
	// Disable the SSI interface
	ROM_SSIDisable(psInstSSD->ssi_base);
	// Fill the Transmit FIFO
	ROM_SSIDataPutNonBlocking(psInstSSD->ssi_base, c);
	//Enable SSI
	ROM_SSIEnable(psInstSSD->ssi_base);
	while (ROM_SSIBusy(psInstSSD->ssi_base)) {
	}
	ROM_SSIDisable(psInstSSD->ssi_base);
}

void display(tSSD1306 *psInstSSD) {
	ssd1306_command(psInstSSD, SSD1306_COLUMNADDR);
	ssd1306_command(psInstSSD, 0);   // Column start address (0 = reset)
	ssd1306_command(psInstSSD, SSD1306_LCDWIDTH - 1); // Column end address (127 = reset)

	ssd1306_command(psInstSSD, SSD1306_PAGEADDR);
	ssd1306_command(psInstSSD, 0); // Page start address (0 = reset)
#if SSD1306_LCDHEIGHT == 64
	ssd1306_command(psInstSSD, 7); // Page end address
#endif
#if SSD1306_LCDHEIGHT == 32
	ssd1306_command(psInstSSD, 3); // Page end address
#endif
#if SSD1306_LCDHEIGHT == 16
	ssd1306_command(psInstSSD, 1); // Page end address
#endif

	// Set DC LOW for command and DC HIGH for data
	ROM_GPIOPinWrite(psInstSSD->gpio_port_base, psInstSSD->dc, psInstSSD->dc);
	// SPI
	uint16_t i;
	for (i = 0; i < (SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8); i++) {
		// Wait until any SPI device is inactive
		while (ROM_SSIBusy(psInstSSD->ssi_base)) {
		}
		ROM_SSIDisable(psInstSSD->ssi_base);
		ROM_SSIDataPutNonBlocking(psInstSSD->ssi_base, psInstSSD->psInstGfx->buffer[i]);
		ROM_SSIEnable(psInstSSD->ssi_base);
	}
	// Wait until any SPI device is inactive
	while (ROM_SSIBusy(psInstSSD->ssi_base)) {
	}
	ROM_SSIDisable(psInstSSD->ssi_base);
}

`


