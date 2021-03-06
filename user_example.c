/* *Copyright(C), Haijie Lee. 2019. AllRightsReserved.
	*FileName:  用户编程入口文件.
	*Author:  Haijie Lee.
	*Version:  1.5.
	*Date:  2019.01.22.
	*Description:  初始化IOT通信的数据内容, 以供用户编程。
							本模块依赖<coap>处理coap流，<nb_cmd>处理AT指令，
							以此完成IOT通信的过程。
	*Others:  用户不应随意修改IOT定义的数据，除非你知道你在做什么。
*/
#include "user_example.h"
/* 本模块的依赖与接口声明 */
#include "coap.h"
#include "nb_cmd.h"

/* 公共依赖 */ 
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


// ——————————————    本地设置    —————————————— //

// ——————————————    本地定义    —————————————— //

/* 用户在平台定义的TLV的名称 */
typedef enum
{
	General_Message_Buf = 0,
	General_Cmd_Buf,
	Mac_Address,
	Error_Code,
}TLV_Name_Define;

/* 用户在平台定义的上报消息的名称 */
typedef enum
{
	General_Message_ID = 1,
	TemperatureSensor_Message_ID = 10,
}Message_ID_Define;

/* 用户在平台定义的下发命令的名称 */
typedef enum
{
	General_Cmd_ID = 101,
}Command_ID_Define;

// ——————————————    本地变量   —————————————— //

uint8_t message1_tlv[2] = {1, General_Message_Buf};		// 消息“General_Message”所包含的的TLV的索引
uint8_t message2_tlv[3] = {2, Mac_Address, General_Message_Buf}; 	// 消息“TemperatureSensor_Message”所包含的的TLV的索引

uint8_t cmd1_tlv[2] = {1, General_Cmd_Buf };	// 命令“General_Cmd”所包含的的TLV的索引

uint8_t cmd1_ack_tlv[2] = {1, Error_Code};	// 命令“General_Cmd”的回复所包含的的TLV的索引

// ——————————————    接口变量   —————————————— //

IOT_TlvCollector_t gl_TLV_table[TLV_MAX] = {
	{0x0f, 1, TLV_STRING_HEX, NULL,},		// TLV: General_Message_Buf
	{0x0f, 2, TLV_STRING_HEX, NULL},		// TLV: General_Cmd_Buf
	{0x0f, 4, TLV_STRING_HEX, NULL},		//TLV:  Mac_Address
	{0x01, 7, TLV_ENUM, NULL},	// TLV: Error_Code
};	// TLV表格（包含所有在平台定义的传感器）

IOT_MessageCollector_t gl_Message_table[MESSAGE_MAX] = {
	{General_Message_ID, 0, 0, message1_tlv},		// General_Message_Buf
	{TemperatureSensor_Message_ID, 0, 0, message2_tlv},		// TemperatureSensor_Message
};	// Message表格（包含所有在平台定义的上报消息）

IOT_CmdCollector_t gl_CMD_table[CMD_MAX] = {
	{General_Cmd_ID, 0, cmd1_tlv},
};	// command表格（包含所有在平台定义的下发命令）

IOT_AckCollector_t gl_ACK_table[CMD_MAX] = {
	{General_Cmd_ID, 0, 0, cmd1_ack_tlv, NULL},
};	// Cmd ACK 表格 （包含所有在平台定义的下发命令的回复）

// ——————————————    本地函数    —————————————— //

/* 功能：初始化IOT通信模块
	说明：无。
	参数：无。
	返回值：成功返回 0，失败返回错误码。
*/
uint8_t Init_NB_IOT(void);


/* 功能：命令回调函数（当用户回复一条命令时，会调用此函数）。
	说明：此函数在此仅为示范作用，用户可根据需要自行修改。
	参数：cmd_type: 传入命令ID，用于在回调函数中识别命令。
	返回值：成功返回 0，失败返回错误码。
*/
uint8_t ReceiveCMD_callback(uint8_t cmd_type);

void main(void);

// ——————————————    本地函数源    —————————————— //

/*	-----------  < 用户编程指引 >  ------------
	提示：应先确定硬件功能接口文件<Hardware Interface>中的功能实现，再按照以下步骤编程。
	
	1. 初始化 Init_NB_IOT()；
	2. 设置命令回调函数 Put_CmdCallback()；
	3. 在某个循环中检查是否接收到消息，以及消息、命令是否发送成功;
	4. “发送消息“没有过多限制，用户可根据实际的需要去选择发送时机。
*/

void main(void)
{
	uint8_t ret, coap_type, message_id;
	
	do{
		ret = Init_NB_IOT();
	}while(ret != 0);
	
	Put_CmdCallback( General_Cmd_ID, ReceiveCMD_callback );
	
	Put_BetteryLevel( 80);
	Put_SignalStrength( 0x00001234 );
	Put_TimeStamp( 0x2019012400173050 );
	Put_TlvValue( General_Message_Buf, "Hello world" );
	IncrementSendCoapMessage( CMT_Message, General_Message_ID );
	
	while(1)
	{
		ret = CheckSendMessageError();
		if( ret ) {
			printf("Send message error code : %d .\n", ret );
		}
		
		ret = CheckReceiveMessage();
		if( ret ) {
			DecrementReceiveCoapMessage(  &coap_type, &message_id );
			if( coap_type == CMT_CMD )
			{
				printf("Receive commmand id[%d] and response it.\n", message_id );
				IncrementSendCoapMessage( CMT_CMD_ACK, message_id );
			}
			else if( coap_type == CMT_Message_ACK )
			{
				printf("Receive message response id[%d] .\n", message_id );
			}
		}
	}
}

uint8_t Init_NB_IOT(void)
{
	uint8_t ret, imei[16]={0}, imsi[16]={0};
	
	ret = Init_NB( imei, imsi );
	if( ret )
		return ret;
	ret = Init_Coap( imei, imsi );
	if( ret )
		return ret;
	
	return 0;
}

uint8_t ReceiveCMD_callback(uint8_t cmd_type)
{
	if( cmd_type == General_Cmd_ID )
	{
		Get_TlvFromCmd();	// 解析TLV
		printf("Receive General_Cmd: '%s'.\n", Get_TlvValue(char*, General_Cmd_Buf) );
		return 0; 	// 返回0代表命令执行成功，其它数值表示执行失败。
	}
	return 1;
}

