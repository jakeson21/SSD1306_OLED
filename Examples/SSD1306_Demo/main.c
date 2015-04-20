/*
 * main.c
 */

//------------------------------------------
// TivaWare Header Files
//------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_nvic.h"

#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/udma.h"
#include "driverlib/pin_map.h"
#include "driverlib/fpu.h"
#include "driverlib/rom.h"

#include <Stellaris_SSD1306.h>
#include <Adafruit_GFX.h>

#define ARDUINO 200
#define STRBUFLEN 30
#define SPI_DC_PIN_BASE GPIO_PORTA_BASE
#define SPI_DC_PIN		GPIO_PIN_6
extern uint8_t buffer[];
char strbuffer[STRBUFLEN];
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

const char *fmtS = "S=%8d";
const char *fmtA = "A=%8d";
const char *fmtB = "B=%8d";
const char *fmtadc = "a=%8d";


void hardware_init(void);

int main(void) {
	hardware_init();
    ssd1306_begin(&ssd1306, SSD1306_SWITCHCAPVCC, NULL, NULL);
    ssd1306_display(&ssd1306);
    clearDisplay(&ssd1306);
    ssd1306_display(&ssd1306);

    uint32_t count = 0;

    while(1){
		clearDisplay(&ssd1306);

		// Update Display
		uint8_t sz;
		memset(strbuffer, 0, STRBUFLEN);
		sz = sprintf(strbuffer, fmtS, count++);
		setCursor(ssd1306.psInstGfx, 0, 0);
		Print(ssd1306.psInstGfx, strbuffer);

		memset(strbuffer, 0, STRBUFLEN);
		sz = sprintf(strbuffer, fmtA, count++);
		setCursor(ssd1306.psInstGfx, 11*6, 0);
		Print(ssd1306.psInstGfx, strbuffer);

		memset(strbuffer, 0, STRBUFLEN);
		sz = sprintf(strbuffer, fmtB, count++);
		setCursor(ssd1306.psInstGfx, 11*6, 8);
		Print(ssd1306.psInstGfx, strbuffer);

		memset(strbuffer, 0, STRBUFLEN);
		sz = sprintf(strbuffer, fmtadc, count++);
		setCursor(ssd1306.psInstGfx, 0, 8);
		Print(ssd1306.psInstGfx, strbuffer);

		ssd1306_TopNRows(&ssd1306, 2);
    }

	//return 0;
}


//---------------------------------------------------------------------------
// hardware_init()
//
// inits GPIO pins for toggling the LED
//---------------------------------------------------------------------------
void hardware_init(void)
{
    //
    // Enable lazy stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    FPUEnable();
    FPUStackingEnable();

	//Set CPU Clock to 80MHz. 400MHz PLL/2 = 200 DIV 2.5 = 80MHz
	SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	// ADD Tiva-C GPIO setup - enables port, sets pins 1-3 (RGB) pins for output
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

	// Turn off the LED
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);

	// Enable SSI PORT Pins
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	//
	// Enable pin PA5 for SSI0 SSI0TX  = MOSI
	// Enable pin PA4 for SSI0 SSI0RX  = MISO
	// Enable pin PA3 for SSI0 SSI0FSS = CSN
	// Enable pin PA2 for SSI0 SSI0CLK = SCK
	//  For Freescale SPI and MICROWIRE frame formats, the serial frame (SSInFss) pin is active Low,
	//  and is asserted (pulled down) during the entire transmission of the frame.
	//
	GPIOPinConfigure(GPIO_PA4_SSI0RX | GPIO_PA3_SSI0FSS | GPIO_PA2_SSI0CLK | GPIO_PA5_SSI0TX);
	GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_2);

	//
	// Set PA6 as RF24 CE pin (D/C)
	//
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_6);

	while (SSIBusy(SSI0_BASE)) {}
	SSIDisable(SSI0_BASE);
	SSIConfigSetExpClk(SSI0_BASE,
		SysCtlClockGet(),
		SSI_FRF_MOTO_MODE_0,
		SSI_MODE_MASTER, 1000000, 8);
	SSIEnable(SSI0_BASE);
	while (SSIBusy(SSI0_BASE)) {}
	SysCtlDelay(2000000);

	// Setup Graphics objects
	Adafruit_GFX_init(&sAdafruit_GFX, SSD1306_LCDWIDTH, SSD1306_LCDHEIGHT, buffer);
	setRotation(&sAdafruit_GFX, 0);
	ssd1306_init(&ssd1306, &sAdafruit_GFX, SPI_DC_PIN_BASE, SPI_DC_PIN);
	ssd1306.psInstGfx->textcolor = WHITE;
	ssd1306.psInstGfx->textbgcolor = BLACK;

	// Turn off the LED
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);

}



//------------------------------------------
// Send a command frame
//------------------------------------------
void ssd1306_command(tSSD1306 *psInstSSD, uint8_t data) {
	while(SSIBusy(SSI0_BASE)){}
	// Set DC LOW for command
	GPIOPinWrite(psInstSSD->dc_port_base, psInstSSD->dc_pin, 0);
	SSIDataPutNonBlocking(SSI0_BASE, data);
}

//------------------------------------------
// Send a data frame
//------------------------------------------
void ssd1306_data(tSSD1306 *psInstSSD, uint8_t data) {
	while(SSIBusy(SSI0_BASE)){}
	// Set DC HIGH for data
	GPIOPinWrite(psInstSSD->dc_port_base, psInstSSD->dc_pin, psInstSSD->dc_pin);
	SSIDataPutNonBlocking(SSI0_BASE, data);
}

//------------------------------------------
// Update the entire display
//------------------------------------------
void ssd1306_display(tSSD1306 *psInstSSD) {
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

	uint16_t n=0;
	for (n=0; n<SPI_MSG_LENGTH; n++){
		ssd1306_data(psInstSSD, psInstSSD->psInstGfx->buffer[n]);
	}
}


//------------------------------------------
// Update the top N rows only
//------------------------------------------
void ssd1306_TopNRows(tSSD1306 *psInstSSD, uint8_t N)
{
    // Set DC LOW for command
	GPIOPinWrite(psInstSSD->dc_port_base, psInstSSD->dc_pin, 0);

	ssd1306_command(psInstSSD, SSD1306_COLUMNADDR);
	ssd1306_command(psInstSSD, 0);   // Column start address (0 = reset)
	ssd1306_command(psInstSSD, SSD1306_LCDWIDTH - 1); // Column end address (127 = reset)

	ssd1306_command(psInstSSD, SSD1306_PAGEADDR);
	ssd1306_command(psInstSSD, 0); // Page start address (0 = reset)
	ssd1306_command(psInstSSD, N-1); // Page end address - Top 2 rows

	// Set DC HIGH for data
	GPIOPinWrite(psInstSSD->dc_port_base, psInstSSD->dc_pin, psInstSSD->dc_pin);
	uint16_t n = 0;
	for (n = 0; n < psInstSSD->psInstGfx->WIDTH*N; n++) {
		ssd1306_data(psInstSSD, psInstSSD->psInstGfx->buffer[n]);
	}
}
