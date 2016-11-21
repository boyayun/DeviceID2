#ifndef USERUARTLINKCLASS_H
#define USERUARTLINKCLASS_H


#define UART_LINK_HEAD 0xF5      ///< 帧头字节
#define UART_LINK_TRS  0x55      ///< 转义字节

#define UART_LINK_LEN_MIN 0x01   ///< Len字节最小值
#define UART_LINK_LEN_MAX 0x85   ///< Len字节最大值


typedef enum
{
    UART_LINK_HEAD1 = 0,
    UART_LINK_HEAD2,
    UART_LINK_CTL,
    UART_LINK_LEN,
    UART_LINK_LOAD,
}uartLink_state;

typedef struct
{
    unsigned char state=0;
    unsigned char alllen;  ///< 所有字节长度，包含插入的转义字节
    unsigned char linktrs;
    unsigned char datalen; ///< 除去转义字节后的长度，用于计算是否收齐所有字节（因协议len长度不包含插入的转义字节）
    unsigned char data[UART_LINK_LEN_MAX+UART_LINK_LEN_MAX]; ///< 存储链路层所有数据
    unsigned char msg_flag=0; ///< 数据帧标志(0:无可用帧，1:帧可用)
    unsigned char msg_len;
    unsigned char msg_data[UART_LINK_LEN_MAX];
}UserUartLinkContext;


class UserUartLinkClass
{

private:
    UserUartLinkContext sUserUartLinkContext;

    unsigned int crc8408(unsigned char length, unsigned char *address);
    unsigned char bcc_checksum(unsigned char* data,unsigned char len);
    unsigned char strim(unsigned char* des,unsigned char*src,unsigned char srclen);
    unsigned char unstrim(unsigned char*des,unsigned char*src,unsigned char srclen);
    void exitUartLinkUnpack(void);
    unsigned char isUartMsgLenValid(unsigned char len);
    unsigned char isMsgBufBusy(void);


public:
    UserUartLinkClass();
    void initUserUartLink(void);

    /**
         @brief 串口数据链路层组包并解包后存起来
       @attention 串口收到数据后，调用该接口
       @param data 本次处理的数据指针
       @param len 本次处理的数据长度
         @retval none
    */
    bool UserUartLinkUnpack(unsigned char* data,unsigned char len);


    /**
         @brief 获取接收到的数据帧
       @attention 轮询获取接收到的帧，返回值大于零则表示获得有效帧
       @param des 接收数据的指针
         @retval 数据长度(0:无内容)
    */
    unsigned char getUserUartLinkMsg(unsigned char* des);


    /**
         @brief 串口数据链路层组包并打包后通过物理层发送出去
       @attention 将需要发送的数据组包
       @param des 接收打包后的数据的指针
       @param src 需要发送的负载内容，包含帧类型和数据
       @param len 本次处理的数据长度
       @param check 校验类型(2:crc8408,3:bcc,else:none)
         @retval 打包后的字节长度
    */
    unsigned char UserUartLinkPack(unsigned char* des,unsigned char*src,unsigned char srclen,unsigned char check);

};

#endif // USERUARTLINKCLASS_H
