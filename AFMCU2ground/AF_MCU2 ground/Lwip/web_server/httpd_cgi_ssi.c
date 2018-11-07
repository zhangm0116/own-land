#include "lwip/debug.h"
#include "httpd.h"
#include "lwip/tcp.h"
#include "fs.h"
#include "rtthread.h"
#include <string.h>
#include <stdlib.h>
#include "interface.h"
#include "udp_connect.h" 

#define NUM_CONFIG_CGI_URIS	(sizeof(ppcURLs) / sizeof(tCGI))
#define NUM_CONFIG_SSI_TAGS	 (sizeof(ppcTAGs) / sizeof(char *))
	
union uint32_to_uint8
{
   uint32_t value_long;    //
   uint8_t  single_byte[4];	
	
};


union  uint32_to_uint8  u32tou8;
struct dev_status_t	dev_status_buf;
extern udp_connect_struct  udp_con_struct;        
uint8_t  cgibuf[8];

extern struct rt_semaphore		sem_cgi_set; 
extern uint8_t  AirTxBuf[16];

const char* CONFIG_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* TXFREQ_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);
const char* BACKHOME_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);
const char* RXFREQ_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);
const char* TXPOWER_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);
const char* DOWNLINK_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);
const char* ANT_SEL_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);
const char *AIRTXFREQ_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);
const char *AIRANTSEL_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);
const char *AIRRXFREQ_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);
const char *AIRANTSEL_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);
const char *AIRTXPOWER_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);
const char *AIRDOWNLINK_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);

const char downlink[][7]={{'1','/','2','B','P','S','K'},
                     {'2','/','3','B','P','S','K'},
										 {'3','/','4','B','P','S','K'},
										 {'5','/','6','B','P','S','K'},
										 {'1','/','2','Q','P','S','K'},
										 {'2','/','3','Q','P','S','K'},
										 {'3','/','4','Q','P','S','K'},
										 {'5','/','6','Q','P','S','K'},
										 {'1','/','2','Q','A','M'},
										 {'2','/','3','Q','A','M'},
										 {'3','/','4','Q','A','M'},
										 {'5','/','6','Q','A','M'},     };
 
										 
const char BW_table[][5]={ {'2','.','3','1'},
                           {'3','.','0','8'},
                           {'3','.','4','7'},
                           {'3','.','8','6'},
                           {'4','.','6','3'}, 
													 {'6','.','1','7'},
													 {'6','.','9','4'},
													 {'7','.','7','1'},
													 {'9','.','2','5'},
													 {'1','1','.','8','3'},
													 {'1','3','.','3','1'},
													 {'1','4','.','7','8'} };

static const char *ppcTAGs[]=  //SSI的Tag
{              
	          
	"a",   //   txfreq
	"b",   //   rxfreq
	"c",  //    ldpc pass
	"d",   //   ldpc fail
	"e",  //    snr   
  "f",
	"g",
	"h",
	"i",
	"j",
	"k",
	"l",
	"m",
	"n",
	"o",
	"ga",
	"gb",
	"gc",
	"gd",
	"ge",
	"gf",
	"gg",
	"gh",
	"gi",
	"gj", //j
	"gk",
	"gl",
	"gm",
	"gn",
	"go"
	
};


static const tCGI ppcURLs[]=       
{
	{"/configure.cgi",CONFIG_CGI_Handler},
	{"/back.cgi",BACKHOME_CGI_Handler}, 
	{"/txfreq.cgi",TXFREQ_CGI_Handler}, 
	{"/rxfreq.cgi",RXFREQ_CGI_Handler}, 
	{"/txpower.cgi",TXPOWER_CGI_Handler}, 
  {"/downlink.cgi",DOWNLINK_CGI_Handler}, 
	{"/ant_sel.cgi",ANT_SEL_CGI_Handler},
	{"/airtxfreq.cgi",AIRTXFREQ_CGI_Handler},
	{"/airrxfreq.cgi",AIRRXFREQ_CGI_Handler},
	{"/airtxpower.cgi",AIRTXPOWER_CGI_Handler},
	{"/airdownlink.cgi",AIRDOWNLINK_CGI_Handler},
	{"/airantsel.cgi",AIRANTSEL_CGI_Handler}

};


static int FindCGIParameter(const char *pcToFind,char *pcParam[],int iNumParams)
{
	int iLoop;
	for(iLoop = 0;iLoop < iNumParams;iLoop ++ )
	{
		if(strcmp(pcToFind,pcParam[iLoop]) == 0)
		{
			
			return (iLoop);   //返回iLOOP
		}
	}
	    return (-1);
	
}



void TXfreq_Handler(char *pcInsert)
{ 
        
    *pcInsert 		= (char)(((dev_status_buf.rf_tx_freq)/1000)+0x30);
		*(pcInsert+1) = (char)(((dev_status_buf.rf_tx_freq/100)%10)+0x30);
		*(pcInsert+2)	=	(char)(((dev_status_buf.rf_tx_freq/10)%10)+0x30);
		*(pcInsert+3) = (char)(dev_status_buf.rf_tx_freq%10+0x30);

}

