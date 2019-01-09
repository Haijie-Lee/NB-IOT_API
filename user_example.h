/* 功能：应用编程入口库文件。
	说明：用户可以通过屏蔽功能开关而选择是否启用某些功能，
				除此之外用户不应更改其内容除非你知道自己在做什么。
	Function: 		Application programming entry library file.
	Description: 	User can choose whether to enable certain functions by blocking the Function Switch,
							and should not change otherwise content unless you know what you are doing.
*/
#ifndef __IOT_USER_H
#define __IOT_USER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define BC_26 0x02



/* ---------------------- 功能开关 Function Switch------------------------------*/

#define USE_ENDIAN_CONVERT 		// 字节序转换开关
//#define USE_QUEUE	 		// 消息列队开关
#define EASYIOT_COAP_VERSION 0x01  // COAP流版本号，‘0x81’为不需要平台返回ACK
#define NB_MODEL_VERSION BC_26		// NB模组版本号


/* ----------------------------------------------------*/

typedef enum 	// length类型 （of TLV）
{
	TLV_BYTE = 0x01,	// 1 byte
	TLV_SHORT,	// 2 byte
	TLV_INT32,	// 4 byte
	TLV_INT64,	// 8 byte
	TLV_FLOAT,	// 4 byte
	TLV_DOUBLE,	// 8 byte
	TLV_BOOL,	// 1 byte
	TLV_ENUM, // 1 byte
	TLV_STRING_ISO_8859,	// 不定长
	TLV_STRING_HEX,	// 不定长
	TLV_STRING_UTF_8,	// 不定长
}IOT_DataLength;

typedef enum  {		// IOT消息类型 （of message）
	CMT_Message = 0xF0,
	CMT_Message_ACK = 0xF1, 
	CMT_CMD= 0xF2,
	CMT_CMD_ACK = 0xF3,
	CMT_SYS_CONF_REQ = 0xF4,		// 暂时不用
	CMT_SYS_CONF_RSP = 0xF5,		// 暂时不用
	CMT_SYS_QUERY_REQ = 0xF6,	// 暂时不用
	CMT_SYS_QUERY_RSP = 0xF7		// 暂时不用
}IOT_CoapMessageType;

/* ----------------------------------------------------*/

typedef uint8_t (*CMD_Callback_t)(uint8_t cmd_type);	// 下发命令的回调函数类型


// ——————————————    IOT 数据格式   Data format   —————————————— //

typedef struct __attribute__((__packed__))
{
	uint8_t flag;		// 用于自动处理数据说使用的标志位，不会出现在上传数据流中
	uint8_t type;
	uint16_t length;
	void* value;
}IOT_TlvCollector_t;


typedef struct _IOT_MessageCollector_t_ 
{
	uint8_t  message_type;				// 消息ID
	uint8_t send_ack;						// 发送置1，收到回复置0
	uint16_t period;							// 上报周期（单位：s）
	uint8_t *tlv_use;							// 启用哪些TLV
	//_IOT_MessageCollector_t_* next;
}IOT_MessageCollector_t;


typedef struct
{
	uint8_t cmd_type;						// 命令ID
	uint8_t send_ack;						// 接收置1，回复置0
	uint8_t *tlv_use;							// 命令TLV参数
	//CMD_Callback_t function;			// 回调函数
	//_IOT_CmdCollector_t_* next;
}IOT_CmdCollector_t;


typedef struct __attribute__((__packed__))
{
	uint8_t cmd_type;						// 命令ID
	uint8_t callback_result;				// 回调函数执行结果, 0代表成功
	uint16_t ack_length;					// 回复消息的长度
	uint8_t *tlv_use;							// 命令TLV参数
	CMD_Callback_t function;			// 回调函数
}IOT_AckCollector_t;


#define TLV_MAX 4
#define CMD_MAX 1
#define MESSAGE_MAX 2

extern IOT_TlvCollector_t gl_TLV_table[TLV_MAX];
extern IOT_MessageCollector_t gl_Message_table[MESSAGE_MAX];
extern IOT_CmdCollector_t gl_CMD_table[CMD_MAX];
extern IOT_AckCollector_t gl_ACK_table[CMD_MAX];

