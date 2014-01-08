/*****************************************************************************
*
* File: spi1.c
* 
* Copyright S. Brennen Ball, 2006-2007
* 
* The author provides no guarantees, warantees, or promises, implied or
*	otherwise.  By using this software you agree to indemnify the author
* 	of any damages incurred by using it.
* 
*****************************************************************************/

#include <spi.h>
#include <p24fj64ga004.h>

//JJ: changed this to use SPI2 for nordic / ballon xmit

unsigned char spi1_send_read_byte(unsigned char byte)
{
	//SSPBUF = byte;
	SPI2BUF = byte & 0xff; 
	while(SPI2STATbits.SPITBF);
//	SPI2BUF=0;
	while(!SPI2STATbits.SPIRBF);

	SPI2STATbits.SPIROV = 0;
	return (SPI2BUF); 
}	