void RXfreq_Handler(char *pcInsert)
{

		*pcInsert 		= (char)(((dev_status_buf.rf_rx_freq)/1000)+0x30);
		*(pcInsert+1) = (char)(((dev_status_buf.rf_rx_freq/100)%10)+0x30);
		*(pcInsert+2)	=	(char)(((dev_status_buf.rf_rx_freq/10)%10)+0x30);
		*(pcInsert+3) = (char)(dev_status_buf.rf_rx_freq%10+0x30);
	
	
	
}
void RFtemp_Handler(char *pcInsert)
{
   
		*(pcInsert)	=	(char)(((dev_status_buf.ad9361_temp/10))+0x30);
		*(pcInsert+1) = (char)(dev_status_buf.ad9361_temp%10+0x30);

}
void BBtemp_Handler(char *pcInsert)
{
   *pcInsert = (char)(dev_status_buf.fpga_temp/10+0x30);
	 *(pcInsert+1) = (char)(dev_status_buf.fpga_temp%10+0x30);
}
void 	Antenna_Handler(char *pcInsert)
{
  *pcInsert = (char)(dev_status_buf.ant%10+0x30);

}

void	LDPCPASS_Handler(char *pcInsert)
{ 
	if(dev_status_buf.ldpc_pass/1000000==0) 
	{
	  if((dev_status_buf.ldpc_pass/100000%10)==0)
		{
		  if((dev_status_buf.ldpc_pass/10000%10)==0)
			{
			   if((dev_status_buf.ldpc_pass/1000%10==0))
				 {
					 if(dev_status_buf.ldpc_pass/100%10==0)
					  {
					    if(dev_status_buf.ldpc_pass/10%10==0)
						  	{
							  *pcInsert = (char)(dev_status_buf.ldpc_pass%10+0x30);       
							  *(pcInsert+1)= (char)('\0');
							  }
					      else{
								  *(pcInsert+0)= (char)(dev_status_buf.ldpc_pass%100+0x30);
	                *(pcInsert+1)= (char)(dev_status_buf.ldpc_pass%10+0x30);
									*(pcInsert+2)= (char)('\0');
                  }
					  }
				    else{
					      *(pcInsert+0)= (char)(dev_status_buf.ldpc_pass/100%10+0x30);
								*(pcInsert+1)= (char)(dev_status_buf.ldpc_pass/10%10+0x30);
								*(pcInsert+2)= (char)(dev_status_buf.ldpc_pass%10+0x30);
							  *(pcInsert+3)= (char)('\0');
               }
				 }
			else	{
				        *(pcInsert+0)= (char)(dev_status_buf.ldpc_pass/1000%10+0x30);
								*(pcInsert+1)= (char)(dev_status_buf.ldpc_pass/100%10+0x30);
								*(pcInsert+2)= (char)(dev_status_buf.ldpc_pass/10%10+0x30);
								*(pcInsert+3)= (char)(dev_status_buf.ldpc_pass%10+0x30);
                *(pcInsert+4)= (char)('\0');				
       			}	 
			}
			 else{ 
			
							  *(pcInsert+0)= (char)(dev_status_buf.ldpc_pass/10000%10+0x30);
								*(pcInsert+1)= (char)(dev_status_buf.ldpc_pass/1000%10+0x30);
								*(pcInsert+2)= (char)(dev_status_buf.ldpc_pass/100%10+0x30);
								*(pcInsert+3)= (char)(dev_status_buf.ldpc_pass/10%10+0x30);
								*(pcInsert+4)= (char)(dev_status_buf.ldpc_pass%10+0x30);
				        *(pcInsert+5)= (char)('\0');
       			}
				
		}
		else {
							 	*(pcInsert+0)=(char)(dev_status_buf.ldpc_pass/100000%10+0x30);
								*(pcInsert+1)=(char)(dev_status_buf.ldpc_pass/10000%10+0x30);
								*(pcInsert+2)= (char)(dev_status_buf.ldpc_pass/1000%10+0x30);
								*(pcInsert+3)= (char)(dev_status_buf.ldpc_pass/100%10+0x30);
								*(pcInsert+4)= (char)(dev_status_buf.ldpc_pass/10%10+0x30);
								*(pcInsert+5)= (char)(dev_status_buf.ldpc_pass%10+0x30);
			          *(pcInsert+6)= (char)('\0');
		   }
		
	}
	else 
	{
	  *(pcInsert+0)= (char)(dev_status_buf.ldpc_pass/1000000%10+0x30);
		*(pcInsert+1)= (char)(dev_status_buf.ldpc_pass/100000%10+0x30);
		*(pcInsert+2)= (char)(dev_status_buf.ldpc_pass/10000%10+0x30);
	  *(pcInsert+3)= (char)(dev_status_buf.ldpc_pass/1000%10+0x30);
	  *(pcInsert+4)= (char)(dev_status_buf.ldpc_pass/100%10+0x30);
	  *(pcInsert+5)= (char)(dev_status_buf.ldpc_pass/10%10+0x30);
	  *(pcInsert+6)= (char)(dev_status_buf.ldpc_pass%10+0x30);
    *(pcInsert+7)= (char)('\0');
	}

}
void	LDPCFAIL_Handler(char *pcInsert)
{
  if(dev_status_buf.ldpc_failed/1000000==0) 
	{
	  if((dev_status_buf.ldpc_failed/100000%10)==0)
		{
		  if((dev_status_buf.ldpc_failed/10000%10)==0)
			{
			   if((dev_status_buf.ldpc_failed/1000%10==0))
				 {
					 if(dev_status_buf.ldpc_failed/100%10==0)
					 {
					    if(dev_status_buf.ldpc_failed/10%10==0)
						  	{
							  *pcInsert = (char)(dev_status_buf.ldpc_failed%10+0x30);       
							   *(pcInsert+1)= (char)('\0');
							  }
					        else{
								  *(pcInsert+0)= (char)(dev_status_buf.ldpc_failed/10%10+0x30);
	                *(pcInsert+1)= (char)(dev_status_buf.ldpc_failed%10+0x30);
									*(pcInsert+2)= (char)('\0');
                  }
					 }
				      
					      else{
								  *(pcInsert+0)= (char)(dev_status_buf.ldpc_failed/100%10+0x30);
									*(pcInsert+1)= (char)(dev_status_buf.ldpc_failed/10%10+0x30);
									*(pcInsert+2)= (char)(dev_status_buf.ldpc_failed%10+0x30);
									*(pcInsert+3)= (char)('\0');
										
               }
				 }
			
			      else	{
				        *(pcInsert+0)= (char)(dev_status_buf.ldpc_failed/1000%10+0x30);
							  *(pcInsert+1)= (char)(dev_status_buf.ldpc_failed/100%10+0x30);
								*(pcInsert+2)= (char)(dev_status_buf.ldpc_failed/10%10+0x30);
								*(pcInsert+3)= (char)(dev_status_buf.ldpc_failed%10+0x30);   
               	*(pcInsert+4)= (char)('\0');				
      						}	 
			}
			 else{     
				        *(pcInsert+0)= (char)(dev_status_buf.ldpc_failed/10000%10+0x30);
			          *(pcInsert+1)= (char)(dev_status_buf.ldpc_failed/1000%10+0x30);
								*(pcInsert+2)= (char)(dev_status_buf.ldpc_failed/100%10+0x30);
								*(pcInsert+3)= (char)(dev_status_buf.ldpc_failed/10%10+0x30);
								*(pcInsert+4)= (char)(dev_status_buf.ldpc_failed%10+0x30);
				      	*(pcInsert+5)= (char)('\0');
       			}
				
		}
		else {      
				        *(pcInsert+0)=(char)(dev_status_buf.ldpc_failed/100000%10+0x30);
								*(pcInsert+1)=(char)(dev_status_buf.ldpc_failed/10000%10+0x30);
								*(pcInsert+2)= (char)(dev_status_buf.ldpc_failed/1000%10+0x30);
								*(pcInsert+3)= (char)(dev_status_buf.ldpc_failed/100%10+0x30);
								*(pcInsert+4)= (char)(dev_status_buf.ldpc_failed/10%10+0x30);
								*(pcInsert+5)= (char)(dev_status_buf.ldpc_failed%10+0x30);
				        *(pcInsert+6)= (char)('\0');
			   
		   }
		
	}
	else 
	{
	  *(pcInsert+0)= (char)(dev_status_buf.ldpc_failed/1000000%10+0x30);
		*(pcInsert+1)= (char)(dev_status_buf.ldpc_failed/100000%10+0x30);
		*(pcInsert+2)= (char)(dev_status_buf.ldpc_failed/10000%10+0x30);
	  *(pcInsert+3)= (char)(dev_status_buf.ldpc_failed/1000%10+0x30);
	  *(pcInsert+4)= (char)(dev_status_buf.ldpc_failed/100%10+0x30);
	  *(pcInsert+5)= (char)(dev_status_buf.ldpc_failed/10%10+0x30);
	  *(pcInsert+6)= (char)(dev_status_buf.ldpc_failed%10+0x30);
		*(pcInsert+7)= (char)('\0');

	}
	
}
void SNR_Handler(char *pcInsert)
{ 
  *pcInsert = (char)(dev_status_buf.SNR/10+0x30);
	*(pcInsert+1) = (char)(dev_status_buf.SNR%10+0x30);
	
}
 void TXPOWER_Handler(char *pcInsert)
 { 
	    if((dev_status_buf.rf_power/10000)==0)
			{
		    *(pcInsert)= (char)(dev_status_buf.rf_power/1000%10+0x30);
				*(pcInsert+1)= (char)(dev_status_buf.rf_power/100%10+0x30);
				*(pcInsert+2)= (char)(dev_status_buf.rf_power/10%10+0x30);
				*(pcInsert+3)= (char)(dev_status_buf.rf_power%10+0x30);   
				*(pcInsert+4)= (char)('\0');
			 }
			 else{ 
	
				*(pcInsert)=(char)(dev_status_buf.rf_power/10000+0x30);
				*(pcInsert+1)= (char)(dev_status_buf.rf_power/1000%10+0x30);
				*(pcInsert+2)= (char)(dev_status_buf.rf_power/100%10+0x30);
				*(pcInsert+3)= (char)(dev_status_buf.rf_power/10%10+0x30);
				*(pcInsert+4)= (char)(dev_status_buf.rf_power%10+0x30);
				*(pcInsert+5)= (char)('\0');
       			}
 }
 
 void Downlink_Handler(char *pcInsert)
{  
	if(dev_status_buf.rx_mode>=7)  memcpy(pcInsert,(&downlink[dev_status_buf.rx_mode][0]),6);
	else
	{
	  memcpy(pcInsert,(&downlink[dev_status_buf.rx_mode][0]),7);
	}

}
void BW_Handler(char *pcInsert)
{ 
	if(dev_status_buf.rx_mode>=8) memcpy(pcInsert,(&BW_table[dev_status_buf.rx_mode][0]),5);
	else  memcpy(pcInsert,(&BW_table[dev_status_buf.rx_mode][0]),4);

}
void	AVGA_Handler(char *pcInsert)
{

  *pcInsert = (char)(dev_status_buf.A_VGA/10+0x30);
	*(pcInsert+1) = (char)(dev_status_buf.A_VGA%10+0x30);

}
void	ARSSI_Handler(char *pcInsert)
{
    *pcInsert =(char)(0x2d);
		*(pcInsert+1) = (char)(dev_status_buf.A_RSSI/10+0x30);
	  *(pcInsert+2) = (char)(dev_status_buf.A_RSSI%10+0x30);

}

