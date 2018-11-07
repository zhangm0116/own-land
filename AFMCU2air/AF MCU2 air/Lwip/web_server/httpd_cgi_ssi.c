/***********************************************************************
文件名称：http_cgi_ssi.c
功    能：
编写时间:  2018.7
编 写 人： jerry
注    意：
***********************************************************************/


#include "lwip/debug.h"
#include "httpd.h"
#include "lwip/tcp.h"
#include "fs.h"
#include "rtthread.h"
#include <string.h>
#include <stdlib.h>
#include "interface.h"


#define NUM_CONFIG_CGI_URIS	(sizeof(ppcURLs) / sizeof(tCGI))
#define NUM_CONFIG_SSI_TAGS	 (sizeof(ppcTAGs) / sizeof(char *))
	
//extern  struct rt_thread    antenna_swtich_thread;
extern struct dev_status_t	dev_status_buf;
extern struct rt_mailbox mb_spi_dev;   

uint8_t  cgibuf[8];

const char* CONFIG_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* TXFREQ_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);
const char* BACKHOME_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);
const char* RXFREQ_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);
const char* TXPOWER_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);
const char* DOWNLINK_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);
const char* ANT_SEL_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);


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
	"o", //ant
	"p"
};

static const tCGI ppcURLs[]=       
{
	{"/configure.cgi",CONFIG_CGI_Handler},
	{"/back.cgi",BACKHOME_CGI_Handler}, 
	{"/txfreq.cgi",TXFREQ_CGI_Handler}, 
	{"/rxfreq.cgi",RXFREQ_CGI_Handler}, 
	{"/txpower.cgi",TXPOWER_CGI_Handler}, 
  {"/downlink.cgi",DOWNLINK_CGI_Handler}, 
	{"/ant_sel.cgi",ANT_SEL_CGI_Handler}

};


