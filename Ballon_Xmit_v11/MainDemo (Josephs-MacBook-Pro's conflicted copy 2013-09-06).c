/*********************************************************************
 *
 *  Main Application Entry Point and TCP/IP Stack Demo
 *  Module for Microchip TCP/IP Stack
 *   -Demonstrates how to call and use the Microchip TCP/IP stack
 *	 -Reference: Microchip TCP/IP Stack Help (TCPIP Stack Help.chm)
 *
 *********************************************************************
 * FileName:        MainDemo.c
 * Dependencies:    TCPIP.h
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.11b or higher
 *					Microchip C30 v3.24 or higher
 *					Microchip C18 v3.36 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2010 Microchip Technology Inc.  All rights
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and
 * distribute:
 * (i)  the Software when embedded on a Microchip microcontroller or
 *      digital signal controller product ("Device") which is
 *      integrated into Licensee's product; or
 * (ii) ONLY the Software driver source files ENC28J60.c, ENC28J60.h,
 *		ENCX24J600.c and ENCX24J600.h ported to a non-Microchip device
 *		used in conjunction with a Microchip ethernet controller for
 *		the sole purpose of interfacing with the ethernet controller.
 *
 * You should refer to the license agreement accompanying this
 * Software for additional information regarding your rights and
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 *
 * Author              Date         Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Nilesh Rajbharti		4/19/01		Original (Rev. 1.0)
 * Nilesh Rajbharti		2/09/02		Cleanup
 * Nilesh Rajbharti		5/22/02		Rev 2.0 (See version.log for detail)
 * Nilesh Rajbharti		7/9/02		Rev 2.1 (See version.log for detail)
 * Nilesh Rajbharti		4/7/03		Rev 2.11.01 (See version log for detail)
 * Howard Schlunder		10/1/04		Beta Rev 0.9 (See version log for detail)
 * Howard Schlunder		10/8/04		Beta Rev 0.9.1 Announce support added
 * Howard Schlunder		11/29/04	Beta Rev 0.9.2 (See version log for detail)
 * Howard Schlunder		2/10/05		Rev 2.5.0
 * Howard Schlunder		1/5/06		Rev 3.00
 * Howard Schlunder		1/18/06		Rev 3.01 ENC28J60 fixes to TCP, 
 *									UDP and ENC28J60 files
 * Howard Schlunder		3/01/06		Rev. 3.16 including 16-bit micro support
 * Howard Schlunder		4/12/06		Rev. 3.50 added LCD for Explorer 16
 * Howard Schlunder		6/19/06		Rev. 3.60 finished dsPIC30F support, added PICDEM.net 2 support
 * Howard Schlunder		8/02/06		Rev. 3.75 added beta DNS, NBNS, and HTTP client (GenericTCPClient.c) services
 * Howard Schlunder		12/28/06	Rev. 4.00RC added SMTP, Telnet, substantially modified TCP layer
 * Howard Schlunder		04/09/07	Rev. 4.02 added TCPPerformanceTest, UDPPerformanceTest, Reboot and fixed some bugs
 * Howard Schlunder		xx/xx/07	Rev. 4.03
 * HSchlunder & EWood	08/27/07	Rev. 4.11
 * HSchlunder & EWood	10/08/07	Rev. 4.13
 * HSchlunder & EWood	11/06/07	Rev. 4.16
 * HSchlunder & EWood	11/08/07	Rev. 4.17
 * HSchlunder & EWood	11/12/07	Rev. 4.18
 * HSchlunder & EWood	02/11/08	Rev. 4.19
 * HSchlunder & EWood   04/26/08    Rev. 4.50 Moved most code to other files for clarity
 * KHesky               07/07/08    Added MRF24W-specific support
 * SGustafson           01/30/09    Added LinkMgrII, LinkLib, Console, and iperf.
 ********************************************************************/
/*
 * This macro uniquely defines this file as the main entry point.
 * There should only be one such definition in the entire project,
 * and this file must define the AppConfig variable as described below.
 */
#define THIS_IS_STACK_APPLICATION

// Include all headers for any enabled TCPIP Stack functions
#include "TCPIP Stack/TCPIP.h"

#if defined(STACK_USE_ZEROCONF_LINK_LOCAL)
#include "TCPIP Stack/ZeroconfLinkLocal.h"
#endif
#if defined(STACK_USE_ZEROCONF_MDNS_SD)
#include "TCPIP Stack/ZeroconfMulticastDNS.h"
#endif

// Include functions specific to this stack application
#include "MainDemo.h"
#include "nrf24l01.h"
// Used for Wi-Fi assertions
#define WF_MODULE_NUMBER   WF_MODULE_MAIN_DEMO

unsigned char my_board_addr=0;
unsigned char xmit_frame_addr=0;
unsigned char transmit_now=0;
unsigned long gBytesToXmit=0;

unsigned char BALLOON_raw_pack[150];
unsigned char new_balloon_pack=0;

unsigned char rf_xmit_data[33];

unsigned char prev_channel=0;
unsigned char prev_datarate=0;
unsigned char xmit_frame[1000];

#define OSC_PACK_SIZE 144
//must match ballon_osc.c


// Declare AppConfig structure and some other supporting stack variables
APP_CONFIG AppConfig;
static unsigned short wOriginalAppConfigChecksum;	// Checksum of the ROM defaults for AppConfig
BYTE AN0String[8];

// Use UART2 instead of UART1 for stdout (printf functions).  Explorer 16 
// serial port hardware is on PIC UART2 module.
#if defined(EXPLORER_16) || defined(PIC24FJ256DA210_DEV_BOARD)
	int __C30_UART = 2;
#endif


// Private helper functions.
// These may or may not be present in all applications.
static void InitAppConfig(void);
static void InitializeBoard(void);

#if defined(WF_CS_TRIS)
    static void WF_Connect(void);
#endif

//
// PIC18 Interrupt Service Routines
// 
// NOTE: Several PICs, including the PIC18F4620 revision A3 have a RETFIE FAST/MOVFF bug
// The interruptlow keyword is used to work around the bug when using C18
#if defined(__18CXX)
	#if defined(HI_TECH_C)
	void interrupt low_priority LowISR(void)
	#else
	#pragma interruptlow LowISR
	void LowISR(void)
	#endif
	{
	    TickUpdate();
	}
	
	#if defined(HI_TECH_C)
	void interrupt HighISR(void)
	#else
	#pragma interruptlow HighISR
	void HighISR(void)
	#endif
	{
	    #if defined(STACK_USE_UART2TCP_BRIDGE)
			UART2TCPBridgeISR();
		#endif

		#if defined(WF_CS_TRIS)
			WFEintISR();
		#endif // WF_CS_TRIS
	}

	#if !defined(HI_TECH_C)
	#pragma code lowVector=0x18
	void LowVector(void){_asm goto LowISR _endasm}
	#pragma code highVector=0x8
	void HighVector(void){_asm goto HighISR _endasm}
	#pragma code // Return to default code section
	#endif

// C30 and C32 Exception Handlers
// If your code gets here, you either tried to read or write
// a NULL pointer, or your application overflowed the stack
// by having too many local variables or parameters declared.
#elif defined(__C30__)

    #if defined(STACK_USE_UART)
      #define UART2PrintString    putrsUART
    #else
      #define UART2PrintString(x)
    #endif

    void __attribute__((interrupt, auto_psv)) _DefaultInterrupt(void)
    {
      UART2PrintString( "!!! Default interrupt handler !!!\r\n" );
      while (1)
      {
          Nop();
          Nop();
          Nop();
      }
    }

    void __attribute__((interrupt, auto_psv)) _OscillatorFail(void)
    {
      UART2PrintString( "!!! Oscillator Fail interrupt handler !!!\r\n" );
      while (1)
      {
          Nop();
          Nop();
          Nop();
      }
    }
    void __attribute__((interrupt, auto_psv)) _AddressError(void)
    {
      UART2PrintString( "!!! Address Error interrupt handler !!!\r\n" );
      while (1)
      {
          Nop();
          Nop();
          Nop();
      }
    }
    void __attribute__((interrupt, auto_psv)) _StackError(void)
    {
      UART2PrintString( "!!! Stack Error interrupt handler !!!\r\n" );
      while (1)
      {
          Nop();
          Nop();
          Nop();
      }
    }
    void __attribute__((interrupt, auto_psv)) _MathError(void)
    {
      UART2PrintString( "!!! Math Error interrupt handler !!!\r\n" );
      while (1)
      {
          Nop();
          Nop();
          Nop();
      }
    }

#elif defined(__C32__)
	void _general_exception_handler(unsigned cause, unsigned status)
	{
		Nop();
		Nop();
	}
#endif

void LEDtest()
{

	LED1_IO=1;
	DelayMs(100);
	LED1_IO=0;
	DelayMs(100);
	LED0_IO=1;
	DelayMs(100);
	LED0_IO=0;
	DelayMs(100);
	
}

void InitializeSPI2()
{
	
	 IEC2bits.SPI2IE = 0;

     SPI2CON1 = 0x003A;      /* Initalizes the spi module 8:1 / 16:1 */
     SPI2CON2 = 0;
     SPI2STAT = 0x8000;     /* Enable/Disable the spi module */

}

void InitRFbuffer()
{
	unsigned char i;

	for(i=0;i<32;i++)
		rf_xmit_data[i]=i;
	
}

void test_xmit()
{

	unsigned char loop,i=0, int_data[35], outgoing[33];
	unsigned char config=0;
	unsigned char curr_datarate, curr_channel=0, pack=0;


		LED1_IO=1;

	//	while(pack<1)
	//		{
			//outgoing[0]=pack;
			for(i=1;i<32;i++)
				outgoing[i]=0x73;
			nrf24l01_write_tx_payload(outgoing,32,true);
			while(!(nrf24l01_irq_pin_active() && nrf24l01_irq_tx_ds_active()));
			nrf24l01_irq_clear_all(); //clear all interrupts in the 24L01	
		//	DelayMs(1);
		//	pack++;
	//		}

			LED1_IO=0;	

}

void nordic_XMIT()
{
	unsigned char loop,i=0, int_data[35], outgoing[33];
	unsigned char config=0;
	unsigned char curr_datarate, curr_channel=0;

		LED1_IO^=1;
	for(loop=7;loop<OSC_PACK_SIZE;loop=loop+4)		//this will strip out all the extra 0 bytes from the OSC long int padding
		{
			int_data[i]=BALLOON_raw_pack[loop];
			i++;
		}

	if(int_data[0]==0xff)
	{
		LED1_IO=1;
		/*	curr_datarate=(int_data[1]&0x60)>>5;
			curr_channel=int_data[1]&0x1f;
		
			if(curr_channel!=prev_channel)
				{
		 			nrf24l01_set_rf_ch(curr_channel);
					prev_channel=curr_channel;
					DelayMs(10);
				}
		
			if(curr_datarate!=prev_datarate)
				{
		
					prev_datarate=curr_datarate;
					switch(curr_datarate)
					{
					case 1:
						config=0b00000110;
						nrf24l01_write_register(nrf24l01_RF_SETUP, &config, 1);		//1mbps
						break;
					case 2:
						config=0b00001110;
						nrf24l01_write_register(nrf24l01_RF_SETUP, &config, 1);		//2mbps
						break;
					default:
						config=0b00100110;
						nrf24l01_write_register(nrf24l01_RF_SETUP, &config, 1);			//250kbps
						break;
					}	
					DelayMs(10);
				}
		*/	
			for(i=0;i<32;i++)
				outgoing[i]=int_data[i+2];
			
		
			nrf24l01_write_tx_payload(outgoing,32,1);
			while(!(nrf24l01_irq_pin_active() && nrf24l01_irq_tx_ds_active()));
		
			nrf24l01_irq_clear_all(); //clear all interrupts in the 24L01	
			LED1_IO=0;	
	}

}

//
// Main application entry point.
//
#if defined(__18CXX)
void main(void)
#else
int main(void)
#endif
{
	static DWORD t = 0;
	static DWORD dwLastIP = 0;

    #if defined (EZ_CONFIG_STORE)
    static DWORD ButtonPushStart = 0;
    #endif

	unsigned char config=0;
	unsigned char tx_addr[5];

	unsigned char reg_ptr[40];

	// Initialize application specific hardware
	InitializeBoard();

	// Initialize stack-related hardware components that may be 
	// required by the UART configuration routines
    TickInit();

	InitializeSPI2();

	LEDtest();

//	nrf24l01_initialize_debug(false, 32, false); //initialize the 24L01 to the debug configuration as TX, 1 data byte, and auto-ack disabled


	config = nrf24l01_CONFIG_DEFAULT_VAL | nrf24l01_CONFIG_PWR_UP;
	nrf24l01_initialize(config, 
						false,
						0, 
						nrf24l01_EN_RXADDR_DEFAULT_VAL, 
						nrf24l01_SETUP_AW_DEFAULT_VAL, 
						0, 
						20, 
						0x26,  //250kbps, full power  
						NULL, 
						NULL, 
						nrf24l01_RX_ADDR_P2_DEFAULT_VAL, 
						nrf24l01_RX_ADDR_P3_DEFAULT_VAL, 
						nrf24l01_RX_ADDR_P4_DEFAULT_VAL, 
						nrf24l01_RX_ADDR_P5_DEFAULT_VAL, 
						NULL, 
						32, 
						nrf24l01_RX_PW_P1_DEFAULT_VAL, 
						nrf24l01_RX_PW_P2_DEFAULT_VAL, 
						nrf24l01_RX_PW_P3_DEFAULT_VAL, 
						nrf24l01_RX_PW_P4_DEFAULT_VAL, 
						nrf24l01_RX_PW_P5_DEFAULT_VAL);


	InitRFbuffer();

	// Initialize Stack and application related NV variables into AppConfig.
	InitAppConfig();

	// Initialize core stack layers (MAC, ARP, TCP, UDP) and
	// application modules (HTTP, SNMP, etc.)
    StackInit();

    WF_Connect();

	mDNSInitialize(MY_DEFAULT_HOST_NAME);
	mDNSServiceRegister(
		(const char *) MY_DEFAULT_HOST_NAME,	// base name of the service
		"_lumigeek._udp.local",			    // type of the service
		4445,				                // TCP or UDP port, at which this service is available
		((const BYTE *)"Balloon v2.0"),	// TXT info
		1,								    // auto rename the service when if needed
		NULL,							    // no callback function
		NULL							    // no application context
		);

    mDNSMulticastFilterRegister();

	// Now that all items are initialized, begin the co-operative
	// multitasking loop.  This infinite loop will continuously 
	// execute all stack-related tasks, as well as your own
	// application's functions.  Custom functions should be added
	// at the end of this loop.
    // Note that this is a "co-operative mult-tasking" mechanism
    // where every task performs its tasks (whether all in one shot
    // or part of it) and returns so that other tasks can do their
    // job.
    // If a task needs very long time to do its job, it must be broken
    // down into smaller pieces so that other tasks can have CPU time.
    while(1)
    {     

        #if defined (STACK_USE_EZ_CONFIG)
        // Blink LED0 twice per sec when unconfigured, once per sec after config
        if((TickGet() - t >= TICK_SECOND/(4ul - (CFGCXT.isWifiDoneConfigure*2ul))))
        #else
        // Blink LED0 (right most one) every second.
        if(TickGet() - t >= TICK_SECOND/2ul)
        #endif // STACK_USE_EZ_CONFIG
        {
            t = TickGet();
		//	test_xmit();
		//	nrf24l01_get_all_registers((unsigned char*)&reg_ptr);
            LED0_IO ^= 1;
        }

		if(new_balloon_pack)
		{
			new_balloon_pack=0;
			nordic_XMIT();
		}

        // This task performs normal stack task including checking
        // for incoming packet, type of packet and calling
        // appropriate stack entity to process it.
        StackTask();

        // This tasks invokes each of the core stack application tasks
        StackApplications();

        #if defined(STACK_USE_ZEROCONF_MDNS_SD)
        mDNSProcess();
		// Use this function to exercise service update function
		// HTTPUpdateRecord();
        #endif


/*
        // If the local IP address has changed (ex: due to DHCP lease change)
        // write the new IP address to the LCD display, UART, and Announce 
        // service
		if(dwLastIP != AppConfig.MyIPAddr.Val)
		{
			dwLastIP = AppConfig.MyIPAddr.Val;
			
			#if defined(STACK_USE_ANNOUNCE)
				AnnounceIP();
			#endif

            #if defined(STACK_USE_ZEROCONF_MDNS_SD)
				mDNSFillHostRecord();
			#endif
		}
*/
	}
}

#if defined(WF_CS_TRIS)
/*****************************************************************************
 * FUNCTION: WF_Connect
 *
 * RETURNS:  None
 *
 * PARAMS:   None
 *
 *  NOTES:   Connects to an 802.11 network.  Customize this function as needed 
 *           for your application.
 *****************************************************************************/
static void WF_Connect(void)
{
    UINT8 ConnectionProfileID;
    UINT8 channelList[] = MY_DEFAULT_CHANNEL_LIST;
    #if defined(WF_USE_POWER_SAVE_FUNCTIONS)
    BOOL  PsPollEnabled;
    #endif
    
    /* create a Connection Profile */
    WF_CPCreate(&ConnectionProfileID);

    #if defined(STACK_USE_UART)
    putrsUART("Set SSID (");
    putsUART(AppConfig.MySSID);
    putrsUART(")\r\n");
    #endif
    WF_CPSetSsid(ConnectionProfileID, 
                 AppConfig.MySSID, 
                 AppConfig.SsidLength);

    #if defined(STACK_USE_UART)
    putrsUART("Set Network Type\r\n");
	#endif

//  jj: change this when you have hardware
 //   WF_CPSetNetworkType(ConnectionProfileID, AppConfig.networkType);
        WF_CPSetNetworkType(ConnectionProfileID, WF_ADHOC);

  //  if (AppConfig.networkType == WF_ADHOC)
  //  {
        WF_CPSetAdHocBehavior(ConnectionProfileID, WF_ADHOC_CONNECT_THEN_START);
   // }

    
    //putrsUART("Set Security\r\n");
    switch(AppConfig.SecurityMode) {
        case WF_SECURITY_OPEN:
            WF_CPSetSecurity(ConnectionProfileID, WF_SECURITY_OPEN, 0, NULL, 0);
            break;
        case WF_SECURITY_WEP_40:
            // assume key 0
            WF_CPSetSecurity(ConnectionProfileID, AppConfig.SecurityMode, 0, AppConfig.SecurityKey, 5);
            break;
        case WF_SECURITY_WEP_104:
            // assume key 0
            WF_CPSetSecurity(ConnectionProfileID, AppConfig.SecurityMode, 0, AppConfig.SecurityKey, 13);
            break;
        case WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE:
            WF_CPSetSecurity(ConnectionProfileID, WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE, 
                             0, AppConfig.SecurityKey, strlen((char*)AppConfig.SecurityKey));
            break;
        case WF_SECURITY_WPA_AUTO_WITH_KEY:
            WF_CPSetSecurity(ConnectionProfileID, WF_SECURITY_WPA_AUTO_WITH_KEY,
                             0, AppConfig.SecurityKey, 32);
            break;
        default:
			break;
			//putrsUART("\r\n\r\nCaptain this should NOT happen.\r\n\r\n");

    }
        
	#if defined(STACK_USE_UART)
	putrsUART("Set Scan Type\r\n");
	#endif
    WF_CASetScanType(MY_DEFAULT_SCAN_TYPE);
    
    #if defined(STACK_USE_UART)
    putrsUART("Set Channel List\r\n");
    #endif    
    WF_CASetChannelList(channelList, sizeof(channelList));
    
    #if defined(STACK_USE_UART)
    putrsUART("Set list retry count\r\n");
    #endif
    WF_CASetListRetryCount(MY_DEFAULT_LIST_RETRY_COUNT);

    #if defined(STACK_USE_UART)        
    putrsUART("Set Event Notify\r\n");    
    #endif
    WF_CASetEventNotificationAction(MY_DEFAULT_EVENT_NOTIFICATION_LIST);
    
#if defined(WF_USE_POWER_SAVE_FUNCTIONS)
    PsPollEnabled = (MY_DEFAULT_PS_POLL == WF_ENABLED);
    if (!PsPollEnabled)
    {    
        /* disable low power (PS-Poll) mode */
        #if defined(STACK_USE_UART)
        putrsUART("Disable PS-Poll\r\n");        
        #endif
        WF_PsPollDisable();
    }    
    else
    {
        /* Enable low power (PS-Poll) mode */
        #if defined(STACK_USE_UART)
        putrsUART("Enable PS-Poll\r\n");        
        #endif
        WF_PsPollEnable(TRUE);
    }    
#endif

    #if defined(STACK_USE_UART)
    putrsUART("Set Beacon Timeout\r\n");
    #endif
    WF_CASetBeaconTimeout(40);


    #if defined(STACK_USE_UART)                     
    putrsUART("Start WiFi Connect\r\n");        
    #endif
    WF_CMConnect(ConnectionProfileID);
}   
#endif /* WF_CS_TRIS */

// Writes an IP address to the LCD display and the UART as available
void DisplayIPValue(IP_ADDR IPVal)
{
//	printf("%u.%u.%u.%u", IPVal.v[0], IPVal.v[1], IPVal.v[2], IPVal.v[3]);
    BYTE IPDigit[4];
	BYTE i;
#ifdef USE_LCD
	BYTE j;
	BYTE LCDPos=16;
#endif

	for(i = 0; i < sizeof(IP_ADDR); i++)
	{
	    uitoa((WORD)IPVal.v[i], IPDigit);

		#if defined(STACK_USE_UART)
			putsUART((char *) IPDigit);
		#endif

		#ifdef USE_LCD
			for(j = 0; j < strlen((char*)IPDigit); j++)
			{
				LCDText[LCDPos++] = IPDigit[j];
			}
			if(i == sizeof(IP_ADDR)-1)
				break;
			LCDText[LCDPos++] = '.';
		#else
			if(i == sizeof(IP_ADDR)-1)
				break;
		#endif

		#if defined(STACK_USE_UART)
			while(BusyUART());
			WriteUART('.');
		#endif
	}

	#ifdef USE_LCD
		if(LCDPos < 32u)
			LCDText[LCDPos] = 0;
		LCDUpdate();
	#endif
}

/****************************************************************************
  Function:
    static void InitializeBoard(void)

  Description:
    This routine initializes the hardware.  It is a generic initialization
    routine for many of the Microchip development boards, using definitions
    in HardwareProfile.h to determine specific initialization.

  Precondition:
    None

  Parameters:
    None - None

  Returns:
    None

  Remarks:
    None
  ***************************************************************************/
static void InitializeBoard(void)
{	
	// LEDs
	LED0_TRIS = 0;
	LED1_TRIS = 0;
	LED_PUT(0x00);

	CLKDIVbits.RCDIV0=0; //clock divider to 0 **Improved over origional release**


/*
	#if defined(__dsPIC33F__) || defined(__PIC24H__)
		// Crank up the core frequency
		PLLFBD = 38;				// Multiply by 40 for 160MHz VCO output (8MHz XT oscillator)
		CLKDIV = 0x0000;			// FRC: divide by 2, PLLPOST: divide by 2, PLLPRE: divide by 2
	
		// Port I/O
		AD1PCFGHbits.PCFG23 = 1;	// Make RA7 (BUTTON1) a digital input
		AD1PCFGHbits.PCFG20 = 1;	// Make RA12 (INT1) a digital input for MRF24WB0M PICtail Plus interrupt

		// ADC
	    AD1CHS0 = 0;				// Input to AN0 (potentiometer)
		AD1PCFGLbits.PCFG5 = 0;		// Disable digital input on AN5 (potentiometer)
		AD1PCFGLbits.PCFG4 = 0;		// Disable digital input on AN4 (TC1047A temp sensor)
	#else	//defined(__PIC24F__) || defined(__PIC32MX__)
		#if defined(__PIC24F__)
			CLKDIVbits.RCDIV = 0;		// Set 1:1 8MHz FRC postscalar
		#endif
		
		// ADC
	    #if defined(__PIC24FJ256DA210__) || defined(__PIC24FJ256GB210__)
	    	// Disable analog on all pins
	    	ANSA = 0x0000;
	    	ANSB = 0x0000;
	    	ANSC = 0x0000;
	    	ANSD = 0x0000;
	    	ANSE = 0x0000;
	    	ANSF = 0x0000;
	    	ANSG = 0x0000;
		#else
		    AD1CHS = 0;					// Input to AN0 (potentiometer)
			AD1PCFGbits.PCFG4 = 0;		// Disable digital input on AN4 (TC1047A temp sensor)
			#if defined(__32MX460F512L__) || defined(__32MX795F512L__)	// PIC32MX460F512L and PIC32MX795F512L PIMs has different pinout to accomodate USB module
				AD1PCFGbits.PCFG2 = 0;		// Disable digital input on AN2 (potentiometer)
			#else
				AD1PCFGbits.PCFG5 = 0;		// Disable digital input on AN5 (potentiometer)
			#endif
		#endif
	#endif

	// ADC
	AD1CON1 = 0x84E4;			// Turn on, auto sample start, auto-convert, 12 bit mode (on parts with a 12bit A/D)
	AD1CON2 = 0x0404;			// AVdd, AVss, int every 2 conversions, MUXA only, scan
	AD1CON3 = 0x1003;			// 16 Tad auto-sample, Tad = 3*Tcy
	#if defined(__32MX460F512L__) || defined(__32MX795F512L__)	// PIC32MX460F512L and PIC32MX795F512L PIMs has different pinout to accomodate USB module
		AD1CSSL = 1<<2;				// Scan pot
	#else
		AD1CSSL = 1<<5;				// Scan pot
	#endif

	// UART
	#if defined(STACK_USE_UART)
		UARTTX_TRIS = 0;
		UARTRX_TRIS = 1;
		UMODE = 0x8000;			// Set UARTEN.  Note: this must be done before setting UTXEN

		#if defined(__C30__)
			USTA = 0x0400;		// UTXEN set
			#define CLOSEST_UBRG_VALUE ((GetPeripheralClock()+8ul*BAUD_RATE)/16/BAUD_RATE-1)
			#define BAUD_ACTUAL (GetPeripheralClock()/16/(CLOSEST_UBRG_VALUE+1))
		#else	//defined(__C32__)
			USTA = 0x00001400;		// RXEN set, TXEN set
			#define CLOSEST_UBRG_VALUE ((GetPeripheralClock()+8ul*BAUD_RATE)/16/BAUD_RATE-1)
			#define BAUD_ACTUAL (GetPeripheralClock()/16/(CLOSEST_UBRG_VALUE+1))
		#endif
	
		#define BAUD_ERROR ((BAUD_ACTUAL > BAUD_RATE) ? BAUD_ACTUAL-BAUD_RATE : BAUD_RATE-BAUD_ACTUAL)
		#define BAUD_ERROR_PRECENT	((BAUD_ERROR*100+BAUD_RATE/2)/BAUD_RATE)
		#if (BAUD_ERROR_PRECENT > 3)
			#warning UART frequency error is worse than 3%
		#elif (BAUD_ERROR_PRECENT > 2)
			#warning UART frequency error is worse than 2%
		#endif
	
		UBRG = CLOSEST_UBRG_VALUE;
	#endif

#endif
*/

	NRF_CSN_TRIS=0;
	NRF_CE_TRIS=0;
	NRF_SDI_TRIS=1;
	NRF_SCK_TRIS=0;
	NRF_SDO_TRIS=0;
	
	NRF_CSN_IO=1;

	NRF_INT_TRIS=1;


#if defined(WF_CS_TRIS)
	WF_CS_IO = 1;
	WF_CS_TRIS = 0;
#endif


	__builtin_write_OSCCONL(OSCCON & 0xBF);  // Unlock PPS
	
	// Inputs
//	RPINR19bits.U2RXR = 19;			//U2RX = RP19
	RPINR22bits.SDI2R = 3;			//SDI2 = RP3
	RPINR20bits.SDI1R = 22;			//SDI1 = RP22
	RPINR0bits.INT1R=23;
	
	// Outputs
//	RPOR12bits.RP25R = U2TX_IO;		//RP25 = U2TX  
	RPOR2bits.RP4R = SCK2OUT_IO; 	//RP4 = SCK2
	RPOR8bits.RP16R = SDO2_IO;		//RP16 = SDO2
	RPOR12bits.RP24R = SCK1OUT_IO; 	//RP24 = SCK1
	RPOR5bits.RP11R = SDO1_IO;		//RP11 = SDO1
	
	AD1PCFG = 0xFFFF;				//All digital inputs 

	__builtin_write_OSCCONL(OSCCON | 0x40); // Lock PPS


}

/*********************************************************************
 * Function:        void InitAppConfig(void)
 *
 * PreCondition:    MPFSInit() is already called.
 *
 * Input:           None
 *
 * Output:          Write/Read non-volatile config variables.
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
// MAC Address Serialization using a MPLAB PM3 Programmer and 
// Serialized Quick Turn Programming (SQTP). 
// The advantage of using SQTP for programming the MAC Address is it
// allows you to auto-increment the MAC address without recompiling 
// the code for each unit.  To use SQTP, the MAC address must be fixed
// at a specific location in program memory.  Uncomment these two pragmas
// that locate the MAC address at 0x1FFF0.  Syntax below is for MPLAB C 
// Compiler for PIC18 MCUs. Syntax will vary for other compilers.
//#pragma romdata MACROM=0x1FFF0
static ROM BYTE SerializedMACAddress[6] = {MY_DEFAULT_MAC_BYTE1, MY_DEFAULT_MAC_BYTE2, MY_DEFAULT_MAC_BYTE3, MY_DEFAULT_MAC_BYTE4, MY_DEFAULT_MAC_BYTE5, MY_DEFAULT_MAC_BYTE6};
//#pragma romdata

static void InitAppConfig(void)
{
#if defined(EEPROM_CS_TRIS) || defined(SPIFLASH_CS_TRIS)
	unsigned char vNeedToSaveDefaults = 0;
#endif
	
	while(1)
	{
		// Start out zeroing all AppConfig bytes to ensure all fields are 
		// deterministic for checksum generation
		memset((void*)&AppConfig, 0x00, sizeof(AppConfig));
		
		AppConfig.Flags.bIsDHCPEnabled = TRUE;
		AppConfig.Flags.bInConfigMode = TRUE;
		memcpypgm2ram((void*)&AppConfig.MyMACAddr, (ROM void*)SerializedMACAddress, sizeof(AppConfig.MyMACAddr));
//		{
//			_prog_addressT MACAddressAddress;
//			MACAddressAddress.next = 0x157F8;
//			_memcpy_p2d24((char*)&AppConfig.MyMACAddr, MACAddressAddress, sizeof(AppConfig.MyMACAddr));
//		}
		AppConfig.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2<<8ul | MY_DEFAULT_IP_ADDR_BYTE3<<16ul | MY_DEFAULT_IP_ADDR_BYTE4<<24ul;
		AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
		AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2<<8ul | MY_DEFAULT_MASK_BYTE3<<16ul | MY_DEFAULT_MASK_BYTE4<<24ul;
		AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
		AppConfig.MyGateway.Val = MY_DEFAULT_GATE_BYTE1 | MY_DEFAULT_GATE_BYTE2<<8ul | MY_DEFAULT_GATE_BYTE3<<16ul | MY_DEFAULT_GATE_BYTE4<<24ul;
		AppConfig.PrimaryDNSServer.Val = MY_DEFAULT_PRIMARY_DNS_BYTE1 | MY_DEFAULT_PRIMARY_DNS_BYTE2<<8ul  | MY_DEFAULT_PRIMARY_DNS_BYTE3<<16ul  | MY_DEFAULT_PRIMARY_DNS_BYTE4<<24ul;
		AppConfig.SecondaryDNSServer.Val = MY_DEFAULT_SECONDARY_DNS_BYTE1 | MY_DEFAULT_SECONDARY_DNS_BYTE2<<8ul  | MY_DEFAULT_SECONDARY_DNS_BYTE3<<16ul  | MY_DEFAULT_SECONDARY_DNS_BYTE4<<24ul;
	
	
		// Load the default NetBIOS Host Name
		memcpypgm2ram(AppConfig.NetBIOSName, (ROM void*)MY_DEFAULT_HOST_NAME, 16);
		FormatNetBIOSName(AppConfig.NetBIOSName);
	
		#if defined(WF_CS_TRIS)
			// Load the default SSID Name
			WF_ASSERT(sizeof(MY_DEFAULT_SSID_NAME) <= sizeof(AppConfig.MySSID));
			memcpypgm2ram(AppConfig.MySSID, (ROM void*)MY_DEFAULT_SSID_NAME, sizeof(MY_DEFAULT_SSID_NAME));
			AppConfig.SsidLength = sizeof(MY_DEFAULT_SSID_NAME) - 1;
	        #if defined (EZ_CONFIG_STORE)
	        AppConfig.SecurityMode = MY_DEFAULT_WIFI_SECURITY_MODE;
	        AppConfig.networkType = MY_DEFAULT_NETWORK_TYPE;
	        AppConfig.dataValid = 0;
	        #endif // EZ_CONFIG_STORE
		#endif

		// Compute the checksum of the AppConfig defaults as loaded from ROM
		wOriginalAppConfigChecksum = CalcIPChecksum((BYTE*)&AppConfig, sizeof(AppConfig));	

		break;
	}

}