void 	BVGA_Handler(char *pcInsert)
{ 
   *pcInsert = (char)(dev_status_buf.B_VGA/10+0x30);
   *(pcInsert+1) = (char)(dev_status_buf.B_VGA%10+0x30);

}
void BRSSI_Handler(char *pcInsert)
{
    *pcInsert =(char)(0x2d);
		*(pcInsert+1) = (char)(dev_status_buf.B_RSSI/10+0x30);
	  *(pcInsert+2) = (char)(dev_status_buf.B_RSSI%10+0x30);
	  
}
void ANTENNA_Handler(char *pcInsert)	
{ 
	if(dev_status_buf.ant==0)
	{
	    *pcInsert = (char)(1+0x30);
			*(pcInsert+1)= (char)('_');
		  *(pcInsert+2)= (char)(3+0x30);
		
	}
	else if(dev_status_buf.ant==1)
	{
		  *pcInsert = (char)(2+0x30);
			*(pcInsert+1)= (char)('_');
		  *(pcInsert+2)= (char)(4+0x30);
	
	}
 else ;

}
void AIRTXfreq_Handler(char *pcInsert)
{   
	  u32tou8.value_long=0;
	  u32tou8.value_long=udp_con_struct.udp_rcv_buf[17]*256+udp_con_struct.udp_rcv_buf[16];
		*pcInsert = (char)((u32tou8.value_long/1000)+0x30);
		*(pcInsert+1) = (char)(((u32tou8.value_long/100)%10)+0x30);
		*(pcInsert+2)	=	(char)(((u32tou8.value_long/10)%10)+0x30);
		*(pcInsert+3) = (char)(u32tou8.value_long%10+0x30);

}

