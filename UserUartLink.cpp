//#include "MICO.h"
//#include "SocketUtils.h"
//#include "MICOAPPDefine.h"
//#include "UserConfig.h"
//#include "..\UserLib\geniot.h"
//#include "UserUtilities.h"

#include "UserUartLink.h"
// #pragma diag_suppress 174

/**
******************************************************************************
* @file    UserUartLink.c
* @author  Liqi
* @version V1.0.0
* @date    2016-2-23
* @brief   UserUart API,pack/unpack the msg to/from bytes
******************************************************************************

******************************************************************************
*/ 

/** @addtogroup UserUartApi
* @{
*/

#define UART_LINK_HEAD 0xF5      ///< 帧头字节
#define UART_LINK_TRS  0x55      ///< 转义字节


typedef enum
{
	UART_LINK_HEAD1 = 0,
	UART_LINK_HEAD2,
	UART_LINK_CTL,
	UART_LINK_LEN,
	UART_LINK_LOAD,
}uartLink_state;

static struct
{
	unsigned char state;
	unsigned char alllen;  ///< 所有字节长度，包含插入的转义字节
	unsigned char linktrs;
	unsigned char datalen; ///< 除去转义字节后的长度，用于计算是否收齐所有字节（因协议len长度不包含插入的转义字节）
	unsigned char data[UART_LINK_LEN_MAX+UART_LINK_LEN_MAX]; ///< 存储链路层所有数据
	unsigned char msg_flag; ///< 数据帧标志(0:无可用帧，1:帧可用)
	unsigned char msg_len;
	unsigned char msg_data[UART_LINK_LEN_MAX];
}sUserUartLinkContext;


unsigned int crc8408(unsigned char length, unsigned char *address)
{   
	unsigned char i,j;
	unsigned short crc;
	crc=0xffff;
	for(i=0;i<length;i++)
	  {
	crc^= *address++;
	for(j=0;j<8;j++)
	    {
	if(crc&0x01)
	       {
	crc=(crc>>1)^0x8408;
	       }
	else
		  {
		crc>>=0x01;
		  } 
	     }   	
	   }
	return ~crc;
}

unsigned char bcc_checksum(unsigned char* data,unsigned char len)
{
	unsigned char i,result = 0;
	
	for(i = 0;i < len;i++)
	{
		result ^= data[i];
	}
	return result;
}

static unsigned char strim(unsigned char* des,unsigned char*src,unsigned char srclen)
{
	// 将数据区域中的0xF5后面的0x55去掉
	unsigned char i;
	unsigned char deslen=0;
	unsigned char linkstr = 0;
	
	for(i = 0;i < srclen ;i++){
		if (linkstr == 0){
			if (src[i] == UART_LINK_HEAD)
			{
				linkstr = 1;
				des[deslen++] = src[i];
			} 
			else
			{
				des[deslen++] = src[i];
			}
		}else{
			if (src[i] == UART_LINK_TRS)
			{
				linkstr = 0;
			} 
			else
			{
				return 0;
			}
		}
	}
	return deslen;
}

static unsigned char unstrim(unsigned char*des,unsigned char*src,unsigned char srclen)
{
	// 在数据区域中0xF5后面插入0x55
	unsigned char i;
	unsigned char deslen = 0;
	
	for(i = 0;i < srclen;i++)
	{
		des[deslen++] = src[i];
		if(src[i] == UART_LINK_HEAD)
		{
			des[deslen++] = UART_LINK_TRS;
		}
		else
		{
			
		}
	}
	return deslen;
	
}


void initUserUartLink(void)
{
	sUserUartLinkContext.state = 0;
}

inline static void exitUartLinkUnpack(void)
{
    sUserUartLinkContext.state = UART_LINK_HEAD1;
	  sUserUartLinkContext.alllen = 0;
	  sUserUartLinkContext.datalen = 0;
}

inline static unsigned char isUartMsgLenValid(unsigned char len)
{
	if((len <= UART_LINK_LEN_MAX)
   &&(len >= UART_LINK_LEN_MIN)){
		return 1;
	}else{
		return 0;
	}
}

unsigned char isMsgBufBusy(void)
{
	if(sUserUartLinkContext.msg_flag == 1)return 1;
	else return 0;
}

/**
	 @brief 获取接收到的数据帧
   @attention
   @param des 接收数据的指针
	 @retval 数据长度
*/
unsigned char getUserUartLinkMsg(unsigned char* des)
{
	// memcpy(des,sUserUartLinkContext.msg_data,sUserUartLinkContext.msg_len);
	unsigned char i;
	
	if(sUserUartLinkContext.msg_flag != 0)
	{
		for(i = 0;i < sUserUartLinkContext.msg_len;i++)
		{
			des[i] = sUserUartLinkContext.msg_data[i];
		}
		sUserUartLinkContext.msg_flag = 0;
	}
	else
	{
		return 0;
	}
	
	return sUserUartLinkContext.msg_len;
}