/* ----------------------------------------------------*/



/* -----------------------------------    API 函数 Function   ------------------------------------ */

extern uint8_t INIT_NB_IOT(void);
/* 功能：	初始化 NB模组 与 COAP流处理模块。
	返回值: 成功返回0，失败返回非0。
*/


typedef uint8_t (*CMD_Callback_t)(uint8_t cmd_type);	// 下发命令的回调函数类型
extern uint8_t PUT_IOT_CmdCallback(uint8_t cmd_type, CMD_Callback_t function);
/* 功能：	设置指定ID的命令的回调函数。
	说明：	回调函数的执行结果会保存到命令回复中的执行结果中：
				0为成功，其它为失败，不设置回调函数默认为1（失败）。
	参数：	@cmd_type: 命令ID；
				@function:  回调函数。
	返回值: 成功返回0，失败返回非0。
*/

extern uint8_t GET_IOT_CmdTLV(void);
/* 功能：	解析当前下发命令中的TLV，到TLV的存储空间中。
	说明：	请在【下发命令的回调函数(CMD_Callback_t)】中调用此函数。
	参数：	@cmd_type: 命令ID；
				@function:  回调函数。
	返回值: 成功返回0，失败返回非0。
*/

extern uint8_t PUT_IOT_MessagePeriod(uint8_t message_type, uint16_t period);
extern uint32_t GET_IOT_MessagePeriod(uint8_t message_type);
/* 功能：	设置与查询上报消息的上报周期（单位不确定）。
	说明：	此数值是为【方便用户做上报周期设定】而预留的，
				在此API中并没有实际的功能，即用户可自定义其功能。
	参数：	@message_type：消息ID
				@period：上报周期
*/


extern uint8_t PUT_IOT_TLV_Value(uint8_t tlv_index, void* value);
/* 功能：	传递TLV数值，到指定的TLV中。
	参数：	@ value：4 byte及以下长度的数据,直接转换成空指针类型即可（例如 (void *) int8_arg），
								超过4 byte长度的数据使用指针传达数据（例如 (void *) & int64_arg）；
				@tlv_index: TLV索引号，用于在TLV表中查找到指定的TLV。
*/

#define GET_IOT_TLV_value(value_type, index) ( gl_TLV_table[index].flag > 0x04 ?	\
	*(value_type*)&gl_TLV_table[index].value)	:	*(value_type*)gl_TLV_table[index].value)	)
/* 功能：	从TLV的存储空间中【读取TLV数值】（宏函数）。
	参数：	@value_type: 指定读出TLV数值的数据类型（例如int、float）；
				@index: TLV索引号，用于在TLV表中查找到指定的TLV。
*/

extern void PUT_IOT_BetteryLevel(uint8_t level);	// 修改COAP流公共参数——电池电量
extern void PUT_IOT_SignalStrength(uint32_t strength); 	// 修改COAP流公共参数——信号强度
extern void PUT_IOT_TimeStamp(uint64_t timestamp); 	// 修改COAP流公共参数——系统时间


	// --------------------- 若使用列队 If open Queue Switch -------------------- //
#ifdef USE_QUEUE 	

extern uint8_t POST_NB_SendMessage_ToQueue(uint8_t coap_type, uint8_t message_type);
/* 功能：	传递指定ID的发送消息到列队中。
	说明：	列队中的数据流为原始COAP流。
	参数：	@coap_type: 选填 CMT_Message 或 CMT_CMD_ACK；
				@message_type: 发送的消息ID 或 命令回复ID。
	返回值: 成功返回0，失败返回非0。
*/

extern uint8_t GET_NB_SendMessage_FromQueue(uint8_t *out_buf, uint16_t *message_length);
/* 功能：	从列队中读取需要发送的消息到指定缓冲区中。
	说明：	缓冲区的数据可直接发送到NB模组中。
	参数：	@out_buf: 缓冲区地址；
				@buf_max_length: 缓冲区的最大长度。
	返回值: 成功返回0，失败返回非0。
*/