void AIRRXfreq_Handler(char *pcInsert)
{   
	  u32tou8.value_long=0;
	  u32tou8.value_long=udp_con_struct.udp_rcv_buf[21]*256+udp_con_struct.udp_rcv_buf[20];
	  *pcInsert = (char)((u32tou8.value_long/1000)+0x30);
		*(pcInsert+1) = (char)(((u32tou8.value_long/100)%10)+0x30);
		*(pcInsert+2)	=	(char)(((u32tou8.value_long/10)%10)+0x30);
		*(pcInsert+3) = (char)(u32tou8.value_long%10+0x30);

}

void AIRLDPCPASS_Handler(char *pcInsert)
{ 
	u32tou8.value_long=0;
	u32tou8.single_byte[0]=udp_con_struct.udp_rcv_buf[0];
	u32tou8.single_byte[1]=udp_con_struct.udp_rcv_buf[1];
	u32tou8.single_byte[2]=udp_con_struct.udp_rcv_buf[2];
	u32tou8.single_byte[3]=udp_con_struct.udp_rcv_buf[3];
	
  if(u32tou8.value_long/1000000==0) 
	{
	  if((u32tou8.value_long/100000%10)==0)
		{
		  if((u32tou8.value_long/10000%10)==0)
			{
			   if((u32tou8.value_long/1000%10==0))
				 {
					 if(u32tou8.value_long/100%10==0)
					  {
					    if(u32tou8.value_long/10%10==0)
						  	{
							  *pcInsert = (char)(u32tou8.value_long%10+0x30);       
							  *(pcInsert+1)= (char)('\0');
							  }
					      else{
								  *(pcInsert+0)= (char)(u32tou8.value_long%100+0x30);
	                *(pcInsert+1)= (char)(u32tou8.value_long%10+0x30);
									*(pcInsert+2)= (char)('\0');
                  }
					  }
				    else{
					      *(pcInsert+0)= (char)(u32tou8.value_long/100%10+0x30);
								*(pcInsert+1)= (char)(u32tou8.value_long/10%10+0x30);
								*(pcInsert+2)= (char)(u32tou8.value_long%10+0x30);
               }
				 }
			else	{
				        *(pcInsert+0)= (char)(u32tou8.value_long/1000%10+0x30);
								*(pcInsert+1)= (char)(u32tou8.value_long/100%10+0x30);
								*(pcInsert+2)= (char)(u32tou8.value_long/10%10+0x30);
								*(pcInsert+3)= (char)(u32tou8.value_long%10+0x30);   
       			}	 
			}
			 else{ 
			
							  *(pcInsert+0)= (char)(u32tou8.value_long/10000%10+0x30);
								*(pcInsert+1)= (char)(u32tou8.value_long/1000%10+0x30);
								*(pcInsert+2)= (char)(u32tou8.value_long/100%10+0x30);
								*(pcInsert+3)= (char)(u32tou8.value_long/10%10+0x30);
								*(pcInsert+4)= (char)(u32tou8.value_long%10+0x30);
       			}
				
		}
		else {
							 	*(pcInsert+0)=(char)(u32tou8.value_long/100000%10+0x30);
								*(pcInsert+1)=(char)(u32tou8.value_long/10000%10+0x30);
								*(pcInsert+2)= (char)(u32tou8.value_long/1000%10+0x30);
								*(pcInsert+3)= (char)(u32tou8.value_long/100%10+0x30);
								*(pcInsert+4)= (char)(u32tou8.value_long/10%10+0x30);
								*(pcInsert+5)= (char)(u32tou8.value_long%10+0x30);
			   
		   }
		
	}
	else 
	{
	  *(pcInsert+0)= (char)(u32tou8.value_long/1000000%10+0x30);
		*(pcInsert+1)= (char)(u32tou8.value_long/100000%10+0x30);
		*(pcInsert+2)= (char)(u32tou8.value_long/10000%10+0x30);
	  *(pcInsert+3)= (char)(u32tou8.value_long/1000%10+0x30);
	  *(pcInsert+4)= (char)(u32tou8.value_long/100%10+0x30);
	  *(pcInsert+5)= (char)(u32tou8.value_long/10%10+0x30);
	  *(pcInsert+6)= (char)(u32tou8.value_long%10+0x30);

	}

}
void AIRLDPCFAIL_Handler(char *pcInsert)
{
     
	u32tou8.single_byte[0]=udp_con_struct.udp_rcv_buf[4];
	u32tou8.single_byte[1]=udp_con_struct.udp_rcv_buf[5];
	u32tou8.single_byte[2]=udp_con_struct.udp_rcv_buf[6];
	u32tou8.single_byte[3]=udp_con_struct.udp_rcv_buf[7];
	 if(u32tou8.value_long/1000000==0) 
	{
	  if((u32tou8.value_long/100000%10)==0)
		{
		  if((u32tou8.value_long/10000%10)==0)
			{
			   if((u32tou8.value_long/1000%10==0))
				 {
					 if(u32tou8.value_long/100%10==0)
					 {
					    if(u32tou8.value_long/10%10==0)
						  	{
							  *pcInsert = (char)(u32tou8.value_long%10+0x30);       
							   *(pcInsert+1)= (char)('\0');
							  }
					        else{
								  *(pcInsert+0)= (char)(u32tou8.value_long/10%10+0x30);
	                *(pcInsert+1)= (char)(u32tou8.value_long%10+0x30);
									*(pcInsert+2)= (char)('\0');
                  }
					 }
				      
					      else{
								  *(pcInsert+0)= (char)(u32tou8.value_long/100%10+0x30);
									*(pcInsert+1)= (char)(u32tou8.value_long/10%10+0x30);
									*(pcInsert+2)= (char)(u32tou8.value_long%10+0x30);
									*(pcInsert+3)= (char)('\0');
										
               }
				 }
			
			      else	{
				        *(pcInsert+0)= (char)(u32tou8.value_long/1000%10+0x30);
							  *(pcInsert+1)= (char)(u32tou8.value_long/100%10+0x30);
								*(pcInsert+2)= (char)(u32tou8.value_long/10%10+0x30);
								*(pcInsert+3)= (char)(u32tou8.value_long%10+0x30);   
               	*(pcInsert+4)= (char)('\0');				
      						}	 
			}
			 else{     
				        *(pcInsert+0)= (char)(u32tou8.value_long/10000%10+0x30);
			          *(pcInsert+1)= (char)(u32tou8.value_long/1000%10+0x30);
								*(pcInsert+2)= (char)(u32tou8.value_long/100%10+0x30);
								*(pcInsert+3)= (char)(u32tou8.value_long/10%10+0x30);
								*(pcInsert+4)= (char)(u32tou8.value_long%10+0x30);
				      	*(pcInsert+5)= (char)('\0');
       			}
				
		}
		else {      
				        *(pcInsert+0)=(char)(u32tou8.value_long/100000%10+0x30);
								*(pcInsert+1)=(char)(u32tou8.value_long/10000%10+0x30);
								*(pcInsert+2)= (char)(u32tou8.value_long/1000%10+0x30);
								*(pcInsert+3)= (char)(u32tou8.value_long/100%10+0x30);
								*(pcInsert+4)= (char)(u32tou8.value_long/10%10+0x30);
								*(pcInsert+5)= (char)(u32tou8.value_long%10+0x30);
				        *(pcInsert+6)= (char)('\0');
			   
		   }
		
	}
	else 
	{
	  *(pcInsert+0)= (char)(u32tou8.value_long/1000000%10+0x30);
		*(pcInsert+1)= (char)(u32tou8.value_long/100000%10+0x30);
		*(pcInsert+2)= (char)(u32tou8.value_long/10000%10+0x30);
	  *(pcInsert+3)= (char)(u32tou8.value_long/1000%10+0x30);
	  *(pcInsert+4)= (char)(u32tou8.value_long/100%10+0x30);
	  *(pcInsert+5)= (char)(u32tou8.value_long/10%10+0x30);
	  *(pcInsert+6)= (char)(u32tou8.value_long%10+0x30);
		*(pcInsert+7)= (char)('\0');
}
}
void  AIRSNR_Handler(char *pcInsert)
{
      *pcInsert = (char)(udp_con_struct.udp_rcv_buf[8]/10+0x30); 
      *(pcInsert+1)= (char)(udp_con_struct.udp_rcv_buf[8]%10+0x30); 
}
void AIRTXPOWER_Handler(char *pcInsert)
{
  u32tou8.value_long=0;
	u32tou8.value_long=udp_con_struct.udp_rcv_buf[25]*256+udp_con_struct.udp_rcv_buf[24];
	
  if((dev_status_buf.rf_power/10000)==0)
			{
		    *(pcInsert)= (char)(u32tou8.value_long/1000%10+0x30);
				*(pcInsert+1)= (char)(u32tou8.value_long/100%10+0x30);
				*(pcInsert+2)= (char)(u32tou8.value_long/10%10+0x30);
				*(pcInsert+3)= (char)(u32tou8.value_long%10+0x30);   
				*(pcInsert+4)= (char)('\0');
			 }
			 else{ 
	
				*(pcInsert)=(char)(u32tou8.value_long/10000+0x30);
				*(pcInsert+1)= (char)(u32tou8.value_long/1000%10+0x30);
				*(pcInsert+2)= (char)(u32tou8.value_long/100%10+0x30);
				*(pcInsert+3)= (char)(u32tou8.value_long/10%10+0x30);
				*(pcInsert+4)= (char)(u32tou8.value_long%10+0x30);
				*(pcInsert+5)= (char)('\0');
       			}


}
void AIRRFtemp_Handler(char *pcInsert)
{
	
	 	*(pcInsert)	=	(char)(((udp_con_struct.udp_rcv_buf[60]/10))+0x30);
		*(pcInsert+1) = (char)(udp_con_struct.udp_rcv_buf[60]%10+0x30);  
      
}
void AIRBBtemp_Handler(char *pcInsert)
{
   *pcInsert = (char)(udp_con_struct.udp_rcv_buf[56]/10+0x30);
	 *(pcInsert+1) = (char)(udp_con_struct.udp_rcv_buf[56]%10+0x30);   

}
void  AIRDownlink_Handler(char *pcInsert)
{
 
	
  if(udp_con_struct.udp_rcv_buf[28]>=7)  memcpy(pcInsert,(&downlink[udp_con_struct.udp_rcv_buf[28]]),6);
	else
	{
	  memcpy(pcInsert,(&downlink[udp_con_struct.udp_rcv_buf[28]]),7);
	}
}

