/* *Copyright(C), Haijie Lee. 2019. AllRightsReserved.
	*FileName:  BC26模组的通信处理 模块头文件.
	*Author:  Haijie Lee.
	*Version:  1.5.
	*Date:  2019.01.22.
	*Description:  用户可通过调用此文件的接口函数完成【AT指令和Coap数据流】的发送/接收。
	*Others:  用户不应随意修改此文件，除非你知道自己在做什么。
*/
#ifndef IOT_API__COMPONENT__BC26_MODULE_H
#define IOT_API__COMPONENT__BC26_MODULE_H

#include <stdint.h>



// ——————————————   接口数据格式     —————————————— //

/* 发送AT指令时，需要把指令的内容，和该指令的（NB模组的）回复内容，
	打包成以下的数据格式。
*/

typedef struct cmd_response_
{
	char *response;
	struct cmd_response_ *next;
}cmd_response_t;	// “指令回复”数据包

typedef struct cmd_pack_
{
	char *cmd;
	uint8_t cmd_response_max;
	uint8_t cmd_response_n;
	cmd_response_t *reg_response;
	cmd_response_t *cmd_response;
}CmdPack_t;	// “指令”数据包


// ——————————————    接口变量    —————————————— //



// ——————————————   接口函数     —————————————— //

/* 功能：检查是否接收到消息。
	说明：若接收到消息则可进行下一步处理。
	参数：无。
	返回值：收到消息返回1，失败返回0。
*/
uint8_t IF_ReceiveMessage(void);

/* 功能：检查是否接收到AT指令回复。
	说明：若接收到回复代表AT指令执行结束。
	参数：无。
	返回值：收到指令回复返回1，失败返回0。
*/
uint8_t IF_ReveivedCmdResponse(void);

/* 功能：检查Coap发送是否出错。
	说明：无。
	参数：无。
	返回值：出错返回1，成功返回0。
*/
uint8_t IF_SendMessageError(void);


/* 功能：发送Coap消息。
	说明：此函数仅为把Coap流加入发送列队中，实际发送时机由本模块决定。
	参数：@buff: Coap数据流缓冲区；
				@data_length: Coap数据流长度。
	返回值：成功返回 0，失败返回错误码。
*/
uint8_t SendMessage( uint8_t *buff, uint8_t data_length );

/* 功能：接收Coap消息。
	说明：此函数仅为把Coap流从接收列队中取出，实际接收的时机由本模块决定。
	参数：@out_buff: 接收【Coap数据流缓冲区】；
				@buff_size: 数据流缓冲区的大小；
				@data_length: 接收【Coap数据流长度】的指针。。
	返回值：成功返回 0，失败返回错误码。
*/
uint8_t ReceiveMessage( uint8_t *out_buff, uint16_t buff_size, uint16_t *data_length );

/* 功能：发送AT指令到NB模组。
	说明：若无法发送命令，可能是当前正在发送其他命令。
	参数：@cmd: 命令包；
				@delay; 发送此命令后的延迟时间（常用于等待NB模组执行命令）。
	返回值：成功返回 0，失败返回错误码。
*/
uint8_t SendCmd( CmdPack_t *cmd, uint16_t delay );

/* 功能：接收当前的（正在执行或执行完成的）指令包。
	说明：无。
	参数：无。
	返回值：无执行命令返回 NULL，有则返回包地址。
*/
CmdPack_t* ReceiveCmd	(void);


/* 功能：尝试把数据写入串口。
	说明：无。
	参数：无。
	返回值：成功返回 0，失败返回错误码。
*/
uint8_t WriteData_ToSerialPort( void );

/* 功能：尝试从串口读出数据。
	说明：无。
	参数：无。
	返回值：成功返回 0，失败返回错误码。
*/
uint8_t ReadData_FromSerialPort( void );

void RepealCmd(void);

// ——————————————    END    —————————————— //


#endif