/**
	 @brief 串口数据链路层组包并解包后存起来
   @attention
   @param data 本次处理的数据指针
   @param len 本次处理的数据长度
	 @retval none
*/
bool UserUartLinkUnpack(unsigned char* data,unsigned char len)
{
	unsigned char i;
	unsigned char check;
	unsigned char valid;
	unsigned short crc;
	unsigned short crccalc;
	unsigned char crch;
	unsigned char crcl;
	unsigned char bcc;
	unsigned char checklen;
	//unsigned char msglen;
	//unsigned char msg[UART_LINK_LEN_MAX];
	
	for(i = 0;i < len;i++)
	{
		switch(sUserUartLinkContext.state)
		{
			case UART_LINK_HEAD1:
				   if(isMsgBufBusy() == 1)break;
			     
				   if(data[i] == UART_LINK_HEAD){
					     sUserUartLinkContext.alllen = 0;
						   sUserUartLinkContext.datalen = 0;
						   sUserUartLinkContext.data[sUserUartLinkContext.alllen++] = data[i];
						 sUserUartLinkContext.datalen++;
						   sUserUartLinkContext.state++;
					 }
				   break;
			
			case UART_LINK_HEAD2:
				   if(data[i] == UART_LINK_HEAD){
						   sUserUartLinkContext.data[sUserUartLinkContext.alllen++] = data[i];
						   sUserUartLinkContext.datalen++;
						   sUserUartLinkContext.state++;
					 }else{
						   exitUartLinkUnpack();
                           return false;
					 }
				   break;
			
			case UART_LINK_CTL:
				   sUserUartLinkContext.data[sUserUartLinkContext.alllen++] = data[i];
			     sUserUartLinkContext.datalen++;
					 sUserUartLinkContext.state++;
				   break;
			
			case UART_LINK_LEN:
				   if(isUartMsgLenValid(data[i])){
					     sUserUartLinkContext.data[sUserUartLinkContext.alllen++] = data[i];
						   sUserUartLinkContext.datalen++;
					     sUserUartLinkContext.state++;
						 sUserUartLinkContext.linktrs = 0;
					 }else{
					     exitUartLinkUnpack();
                         return false;
					 }
				   break;
			
			case UART_LINK_LOAD:
					sUserUartLinkContext.data[sUserUartLinkContext.alllen++] = data[i];
					if (sUserUartLinkContext.linktrs == 0){
						if (data[i] == UART_LINK_HEAD){
							sUserUartLinkContext.linktrs = 1;
						}else{
							sUserUartLinkContext.datalen++;
						}
					}else{
						if (data[i] == UART_LINK_TRS){
							sUserUartLinkContext.datalen++;
							sUserUartLinkContext.linktrs = 0;
						}else{
							exitUartLinkUnpack();
                            return false;
						}
					}
 
				   if(sUserUartLinkContext.datalen >= (sUserUartLinkContext.data[UART_LINK_LEN]+UART_LINK_LEN+1) )
					 {			 
						 if (sUserUartLinkContext.linktrs != 0){
							 exitUartLinkUnpack();
                             return false;
						 }
						 
						 // 收齐byets，先Strim出CRC/BCC再check再Strim出data再app【混乱！！】

						 /**
						 校验
						 */
						 check = (sUserUartLinkContext.data[UART_LINK_CTL] >> 6) & 0x01;
						 // HEAD1 HEAD2 CTL LEN DATA CRC[2]/BCC
						 valid = 0;
						 if(check == 0){ // CRC
							   if(sUserUartLinkContext.data[sUserUartLinkContext.alllen-2] == UART_LINK_HEAD)
								 {
									 crcl = sUserUartLinkContext.data[sUserUartLinkContext.alllen-2];
									 if(sUserUartLinkContext.data[sUserUartLinkContext.alllen-4] == UART_LINK_HEAD)
									 {
										 crch = sUserUartLinkContext.data[sUserUartLinkContext.alllen-4];
										 checklen = 4;
									 }
									 else
									 {
										 crch = sUserUartLinkContext.data[sUserUartLinkContext.alllen-3];
										 checklen = 3;
									 }
								 }
								 else
                                {
									 crcl = sUserUartLinkContext.data[sUserUartLinkContext.alllen-1];
									 if(sUserUartLinkContext.data[sUserUartLinkContext.alllen-3] == UART_LINK_HEAD)
									 {
										 crch = sUserUartLinkContext.data[sUserUartLinkContext.alllen-3];
										 checklen = 3;
									 }
									 else
									 {
										 crch = sUserUartLinkContext.data[sUserUartLinkContext.alllen-2];
										 checklen = 2;
									 }
								 }
								 
							   
							   crc = crch;
							   crc <<= 8;
							   crc += crcl;
							   crccalc = crc8408(sUserUartLinkContext.alllen-checklen,sUserUartLinkContext.data);
						     if(crc == crccalc){
									 valid = 1;
								 }
						 }else{ // BCC
                               if(sUserUartLinkContext.data[sUserUartLinkContext.alllen-3] == UART_LINK_HEAD)
								 {
                                     bcc = sUserUartLinkContext.data[sUserUartLinkContext.alllen-3];
                                     checklen = 3;
								 }
								 else
								 {
                                     bcc = sUserUartLinkContext.data[sUserUartLinkContext.alllen-2];
                                     checklen = 2;
								 }
							   
						     if(bcc == bcc_checksum(sUserUartLinkContext.data,sUserUartLinkContext.alllen-checklen)){
									 valid = 1;
								 }
						 }
						 
						 /**
						 Strim
						 */
						 if(valid == 1){
							   if(sUserUartLinkContext.msg_flag == 0){
									 sUserUartLinkContext.msg_len = strim(sUserUartLinkContext.msg_data,\
									                                      &sUserUartLinkContext.data[UART_LINK_LOAD],\
									                                      sUserUartLinkContext.alllen-UART_LINK_LOAD-checklen);
									 if(sUserUartLinkContext.msg_len > 0){
								     sUserUartLinkContext.msg_flag = 1;
										 
								   }
								 }
								 
							   //msglen = strim(msg,&sUserUartLinkContext.data[UART_LINK_LOAD],sUserUartLinkContext.alllen-UART_LINK_LOAD-checklen);
						     //if(msglen > 0){
									 // UserUartAppParse(msg,msglen); ///< 应用协议处理
								 //}
						 }
						 
						 exitUartLinkUnpack();
                         return true;						 
					 }
				   break;
			

			default:
				    exitUartLinkUnpack();
                    return false;
				   break;
		}
	}
    return false;
}