void AIRBW_Handler(char *pcInsert)
{

  if(udp_con_struct.udp_rcv_buf[28]>=8) memcpy(pcInsert,(&BW_table[udp_con_struct.udp_rcv_buf[28]]),5);
	else  memcpy(pcInsert,(&BW_table[udp_con_struct.udp_rcv_buf[28]]),4);
 
}
void AIRAVGA_Handler(char *pcInsert)
{
  *pcInsert=(char)(udp_con_struct.udp_rcv_buf[9]/10+0x30);
	*(pcInsert+1) = (char)(udp_con_struct.udp_rcv_buf[9]%10+0x30);

}
void  AIRARSSI_Handler(char *pcInsert)
{
    *pcInsert =(char)(0x2d);
		*(pcInsert+1) = (char)(udp_con_struct.udp_rcv_buf[10]/10+0x30);
	  *(pcInsert+2) = (char)(udp_con_struct.udp_rcv_buf[10]%10+0x30);


}
void AIRBVGA_Handler(char *pcInsert)
{ 
 
	*pcInsert=(char)(udp_con_struct.udp_rcv_buf[11]/10+0x30);
	*(pcInsert+1) = (char)(udp_con_struct.udp_rcv_buf[11]%10+0x30);

 }
void  AIRBRSSI_Handler(char *pcInsert)
{
    *pcInsert =(char)(0x2d);
		*(pcInsert+1) = (char)(udp_con_struct.udp_rcv_buf[12]/10+0x30);
	  *(pcInsert+2) = (char)(udp_con_struct.udp_rcv_buf[12]%10+0x30);


}
void  AIRANTENNA_Handler(char *pcInsert)
{

	*pcInsert=(char)(udp_con_struct.udp_rcv_buf[13]+0x30);

}

