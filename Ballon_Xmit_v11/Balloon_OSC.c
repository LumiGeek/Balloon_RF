/*********************************************************************
 *  
 *	OpenOSC UDP sending routines - jmartin
 *  Module for Microchip TCP/IP Stack
 *	 -Provides device hostname and IP address discovery on a local 
 *    Ethernet subnet (same broadcast domain)
 *	 -Reference: None.  Hopefully AN833 in the future.
 *
 *********************************************************************/ 

#define __OPENOSC_C

#include "TCPIPConfig.h"
#include "TCPIP Stack/TCPIP.h"

#define OSC_PORT	4444
#define OSC_INCOMING_PORT 4445
//#define NO_Z_AXIS 1
//#define JOEJOE
//#define JONNY5
#define ADHOC
//#define OAKLAND

#define SWINGS 0
#define BLISS_DANCE 0
#define FUZZBALL 0
#define BALLOON 1

#define ONLY_Z

NODE_INFO OSC_remote;

#ifdef FUZZBALL
extern BYTE RED1, RED2, BLUE1, BLUE2, GRN1, GRN2;
extern char first_OSC_packet;
#endif


extern NODE_INFO remoteNode;
char OSCXmsg[30]="/accl/x";
char OSCYmsg[30]="/accl/y";
char OSCZmsg[30]="/accl/z";
char namespace_fire[30]="/hurakan/fire";
char namespace_uplight[30]="/hurakan/uplight";
char OSC_MSG[70];
char padlength=1;
int myLightNum=0;
char boosh_on[9];
char boosh_state=0xff;
char update_boosh=0;
extern unsigned char *incoming;
extern unsigned char xmit_frame[];

extern char serial_dump;
extern char new_balloon_pack;
//BYTE up_red1=0, up_green1=0, up_blue1=0;
//BYTE up_red2=0, up_green2=0, up_blue2=0;
//BYTE up_red3=0, up_green3=0, up_blue3=0;
//BYTE up_red4=0, up_green4=0, up_blue4=0;

#define DMX_MAX_NUM  120   
#define OSC_PACK_SIZE 144
//must match MainDemo.C!!!
extern BYTE *DMX_pack;
extern BYTE *BALLOON_raw_pack;


void OSC_update_namespace(IP_ADDR IPVal)
{
	sprintf((char*)OSCXmsg,"/accl%u/x/",IPVal.v[3]);
	sprintf((char*)OSCYmsg,"/accl%u/y/",IPVal.v[3]);
	sprintf((char*)OSCZmsg,"/accl%u/z/",IPVal.v[3]);
	padlength=(strlen(OSCXmsg)%4);
	if(padlength!=0)
		padlength=4-padlength;
	myLightNum=IPVal.v[3]-96;		//we're offsetting by 96 here... so 192.168.1.96 will pick off packet 0 from ARTnet
}