extern uint8_t POST_NB_Received_ToQueue(uint8_t *rec_buf, uint16_t buf_max_length);
/* 功能：	解析NB接收到的各种数据。
	说明：	若接收到的消息则存放到列队中，列队的数据流为原始的COAP流；
				若接收到其它内容，则【通过匹配事件列表内容】产生事件通知。
	参数：	@rec_buf: 接收的数据缓冲区；
				@buf_max_length: 缓冲区的最大长度。
	返回值: 成功返回0，失败返回非0。
*/

extern uint8_t ACK_NB_Received_FromQueue(uint8_t *coap_type, uint8_t *message_type);
/* 功能：	从列队中读取【接收的coap流】进行自动处理。
	说明：	若接收到命令，则在此处调用命令回调函数。
	参数：	@*coap_type: 传入变量地址用于存放coap_type；
				@*message_type: 传入变量地址用于存放message_type；
	返回值: 成功返回0，失败返回非0。
*/


	// ---------------------消息列队相关 About Message Queue -------------------- //
typedef enum {
	UploadQueue = 0x01,
	DownloadQueue,
}Queue_name_define;

uint8_t IsEmpty_Queue(uint8_t queue_name);
uint8_t IsFull_Queue(uint8_t queue_name);
/* 功能：	用于判断列队是否为空/是否为满。
	说明：	若列队为空则不应该读列队，为满则不应该写列队。
	参数：	@queue_name: 【Queue_name_define】定义的列队名。
	返回值: 否返回0，是返回1，错误返回0xFF。
*/

	// --------------------- 若不使用列队 If close Queue Switch -------------------- //
#else 

uint8_t GET_NB_SendMessage(uint8_t *out_buf, uint16_t *message_length, uint8_t coap_type, uint8_t message_type);
/* 功能：	获取一条需要发送的消息到缓冲区中。
	说明：	若接收到命令，则在此处调用命令回调函数。
	参数：	@out_buf: 缓冲区地址；
				@*message_length: 用于传出【获取到的消息的长度数值】的变量指针；
				@coap_type: 选填 CMT_Message 或 CMT_CMD_ACK；
				@message_type: 发送的消息ID 或 命令回复ID。
	返回值: 成功返回0，失败返回非0。
*/

uint8_t ACK_NB_Received(uint8_t *rec_buf, uint8_t *coap_type, uint8_t *message_type);
/* 功能：	解析NB接收到的各种数据。
	说明：	若接收到命令，则在此处调用命令回调函数；
				若接收到其它内容，则【通过匹配事件列表内容】产生事件通知。
	参数：	@*rec_buf: 接收的缓冲区地址；
				@*coap_type: 用于传出【coap类型】的变量指针；
				@*message_type: 用于传出【命令ID】或 【消息回复ID】的变量指针。
	返回值: 成功返回0，失败返回非0。
*/

#endif


uint8_t POST_NB_Command(uint8_t cmd_name, void *arg);
/* 功能：	请求执行NB模组的指令。
	说明：	此函数为回调函数，具体执行内容根据不同模组而定，
				请参考NB模块 <bc_xx.h>文件中的函数定义。
	参数：	@cmd_name: 传入 【NB_CMD_List】 定义的命令选项；
				@arg: 命令参数，用途根据命令而定。
	返回值: 成功返回0，失败返回非0。
*/


	// --------------------- NB模组与串口的连接处设置  NB-module and Serial-port junction Setting ---------------------- //
	
typedef uint8_t (*NB_Port_Callback_t)(char *arg);
/* 功能：	用于【统一指定某些NB命令的执行内容】的回调函数类型。
	说明：	例如传入【串口发送任务】（可将AT指令发送到串口）、
				【延迟函数】等，便于用户完成某些重复的操作。
	参数：	@arg: 命令对应的AT指令入口。
	返回值: 用户自定义。
*/

void PUT_NB_CommandCallback(NB_Port_Callback_t funtion);
/* 功能：	设置被回调的函数，每次调用 POST_NB_Command 函数时，
				会调用被设置的回调函数。
	说明：	请参考.C 文件中的回调函数示例。
	参数：	@funtion: 回调函数。
	返回值: 无。
*/




#include "NB_Model.h"

#endif