static u16_t SSIHandler(int iIndex,char *pcInsert,int iInsertLen)
{  
	 uint8_t num=0;
	    memset(&dev_status_buf,0,62);
		if_get_dev_status(&dev_status_buf, sizeof(struct dev_status_t));
					 switch(iIndex)
				{
					case 0: 
						TXfreq_Handler(pcInsert);
					  num=4;
						break;
					case 1:
						RXfreq_Handler(pcInsert);
					  num=4;
						break;
					case 2: 
						LDPCPASS_Handler(pcInsert);
					  num=strlen(pcInsert);
						break;
					case 3:
					 LDPCFAIL_Handler(pcInsert);
					 num=strlen(pcInsert);
						break;
					case 4:
						SNR_Handler(pcInsert);
					 num=2;
						break;
					case 5:
					 TXPOWER_Handler(pcInsert);
					 num=strlen(pcInsert);
					 break;
					case 6:
					 RFtemp_Handler(pcInsert);
					 num=2;
					 break;
					case 7:
					 BBtemp_Handler(pcInsert);
					 num=2;
					 break;
					case 8:
					 Downlink_Handler(pcInsert);
				   num=7;
					 break;
					case 9:
					BW_Handler(pcInsert);
					num=4;
					 break;
					case 10:
					AVGA_Handler(pcInsert);
					num=2;
					 break;
				 case 11:
					ARSSI_Handler(pcInsert);
				   num=3;
					 break;
				  case 12:
					BVGA_Handler(pcInsert);
					num=2;
					 break;
					 case 13:
					BRSSI_Handler(pcInsert);
					 num=3;
					 break;
					 case 14:
					ANTENNA_Handler(pcInsert);
					 num=3;
					 break;
				 case  15: 	
					AIRTXfreq_Handler(pcInsert);
					 num=4;
					 break;
				 case 16:
					AIRRXfreq_Handler(pcInsert);
					  num=4;
						break;
				 case 17:
					AIRLDPCPASS_Handler(pcInsert);
					 num=strlen(pcInsert);
						break;
				 case 18:
					 AIRLDPCFAIL_Handler(pcInsert);
					 num=strlen(pcInsert);
						break;
				case 19:
					 AIRSNR_Handler(pcInsert);
					 num=2;
						break;
				case 20:
					  AIRTXPOWER_Handler(pcInsert); 
				    num=strlen(pcInsert);
				    break;
				case  21:
					 AIRRFtemp_Handler(pcInsert);
				   num=2;
			  	break;
				case  22:
					 AIRBBtemp_Handler(pcInsert);
				   num=2;
			  	break;
				case  23:
					 AIRDownlink_Handler(pcInsert);
				   num=7;
			  	 break;	
				case 24 :  
					AIRBW_Handler(pcInsert);
					num=4; break; 
				case  25:
					AIRAVGA_Handler(pcInsert);
					num=2; break;
				case  26:
					AIRARSSI_Handler(pcInsert);
					num=3; break;
				case  27:
			   AIRBVGA_Handler(pcInsert);
					num=2; break;
				case  28:
			   AIRBRSSI_Handler(pcInsert);
					num=3; break;
				case  29:
			   AIRANTENNA_Handler(pcInsert);
					num=1; break;
				 default: break;
				
	}	 
	// memset(&udp_con_struct.udp_rcv_buf[0],0,62);
	return num;
}
		

