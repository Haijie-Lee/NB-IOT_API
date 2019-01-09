/*
	
*/
#include "iot_user.h"



	// --------------------- TLV 初始化 -------------------- //

/*
typedef enum
{
	
}TLV_Name_Define;		// 平台上定义的TLV名称
*/

/* flag 的格式匹配：
	分为需要转换字节序和不需要转换字节序两种格式。
	（1）高4位为1（0x1X）表示该TLV需要转换字节序，可以转换字节序的type有：
		TLV_SHORT, TLV_INT32, TLV_INT64, TLV_FLOAT, TLV_DOUBLE；
		高4位为0表示不需要转换字节序；
	（2）低4位表示数据的长度，不定长的数据用HEX字符“F”表示，
		定长的数据长度有：1，2，4，8（单位：byte）；
		其它长度视为不定长，在处理过程中通过strlen函数自动计算长度。
*/
/*
IOT_TlvCollector_t gl_TLV_table[TLV_MAX] = {
	
};
*/

	// --------------------- 上报消息初始化 -------------------- //
/*typedef enum
{
	
}Message_ID_Define;*/

/*
IOT_MessageCollector_t gl_Message_table[MESSAGE_MAX] = {
	;
};
*/

	// --------------------- 下发命令初始化 -------------------- //
/*typedef enum
{
	
}Command_ID_Define;*/

/*
IOT_CmdCollector_t gl_CMD_table[CMD_MAX] = {	// message表
	
};
*/

	// --------------------- 下发命令回复初始化 -------------------- //


/*
IOT_AckCollector_t gl_ACK_table[CMD_MAX] = {
	;
};
*/


// -----------------------------------  用户的函数  User's Function ------------------------------------------  //


/* ----------- 命令回调函数示例 ----------- 
uint8_t NBcommand_Callback_Example(char *arg)
{
	uart2_write(arg, strlen(arg));
	delay_ms(100);
	return 0;
}
*/


/*	-----------  < 用户编程指引 >  ------------
	*注： 本API不包括硬件驱动，需由用户自行提供。
	
	1. 使用此API之前，需按步骤做【必要的初始化工作】： 
		（1）设置【NB串口发送任务】到回调函数—— PUT_NB_CommandCallback() ；
		（2）初始化 —— INIT_NB_IOT() ；
		（3）设置命令回调函数—— PUT_IOT_CmdCallback()。
	2. 在【连接NB模组的串口】的接收函数中，把接收到的数据传入 ACK_NB_Received() 或
		POST_NB_Received_ToQueue() 函数中，即可接收下发命令或消息回复，和其它模组信息。
	3. 将 GET_NB_SendMessage_FromQueue()、 GET_NB_SendMessage() 传出的数据，发送到
		【连接NB模组的串口】的发送函数中，即可通过NB模组将需要发送的消息发送到平台。
	4. 初始化之后，编程仅需调用 <iot_user.h> 中声明的函数，即可完成自己想要的功能，
		具体可参考用户编程示例文件 <example.c>。
*/


void main()
{
	;
}

// --- end ---
