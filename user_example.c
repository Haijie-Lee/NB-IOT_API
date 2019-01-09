/*	功能：应用编程入口文件。
	说明：TLV表格、上报消息数据表格、下发命令（包括回复）表格，
				均为根据IOT平台上定义的通信内容而生成的，用户不应更改其内容除非你知道自己在做什么。
	其它：这个示例是基于 BC26-NB模组 的。
	Function: 		Application programming entry file.
	Description: 	TLV form, report message data form, issue order (including reply) form,
							both are generated based on the communication content defined on the IOT-platform, 
							and user should not change their content unless you know what you are doing.
	Other:				This example is based on the BC26-NB module.
*/
#include "iot_user.h"



/* 注意：这里使用宏函数代替单片机的串口驱动功能，仅为示范函数功能。
				在实际应用编程中，用户应该自己实现这些功能。
	Note:	The macro function here represents the serial port driver's sending message function just for example.
				In practical applications, user should implement these function by yourself.
*/
#define	 UART_Write( buf, length) 	( printf("UART send message to nb module: '%s' , message length: %d\n", buf, length) )
#define	 delay( ms )	do{ \
	size_t i, j; 	\
	for(i=0; i<ms; i++)	\
		for(j=0; j<100; j++);	\
}while(0);



// --------------------- TLV 初始化 Initialization  -------------------- //
typedef enum
{
	General_Message_Buf = 0,
	General_Cmd_Buf,
	Mac_Address,
	Error_Code,
}TLV_Name_Define;

IOT_TlvCollector_t gl_TLV_table[TLV_MAX] = {
	{0x0f, 1, TLV_STRING_HEX, NULL,},		// General_Message_Buf
	{0x0f, 2, TLV_STRING_HEX, NULL},		// General_Cmd_Buf
	{0x0f, 4, TLV_STRING_HEX, NULL},		// Mac_Address
	{0x01, 7, TLV_ENUM, NULL},	// Error_Code
};

// --------------------- 上报消息初始化 Report message Initialization -------------------- //
typedef enum
{
	General_Message_ID = 1,
	TemperatureSensor_Message_ID = 10,
}Message_ID_Define;

uint8_t message1_tlv[2] = {1, General_Message_Buf}; 
uint8_t message2_tlv[3] = {2, Mac_Address, General_Message_Buf}; 

IOT_MessageCollector_t gl_Message_table[MESSAGE_MAX] = {
	{General_Message_ID, 0, 0, message1_tlv},		// General_Message_Buf
	{TemperatureSensor_Message_ID, 0, 0, message2_tlv},		// TemperatureSensor_Message
};

// --------------------- 下发命令初始化  Issue command Initialization -------------------- //
typedef enum
{
	General_Cmd_ID = 101;
}Command_ID_Define;

uint8_t cmd1_tlv[2] = {1, General_Cmd_Buf };

IOT_CmdCollector_t gl_CMD_table[CMD_MAX] = {
	{General_Cmd_ID, 0, cmd1_tlv},
};

// --------------------- 下发命令回复初始化 Issue command response Initialization -------------------- //

uint8_t cmd1_ack_tlv[2] = {1, Error_Code};

IOT_AckCollector_t gl_ACK_table[CMD_MAX] = {
	{General_Cmd_ID, 0, 0, cmd1_ack_tlv, NULL},
};



// -----------------------------------  用户的函数  User's Function ------------------------------------------  //


uint8_t NB_serialport_callback(char *AT_cmd)
{
	UART_Write( AT_cmd, strlen(AT_cmd) );
	delay(50);
	return 0;
}

uint8_t Issue_CMD_callback(uint8_t cmd_type)
{
	if( cmd_type == General_Cmd_ID )
	{
		GET_IOT_CmdTLV();	// 解析TLV  Parsing TLV
		printf("Receive General_Cmd: '%s'.\n", GET_IOT_TLV_Value(char*, General_Cmd_Buf) );
		return 0; 	// 返回0代表命令执行成功，其它数值表示执行失败。
		// Return zero means execute command succeed and other means failed.
	}
	return 1;
}



void main(void)
{
	uint8_t ret, coap_type, message_id;
	uint8_t send_buf[1024]={0}, rec_buf[512]={0};
	size_t rec_length=0, message_length = 0;
	
	// ------------------ 初始化示例  Initialization example --------------- //
	PUT_NB_CommandCallback(NB_serialport_callback);
	ret = INIT_NB_IOT();
	if(ret) printf("Initialization error code: %d.\n", ret);
	
	PUT_IOT_CmdCallback(General_Cmd_ID, Issue_CMD_callback);
	
	PUT_IOT_BetteryLevel( 90 );
	PUT_IOT_SignalStrength( 78 );
	PUT_IOT_TimeStamp( 0x2019010100001354 );
	PUT_IOT_TLV_Value(General_Message_Buf, "Hello_world" );
	
	// ------------------ 发送消息示例  Send message example --------------- //
	GET_NB_SendMessage( send_buf, &message_length, CMT_Message, General_Message_ID );
	if( message_length <1024 )	// 如果获取消息成功。 If get message successed.
	{
		NB_serialport_callback(send_buf);
		delay( 700 );	// 在BC26-NB模组上发送一条消息的流程需要大约700毫秒。
		// The process of sending a message on the BC26-NB module takes approximately 700 milliseconds.
		if( POST_NB_Command( CHECK_SendMessage_Failed, NULL) )
		{
			printf(" Send message failed, and resend it. \n");
			NB_serialport_callback(send_buf);
		}
		memset(send_buf, 0, 1024);
	}
	
	
	while(1) {
		
	// ------------ 接收消息与回复示例  Receive message and response example ------------- //
		while( rec_length == 0 ) {
			rec_length = UART_Read(rec_buf,  512) ;
		}
		ACK_NB_Received(rec_buf, &coap_type, &message_id );
		
		if( coap_type == CMT_CMD )	// Receive command need to response.
		{
			GET_NB_SendMessage( send_buf, &message_length, CMT_CMD_ACK,  message_id );
			NB_serialport_callback(send_buf);
			delay( 700 );
			memset(send_buf, 0, 1024);
		}
		else if( coap_type == CMT_Message_ACK ) {
			printf("Received message[%d] response.\n", message_id);
		}
		memset(rec_buf, 0, 512);
	}
}