/**
	 @brief 串口数据链路层组包并打包后通过物理层发送出去
   @attention
   @param des 接收打包后的数据的指针
   @param src 需要发送的负载内容，包含帧类型和数据
   @param len 本次处理的数据长度
   @param check 校验类型(0:crc8408,else:bcc)
	 @retval 打包后的字节长度
*/
unsigned char UserUartLinkPack(unsigned char* des,unsigned char*src,unsigned char srclen,unsigned char check)
{
	//unsigned char des[UART_LINK_LEN_MAX+UART_LINK_LEN_MAX];
	unsigned char msg_len;
	unsigned short crc;
	unsigned char bcc;
	unsigned char checklen;
	unsigned char unstrimlen;
	unsigned char alllen;
	unsigned char tmp;
	
	// 先unstrim，再计算CRC/BCC，再unstrimCRC/BCC，再send
	msg_len=0;
	des[msg_len++] = UART_LINK_HEAD;
	des[msg_len++] = UART_LINK_HEAD;
	
	if(check == 0){
	    des[msg_len++] = 0x00;
//		  checklen = 2;     //20161101校验域为固定2字节
	}else{
	    des[msg_len++] = 0x40;
//		  checklen = 1;     //20161101校验域为固定2字节
	}
    checklen = 2;           //20161101校验域为固定2字节
	des[msg_len++] = srclen+checklen;
	
	unstrimlen = unstrim(&des[msg_len],src,srclen);
	alllen = unstrimlen+msg_len;
	
	if(check == 0){  // crc
	    crc = crc8408(alllen,des);
		  
		  tmp = (unsigned char)(crc >> 8);
		  if(tmp == UART_LINK_HEAD)
			{
				des[alllen++] = UART_LINK_HEAD;
				des[alllen++] = UART_LINK_TRS;
			}
			else
			{
			    des[alllen++] = tmp;
			}
			
			tmp = (unsigned char)(crc&0x00ff);
			if(tmp == UART_LINK_HEAD)
			{
				des[alllen++] = UART_LINK_HEAD;
				des[alllen++] = UART_LINK_TRS;
			}
			else
			{
			    des[alllen++] = tmp;
			}
			
	}else{ // bcc
	    bcc = bcc_checksum(des,unstrimlen+msg_len);
		  tmp = bcc;
		  if(tmp == UART_LINK_HEAD)
			{
				des[alllen++] = UART_LINK_HEAD;
				des[alllen++] = UART_LINK_TRS;
                des[alllen++] = 0;      //20161101校验域两字节，低字节补0
			}
			else
			{
			    des[alllen++] = tmp;
                des[alllen++] = 0;      //20161101校验域两字节，低字节补0
			}
	}
	//UserUartSend(des,alllen); ///< 串口发送，Mico自带底层缓冲因此不需要压入队列
	return alllen;
}



/** 
* @}
*/