//当web客户端请求浏览器的时候,使用此函数被CGI handler调用
static int FindCGIParameter(const char *pcToFind,char *pcParam[],int iNumParams)
{
	int iLoop;
	for(iLoop = 0;iLoop < iNumParams;iLoop ++ )
	{
		if(strcmp(pcToFind,pcParam[iLoop]) == 0)
		{
			return (iLoop); //返回iLOOP
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
  *pcInsert = (char)(dev_status_buf.current_ant%10+0x30);

}

void 	LDPCPASS_Handler(char *pcInsert)
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
void 	LDPCFAIL_Handler(char *pcInsert)
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
	if(dev_status_buf.tx_mode>=7) 
	{		
		memcpy(pcInsert,(&downlink[dev_status_buf.tx_mode][0]),6);
		*(pcInsert+6)='\0';
	}
	else
	{
	  memcpy(pcInsert,(&downlink[dev_status_buf.tx_mode][0]),7);
		*(pcInsert+7)='\0';
	}
  
}
void BW_Handler(char *pcInsert)
{ 
	if(dev_status_buf.tx_mode>=8) memcpy(pcInsert,(&BW_table[dev_status_buf.tx_mode][0]),5);
	else  memcpy(pcInsert,(&BW_table[dev_status_buf.tx_mode][0]),4);

	
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
  *pcInsert = (char)(dev_status_buf.current_ant+0x30);

	
}



void RxLost_Handler(char *pcInsert)
{
  *pcInsert = 'N';
  *(pcInsert+1) = 'U';
	*(pcInsert+2) = 'L';
	*(pcInsert+3) = 'L';
	*(pcInsert+4) = '\0';

	
//	
//  if(dev_status_buf.rx_lost/1000000==0) 
//	{
//	  if((dev_status_buf.rx_lost/100000%10)==0)
//		{
//		  if((dev_status_buf.rx_lost/10000%10)==0)
//			{
//			   if((dev_status_buf.rx_lost/1000%10==0))
//				 {
//					 if(dev_status_buf.rx_lost/100%10==0)
//					 {
//					    if(dev_status_buf.rx_lost/10%10==0)
//						  	{
//							  *pcInsert = (char)(dev_status_buf.rx_lost%10+0x30);       
//							   *(pcInsert+1)= (char)('\0');
//							  }
//					        else{
//								  *(pcInsert+0)= (char)(dev_status_buf.rx_lost/10%10+0x30);
//	                *(pcInsert+1)= (char)(dev_status_buf.rx_lost%10+0x30);
//									*(pcInsert+2)= (char)('\0');
//                  }
//					 }
//				      
//					      else{
//								  *(pcInsert+0)= (char)(dev_status_buf.rx_lost/100%10+0x30);
//									*(pcInsert+1)= (char)(dev_status_buf.rx_lost/10%10+0x30);
//									*(pcInsert+2)= (char)(dev_status_buf.rx_lost%10+0x30);
//									*(pcInsert+3)= (char)('\0');
//										
//               }
//				 }
//			
//			      else	{
//				        *(pcInsert+0)= (char)(dev_status_buf.rx_lost/1000%10+0x30);
//							  *(pcInsert+1)= (char)(dev_status_buf.rx_lost/100%10+0x30);
//								*(pcInsert+2)= (char)(dev_status_buf.rx_lost/10%10+0x30);
//								*(pcInsert+3)= (char)(dev_status_buf.rx_lost%10+0x30);   
//               	*(pcInsert+4)= (char)('\0');				
//      						}	 
//			}
//			 else{     
//				        *(pcInsert+0)= (char)(dev_status_buf.rx_lost/10000%10+0x30);
//			          *(pcInsert+1)= (char)(dev_status_buf.rx_lost/1000%10+0x30);
//								*(pcInsert+2)= (char)(dev_status_buf.rx_lost/100%10+0x30);
//								*(pcInsert+3)= (char)(dev_status_buf.rx_lost/10%10+0x30);
//								*(pcInsert+4)= (char)(dev_status_buf.rx_lost%10+0x30);
//				      	*(pcInsert+5)= (char)('\0');
//       			}
//				
//		}
//		else {      
//				        *(pcInsert+0)=(char)(dev_status_buf.rx_lost/100000%10+0x30);
//								*(pcInsert+1)=(char)(dev_status_buf.rx_lost/10000%10+0x30);
//								*(pcInsert+2)= (char)(dev_status_buf.rx_lost/1000%10+0x30);
//								*(pcInsert+3)= (char)(dev_status_buf.rx_lost/100%10+0x30);
//								*(pcInsert+4)= (char)(dev_status_buf.rx_lost/10%10+0x30);
//								*(pcInsert+5)= (char)(dev_status_buf.rx_lost%10+0x30);
//				        *(pcInsert+6)= (char)('\0');
//			   
//		   }
//		
//	}
//	else 
//	{
//	  *(pcInsert+0)= (char)(dev_status_buf.rx_lost/1000000%10+0x30);
//		*(pcInsert+1)= (char)(dev_status_buf.rx_lost/100000%10+0x30);
//		*(pcInsert+2)= (char)(dev_status_buf.rx_lost/10000%10+0x30);
//	  *(pcInsert+3)= (char)(dev_status_buf.rx_lost/1000%10+0x30);
//	  *(pcInsert+4)= (char)(dev_status_buf.rx_lost/100%10+0x30);
//	  *(pcInsert+5)= (char)(dev_status_buf.rx_lost/10%10+0x30);
//	  *(pcInsert+6)= (char)(dev_status_buf.rx_lost%10+0x30);
//		*(pcInsert+7)= (char)('\0');

//	}


}


static u16_t SSIHandler(int iIndex,char *pcInsert,int iInsertLen)
{  
	 uint8_t num=0;
	 		
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
					 num=strlen(pcInsert);
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
					 num=1;
					 break;
				 case 15:
					RxLost_Handler(pcInsert);			
					 num=strlen(pcInsert);
				 break;	
	}	 
	
	return num;
}


//  返回值为指针类型的字符
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
  		 // rt_kprintf("success %s ",pcValue[iIndex]);
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
//				if(value==0)
//				{
//					 rt_thread_suspend(&antenna_swtich_thread);
//					
//				}
//				else 
				
					
				result = if_set_antenna_sw(value);
				rt_kprintf("set ANTENNA result %d\n",result);
					
				
	
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