void OSC_xmit_arp()
{

//jonny
#ifdef JONNY5
  OSC_remote.IPAddr.v[0]=192;
  OSC_remote.IPAddr.v[1]=168;
  OSC_remote.IPAddr.v[2]=1;
  OSC_remote.IPAddr.v[3]=253;
// new 00:1b:63:9c:2a:f2
  OSC_remote.MACAddr.v[0]=0x00;
  OSC_remote.MACAddr.v[1]=0x1b;
  OSC_remote.MACAddr.v[2]=0x63;
  OSC_remote.MACAddr.v[3]=0x9c;
  OSC_remote.MACAddr.v[4]=0x2a;
  OSC_remote.MACAddr.v[5]=0xf2;
#endif

// jj laptop wireless

#ifdef JOEJOE
  OSC_remote.IPAddr.v[0]=192;
  OSC_remote.IPAddr.v[1]=168;
  OSC_remote.IPAddr.v[2]=1;
  OSC_remote.IPAddr.v[3]=100;
  OSC_remote.MACAddr.v[0]=0x00;
  OSC_remote.MACAddr.v[1]=0x20;
  OSC_remote.MACAddr.v[2]=0xa6;
  OSC_remote.MACAddr.v[3]=0x4c;
  OSC_remote.MACAddr.v[4]=0x47;
  OSC_remote.MACAddr.v[5]=0x75;
#endif

#ifdef ADHOC
  OSC_remote.IPAddr.v[0]=192;
  OSC_remote.IPAddr.v[1]=168;
  OSC_remote.IPAddr.v[2]=1;
  OSC_remote.IPAddr.v[3]=100;
  OSC_remote.MACAddr.v[0]=0x00;
  OSC_remote.MACAddr.v[1]=0x20;
  OSC_remote.MACAddr.v[2]=0xa6;
  OSC_remote.MACAddr.v[3]=0x4c;
  OSC_remote.MACAddr.v[4]=0x47;
  OSC_remote.MACAddr.v[5]=0x75;
#endif



// jj laptop wired
/*
  OSC_remote.IPAddr.v[0]=192;
  OSC_remote.IPAddr.v[1]=168;
  OSC_remote.IPAddr.v[2]=1;
  OSC_remote.IPAddr.v[3]=101;
  OSC_remote.MACAddr.v[0]=0x00;
  OSC_remote.MACAddr.v[1]=0x0d;
  OSC_remote.MACAddr.v[2]=0x60;
  OSC_remote.MACAddr.v[3]=0xd0;
  OSC_remote.MACAddr.v[4]=0xba;
  OSC_remote.MACAddr.v[5]=0xc5;
*/

 //jj desktop
/*
  OSC_remote.IPAddr.v[0]=192;
  OSC_remote.IPAddr.v[1]=168;
  OSC_remote.IPAddr.v[2]=1;
  OSC_remote.IPAddr.v[3]=50;
  OSC_remote.MACAddr.v[0]=0x00;
  OSC_remote.MACAddr.v[1]=0x24;
  OSC_remote.MACAddr.v[2]=0x8c;
  OSC_remote.MACAddr.v[3]=0x85;
  OSC_remote.MACAddr.v[4]=0xd0;
  OSC_remote.MACAddr.v[5]=0xc3;
*/			

	ARPResolve(&OSC_remote.IPAddr);  //send an ARP packet to the IP address
}


/*********************************************************************
 * Function:        void OSC_accl

	Transmits X Y Z values from accelerometer as OSC packets
	namespace:  /accl/x
				/accl/y
				/accl/z

 ********************************************************************/
void OSC_accl(int acclX, int acclY, int acclZ)
{

	UDP_SOCKET	MySocket;
	BYTE OSCvar[6]=",i";
	BYTE 		i;

    if ( ARPIsResolved(&OSC_remote.IPAddr, &OSC_remote.MACAddr) )
		;		//here we check for a hit on the ARP to the IP address, and update the MAC for the outgoing packet if so

	// Open a UDP socket for outbound broadcast transmission
	MySocket = UDPOpen(0, &OSC_remote, OSC_PORT);
	
	// Abort operation if no UDP sockets are available
	// If this ever happens, incrementing MAX_UDP_SOCKETS in 
	// StackTsk.h may help (at the expense of more global memory 
	// resources).
	if(MySocket == INVALID_UDP_SOCKET)
		return;
	
	// Make certain the socket can be written to
	while(!UDPIsPutReady(MySocket));
	
	// Begin sending our MAC address in human readable form.
	// The MAC address theoretically could be obtained from the 
	// packet header when the computer receives our UDP packet, 
	// however, in practice, the OS will abstract away the useful
	// information and it would be difficult to obtain.  It also 
	// would be lost if this broadcast packet were forwarded by a
	// router to a different portion of the network (note that 
	// broadcasts are normally not forwarded by routers).

#ifndef ONLY_Z
	UDPPutArray((BYTE*)OSCXmsg,strlen(OSCXmsg));
	for(i=0;i<padlength;i++)		//pad out namespace to proper length
		UDPPut(0);

	UDPPutArray(OSCvar,2);		//add the ",i" for integer and pad out
	UDPPut(0);
	UDPPut(0);

	UDPPut(0);					//padding for "  ii"
	UDPPut(0);
	UDPPut((acclX&0xff00)>>8);
	UDPPut(acclX&0xff);
	// Send the packet
	UDPFlush();
	while(!UDPIsPutReady(MySocket));
	

	UDPPutArray((BYTE*)OSCYmsg,strlen(OSCYmsg));
	for(i=0;i<padlength;i++)		//pad out namespace to proper length
		UDPPut(0);

	UDPPutArray(OSCvar,2);
	UDPPut(0);
	UDPPut(0);

	UDPPut(0);
	UDPPut(0);
	UDPPut((acclY&0xff00)>>8);
	UDPPut(acclY&0xff);
	// Send the packet
	UDPFlush();
	while(!UDPIsPutReady(MySocket));
#endif

#ifndef NO_Z_AXIS
	UDPPutArray((BYTE*)OSCZmsg,strlen(OSCZmsg));
	for(i=0;i<padlength;i++)		//pad out namespace to proper length
		UDPPut(0);

	UDPPutArray(OSCvar,2);
	UDPPut(0);
	UDPPut(0);

	UDPPut(0);
	UDPPut(0);
	UDPPut((acclZ&0xff00)>>8);
	UDPPut(acclZ&0xff);
	// Send the packet
	UDPFlush();
#endif
	
	// Close the socket so it can be used by other modules
	UDPClose(MySocket);
 }