const char* CONFIG_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  return  "/configure.shtm";   
}

const char* BACKHOME_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[])
{
	 	return  "/autoflight.shtm";  

}
  
const char* TXFREQ_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[])   //  定义字符串数组 
{
      uint8_t  i=0,result=0;
	    uint8_t  len=0;
	    uint32_t value=0;
	   

	    iIndex = FindCGIParameter("txfreq",pcParam,iNumParams); 
	   if(iIndex!=-1)
 	   {  //rt_kprintf("%s ",pcValue[iIndex]);
	  	  len=strlen(pcValue[iIndex]);
		    if(len!=4)    return  "/configure.shtm";   
		    memcpy(cgibuf,pcValue[iIndex],len); 
			 
			  for(i=0;i<len;i++)            	
			 {  
				
			    cgibuf[i]= cgibuf[i]-0x30;
				 
				}  
	
			 value=(cgibuf[0]*1000)+(cgibuf[1]*100)+(cgibuf[2]*10)+cgibuf[3];	
      result = if_set_freq(value, 1);
			rt_kprintf("set txfreq result %d\n",result);

	   }
     
     return  "/configure.shtm";   
}
const char* RXFREQ_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[])   //  定义字符串数组 
{
      uint8_t  i=0,result=0;
	    uint8_t  len=0;
	    uint32_t value=0;
	   

	    iIndex = FindCGIParameter("rxfreq",pcParam,iNumParams); 
	   if(iIndex!=-1)
 	   {  //rt_kprintf("success %s ",pcValue[iIndex]);
	  	  len=strlen(pcValue[iIndex]);
		    if(len!=4)    return  "/configure.shtm";   
		    memcpy(cgibuf,pcValue[iIndex],len); 
			 
			  for(i=0;i<len;i++)            	
			 {  
				
			    cgibuf[i]= cgibuf[i]-0x30;
				 
				}  
	
			value=(cgibuf[0]*1000)+(cgibuf[1]*100)+(cgibuf[2]*10)+cgibuf[3];	
      result = if_set_freq(value, 0);
			rt_kprintf("set rxfreq result %d\n",result);
     }
     return  "/configure.shtm";   
}

const char* TXPOWER_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[])
{
     
      uint8_t  i=0,result=0;
	    uint8_t  len=0;
	    uint32_t value=0;
	   
     iIndex = FindCGIParameter("txpower",pcParam,iNumParams); 
	   if(iIndex!=-1)
 	   {  
  		  rt_kprintf("success %s ",pcValue[iIndex]);
	  	  len=strlen(pcValue[iIndex]);
		    if(len>5)    return  "/configure.shtm";   
		    memcpy(cgibuf,pcValue[iIndex],len); 
			  for(i=0;i<len;i++)            	
			 {  
				 cgibuf[i]= cgibuf[i]-0x30;
				}  
	     if(len==4)   value=(cgibuf[0]*1000)+(cgibuf[1]*100)+(cgibuf[2]*10)+cgibuf[3];	
			 else   value=(cgibuf[0]*10000)+(cgibuf[1]*1000)+(cgibuf[2]*100)+(cgibuf[3]*10)+cgibuf[4];	
				
       result = if_set_rf_power(value);
			 rt_kprintf("set txpower result %d\n",result);
      }
     return  "/configure.shtm";   
}

const char* DOWNLINK_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[])
{

      uint8_t  i=0,result=0;
	    uint8_t  len=0;
	    uint32_t value=0;
	   
     iIndex = FindCGIParameter("downlink",pcParam,iNumParams); 
	   if(iIndex!=-1)
 	   {  //rt_kprintf("%s ",pcValue[iIndex]);
	  	  len=strlen(pcValue[iIndex]);
		    if(len>2)    return  "/configure.shtm";   
		    memcpy(cgibuf,pcValue[iIndex],len); 
			  for(i=0;i<len;i++)            	
			 {   
				 cgibuf[i]= cgibuf[i]-0x30;
				}  
	     if(len==2)   value=(cgibuf[0]*10)+cgibuf[1];	
			 else   value=cgibuf[0];	
		
#ifdef AIR_MODULE	
				result = if_set_downlink(value, 1);
#else
				result = if_set_downlink(value, 0);
#endif	
			 rt_kprintf("set downlink result %d\n",result);
      }
     return  "/configure.shtm";   

}
const char* ANT_SEL_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[])
{
  
      uint8_t  i=0,result=0;
	    uint8_t  len=0;
	    uint32_t value=0;
	   
     iIndex = FindCGIParameter("ant_sel",pcParam,iNumParams); 
	   if(iIndex!=-1)
 	   {  //rt_kprintf("%s ",pcValue[iIndex]);
	  	  len=strlen(pcValue[iIndex]);
		    if(len>1)    return  "/configure.shtm";   
		    memcpy(cgibuf,pcValue[iIndex],len); 
			 
			  for(i=0;i<len;i++)            	
			  {   
				 cgibuf[i]= cgibuf[i]-0x30;
				}  
			  
			 value=cgibuf[0];	
			 result = if_set_antenna_sw(value);
			 rt_kprintf("set ANTENNA result %d\n",result);
     }
     return  "/configure.shtm"; 
}

