#ifndef _USER_UARTLINK_H_
#define _USER_UARTLINK_H_


/**
******************************************************************************
* @file    UserUartLink.c
* @author  Liqi
* @version V1.1.0
* @date    2016-3-1
* @brief   User Uart Link API
******************************************************************************

******************************************************************************
*/ 

/** @addtogroup UserUartApi
* @{
*/

#define UART_LINK_LEN_MIN 0x01   ///< Len字节最小值
#define UART_LINK_LEN_MAX 0x85   ///< Len字节最大值

typedef enum
{
	UART_LINK_CHECK_NONE=0,
	UART_LINK_CHECK_CRC=2,
	UART_LINK_CHECK_BCC=3,
}uart_link_check;


/**
	 @brief 模块初始化
	 @retval none
*/
extern void initUserUartLink(void);


/**
	 @brief 串口数据链路层组包并解包后存起来
   @attention 串口收到数据后，调用该接口
   @param data 本次处理的数据指针
   @param len 本次处理的数据长度
	 @retval none
*/
extern bool UserUartLinkUnpack(unsigned char* data,unsigned char len);


/**
	 @brief 获取接收到的数据帧
   @attention 轮询获取接收到的帧，返回值大于零则表示获得有效帧
   @param des 接收数据的指针
	 @retval 数据长度(0:无内容)
*/
extern unsigned char getUserUartLinkMsg(unsigned char* des);


/**
	 @brief 串口数据链路层组包并打包后通过物理层发送出去
   @attention 将需要发送的数据组包
   @param des 接收打包后的数据的指针
   @param src 需要发送的负载内容，包含帧类型和数据
   @param len 本次处理的数据长度
   @param check 校验类型(2:crc8408,3:bcc,else:none)
	 @retval 打包后的字节长度
*/
extern unsigned char UserUartLinkPack(unsigned char* des,unsigned char*src,unsigned char srclen,unsigned char check);


#endif




/** 
* @}
*/

