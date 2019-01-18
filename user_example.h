/* 
	
*/
#ifndef IOT_API__USER_EXAMPLE_H
#define IOT_API__USER_EXAMPLE_H

#include <stdint.h>

#define BC_26_V01 	0x01
#define BC_26 			0x02

// ——————————————    用户设置    —————————————— //

#define USE_ENDIAN_CONVERT 		// 字节序转换开关
#define EASYIOT_COAP_VERSION 0x01  // COAP流版本号，‘0x81’为不需要平台返回ACK
#define NB_MODEL_VERSION BC_26		// NB模组版本号: 可根据需要选择模组版本

// ——————————————    接口定义    —————————————— //

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


/* ----------------------------------------------------*/

typedef struct __attribute__((__packed__))
{
	uint8_t flag;		// 用于自动处理数据说使用的标志位，不会出现在上传数据流中
	uint8_t type;
	uint16_t length;
	void* value;
}IOT_TlvCollector_t;	// TLV表格的数据格式


typedef struct _IOT_MessageCollector_t_ 
{
	uint8_t  message_type;				// 消息ID
	uint8_t send_ack;						// 发送置1，收到回复置0
	uint16_t period;							// 上报周期（单位：s）
	uint8_t *tlv_use;							// 启用哪些TLV
	//_IOT_MessageCollector_t_* next;
}IOT_MessageCollector_t;	// Message表格的数据格式


typedef struct
{
	uint8_t cmd_type;						// 命令ID
	uint8_t send_ack;						// 接收置1，回复置0
	uint8_t *tlv_use;							// 命令TLV参数
	//CMD_Callback_t function;			// 回调函数
	//_IOT_CmdCollector_t_* next;
}IOT_CmdCollector_t;	// Command表格的数据格式


typedef struct __attribute__((__packed__))
{
	uint8_t cmd_type;						// 命令ID
	uint8_t callback_result;				// 回调函数执行结果, 0代表成功
	uint16_t ack_length;					// 回复消息的长度
	uint8_t *tlv_use;							// 命令TLV参数
	CMD_Callback_t function;			// 回调函数
}IOT_AckCollector_t;		// Command Ack 表格的数据格式

/* ----------------------------------------------------*/

#define TLV_MAX 4		// TLV的总数量
#define MESSAGE_MAX 2	// 上报消息的总数量
#define CMD_MAX 1	// 下发命令的总数量

// ——————————————    接口变量   —————————————— //

extern IOT_TlvCollector_t gl_TLV_table[TLV_MAX];
extern IOT_MessageCollector_t gl_Message_table[MESSAGE_MAX];
extern IOT_CmdCollector_t gl_CMD_table[CMD_MAX];
extern IOT_AckCollector_t gl_ACK_table[CMD_MAX];

// ——————————————    接口函数    —————————————— //



// ——————————————   END   —————————————— //

#endif