/*********************************************************************
 * Function:        void OSCTask(void)
 ********************************************************************/
/*void OSCServer_old_dmx(void)
{
	BYTE r_val,g_val,b_val,v,i, fadernum;
	WORD faderval;
	char* info_start;
	int bufflen=0;
	char fire_cmd;

	static UDP_SOCKET	MySocket = INVALID_UDP_SOCKET;

	
	if(MySocket == INVALID_UDP_SOCKET)
		MySocket = UDPOpen(OSC_INCOMING_PORT, NULL, INVALID_UDP_PORT);

	if(MySocket == INVALID_UDP_SOCKET)
		return;

	// Do nothing if no data is waiting
	if(!UDPIsGetReady(MySocket))
		return;

	bufflen=UDPIsGetReady(MySocket);
	LED0_IO^=1;

    UDPGetArray(OSC_MSG,bufflen);

    if(strstr(OSC_MSG,namespace_uplight)!=NULL)		//uplight command match
	{
	   LED0_IO^=1;
	   up_red=(OSC_MSG[31]);		
	   up_green=(OSC_MSG[35]);
	   up_blue=(OSC_MSG[39]);
	}	

	UDPDiscard();  //toss the reset out
	return;

}
*/

/*********************************************************************
 * Function:        void OSCTask(void)
 ********************************************************************/
void OSCServer(void)
{
	BYTE r_val,g_val,b_val,v,i, fadernum;
	BYTE temp_array[4];
	WORD faderval;
	BYTE header_array[50];
	unsigned char * int_start;
	unsigned char * data_start;
	static UDP_SOCKET	MySocket = INVALID_UDP_SOCKET;
	unsigned char int_count=0;
	unsigned char blob_low=0, blob_high=0, spare_bytes=0;
	unsigned long blob_length=0;
	unsigned long data_offset=0, x=0;
	unsigned char incoming[1510];
	
	if(MySocket == INVALID_UDP_SOCKET)
		MySocket = UDPOpen(OSC_INCOMING_PORT, NULL, INVALID_UDP_PORT);

	if(MySocket == INVALID_UDP_SOCKET)
		return;

	// Do nothing if no data is waiting
	if(!UDPIsGetReady(MySocket))
		return;

//	LED0_IO^=1;

#if BALLOON
		UDPGetArray((BYTE*)&incoming,1510);
	

		int_start=incoming;
		int_count=0;

		while(*int_start!=',')   //find the comma
			int_start++;
	
		data_start=int_start;  //we'll deal with this pointer later
		int_start++;		//skip the comma
	
		while(*int_start==0x62)  //this looks for b, should just be one blob
			{
				int_start++;
				int_count++;
			}
		
		//now we should be at the 0x62 blob identifier... time to compute the offset to the actual goodies
		data_offset=(1+int_count);
		spare_bytes=(data_offset%4);
		data_start+=(data_offset+(4-spare_bytes)); 	//jm: made change to add extra row of 00 00 00 00 if aligned on 4-byte
		

		blob_high=data_start[2];
		blob_low=data_start[3];
		blob_length=(blob_high<<8)+blob_low;
//					blob_length=0x3fc;  //jj :  this is the hardcode, comment out to use actual blob length


		data_start+=4;  //we skip past the blob length field here to the start of the pixel data.


//		framecnt=0;

		for(x=0;x<(blob_length);x++)
			{
				xmit_frame[x]=data_start[x];			//packed data RGBA	

			}
//			no need to call a transmit; blob mode is always transmitting.		
			LED0_IO^=1;					
			UDPDiscard();  //toss the reset out

//			}

//		}
#endif

#if BALLOON_OLD
 	UDPGetArray(header_array,48);
	if((header_array[47]==0x69)&&(header_array[12]=0x2c))
		{
		new_balloon_pack=1;
		//serial_dump=1;		//used to flag a new packet, regardless of serial destination
		UDPGetArray((BYTE*)&BALLOON_raw_pack,OSC_PACK_SIZE);
		LED0_IO^=1;
		}
#endif	

	UDPDiscard();  //toss the reset out
	return;

}