const char *AIRTXFREQ_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[])
{    
	    uint8_t  i=0;
	    uint8_t  len=0;
	 
      iIndex = FindCGIParameter("airtxfreq",pcParam,iNumParams); 
	   	   if(iIndex!=-1)
 	   {  //rt_kprintf("%s ",pcValue[iIndex]);
	  	  len=strlen(pcValue[iIndex]);
		    if(len>4)    return  "/configure.shtm";   
		   memcpy(cgibuf,pcValue[iIndex],len); 
			 memset(&AirTxBuf,0,16);
			  for(i=0;i<len;i++)            	
			  {   
				 cgibuf[i]= cgibuf[i]-0x30;
					AirTxBuf[i]=cgibuf[i];
				}  
			 rt_sem_release(&sem_cgi_set);
			 rt_kprintf("send txfreq %d.%d.%d.%d\n",AirTxBuf[0],AirTxBuf[1],AirTxBuf[2],AirTxBuf[3]);
     }
     return  "/configure.shtm";   
}

const char *AIRRXFREQ_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[])
{

	    uint8_t  i=0,result=0;
	    uint8_t  len=0;
	    uint32_t value=0;
      iIndex = FindCGIParameter("airrxfreq",pcParam,iNumParams); 
	   	   if(iIndex!=-1)
 	   {  //rt_kprintf("%s ",pcValue[iIndex]);
	  	  len=strlen(pcValue[iIndex]);
		    if(len>4)    return  "/configure.shtm";   
		   memcpy(cgibuf,pcValue[iIndex],len); 
			 memset(&AirTxBuf,0,16);
			  for(i=0;i<len;i++)            	
			  {   
				 cgibuf[i]= cgibuf[i]-0x30;
					AirTxBuf[4+i]=cgibuf[i];
				}  
			 rt_sem_release(&sem_cgi_set);
			 rt_kprintf("send rxfreq %d.%d.%d.%d\n",AirTxBuf[4],AirTxBuf[5],AirTxBuf[6],AirTxBuf[7]);
     }
     return  "/configure.shtm";   
}
const char *AIRTXPOWER_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[])
{

	    uint8_t  i=0,result=0;
	    uint8_t  len=0;
	    uint32_t value=0;
      iIndex = FindCGIParameter("airtxpower",pcParam,iNumParams); 
	   	   if(iIndex!=-1)
 	   {  //rt_kprintf("%s ",pcValue[iIndex]);
	  	  len=strlen(pcValue[iIndex]);
		    if(len>5)    return  "/configure.shtm";   
		   memcpy(cgibuf,pcValue[iIndex],len); 
			 memset(&AirTxBuf,0,16);
			  if(len==4)
				{
				 for(i=0;i<len;i++)            	
			  {   
				 cgibuf[i]= cgibuf[i]-0x30;
					AirTxBuf[9+i]=cgibuf[i];
				}  
				}
				else if(len==5)
				{
					for(i=0;i<len;i++)            	
					{   
					 cgibuf[i]= cgibuf[i]-0x30;
						AirTxBuf[8+i]=cgibuf[i];
					}  
				}
				else ;
			 rt_sem_release(&sem_cgi_set);
			 rt_kprintf("send txpower %d.%d.%d.%d.%d\r\n",AirTxBuf[8],AirTxBuf[9],AirTxBuf[10],AirTxBuf[11],AirTxBuf[12]);
     }
      return  "/configure.shtm";   
}

const char *AIRDOWNLINK_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[])
{

	    uint8_t  i=0,result=0;
	    uint8_t  len=0;
	    uint32_t value=0;
      iIndex = FindCGIParameter("airdownlink",pcParam,iNumParams); 
	   	if(iIndex!=-1)
 	   {  //rt_kprintf("%s ",pcValue[iIndex]);
	  	  len=strlen(pcValue[iIndex]);
		    if(len>1)    return  "/configure.shtm";   
		   memcpy(cgibuf,pcValue[iIndex],len); 
			 memset(&AirTxBuf,0,16);
			 if(len==1)   AirTxBuf[14]=cgibuf[0]-0x30;
			 else  if(len==2) 
			 {		
				 AirTxBuf[13]=cgibuf[0]-0x30;
				 AirTxBuf[14]=cgibuf[1]-0x30;
			 }
			rt_sem_release(&sem_cgi_set);
			 rt_kprintf("send downlink %d.%d\r\n",AirTxBuf[13],AirTxBuf[14]);
     }
     return  "/configure.shtm";   
}
const char *AIRANTSEL_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[])
{

	    uint8_t  i=0,result=0;
	    uint8_t  len=0;
	    uint32_t value=0;
      iIndex = FindCGIParameter("airantsel",pcParam,iNumParams); 
	   	   if(iIndex!=-1)
 	   {  //rt_kprintf("%s ",pcValue[iIndex]);
	  	  len=strlen(pcValue[iIndex]);
		    if(len>1)    return  "/configure.shtm";   
		   memcpy(cgibuf,pcValue[iIndex],len); 
			 memset(&AirTxBuf,0,16);
			  for(i=0;i<len;i++)            	
			  {   
				 cgibuf[i]= cgibuf[i]-0x30;
					AirTxBuf[15]=cgibuf[i];
				}  
			 rt_sem_release(&sem_cgi_set);
			 rt_kprintf("send antsel %d \r\n",AirTxBuf[15]);
     }
     return  "/configure.shtm";   
}
//SSI句柄初始化
void httpd_ssi_init(void)
{  

	http_set_ssi_handler(SSIHandler,ppcTAGs,NUM_CONFIG_SSI_TAGS);
	
}

//CGI句柄初始化
void httpd_cgi_init(void)
{ 
  
	
  http_set_cgi_handlers(ppcURLs, NUM_CONFIG_CGI_URIS);
	
	
	
	
}


