/* *Copyright(C), Haijie Lee. 2019. AllRightsReserved.
	*FileName:  Coap流处理 模块头文件.
	*Author:  Haijie Lee.
	*Version:  1.5.
	*Date:  2019.01.22.
	*Description:  用户可通过调用此文件的接口函数完成对Coap数据流的处理。
	*Others:  用户不应随意修改此文件，除非你知道自己在做什么。
*/
#ifndef IOT_API__COMPONENT__COAP_H
#define IOT_API__COMPONENT__COAP_H


#include <stdint.h>

#include "iot_user.h"


// ——————————————    接口定义    —————————————— //

// ——————————————    接口函数    —————————————— //

/*  */

/* 宏函数： 获取指定名称的 TLV 的数值。
	说明：无。
	参数：@value_type: 用什么格式读出TLV数值；
				@index: TLV的名字（即TLV表格索引号）。
	返回值：成功返回 0，失败返回错误码。
*/
#define Get_TlvValue(value_type,index)  ( gl_TLV_table[index].flag > 0x04 ?	\
	*(value_type*)(&gl_TLV_table[index].value) : *(value_type*)gl_TLV_table[index].value )

/* 功能：初始化Coap流处理模块。
	说明：无。
	参数：@imei: IMEI字符串（ACCII字符）的指针
				@imsi: IMSI字符串（ACCII字符）的指针。
	返回值：成功返回 0，失败返回错误码。
*/
uint8_t Init_Coap(const char* imei, const char* imsi);

/* 功能：放置TLV数值到TLV表格中。
	说明：小于等于4字节的数据，使用【(void *) vlaue】方式传递数值；
				大于4字节的数据，使用【(void *)&value】方式传递数值。
	参数：@tlv_index：TLV序号，即TLV名称.；
				@value: TLV数值。
	返回值：成功返回 0，失败返回错误码。
*/
uint8_t Put_TlvValue(uint8_t tlv_index, void* value);

/* 功能：从下发命令的Coap中获取TLV到TLV表格中。
	说明：无。
	参数：无。
	返回值：成功返回 0，失败返回错误码。
*/
uint8_t Get_TlvFromCmd(void);


/* 功能：设置消息发送周期。
	说明：周期性发送消息的功能目前暂未开发，此处的数值可供用户自定义其功能。
	参数：@message_id: 消息ID；
				@period: 周期。
	返回值：成功返回 0，失败返回错误码。
*/
uint8_t Put_MessagePeriod( uint8_t message_id, uint16_t period );

/* 功能：获取消息发送周期。
	说明：无。
	参数：@message_id: 消息ID。
	返回值：成功返回 0，失败返回错误码。
*/
uint16_t Get_MessagePeriod( uint8_t message_id );

/* 功能：设置命令回调函数。
	说明：【被设置的命令回调函数】会在回复命令时被调用。
	参数：@cmd_id: 命令的ID；
				@function: 回调函数的指针（填入函数名称即可）。
	返回值：成功返回 0，失败返回错误码。
*/
uint8_t Put_CmdCallback(uint8_t cmd_id, CMD_Callback_t function );

/* 功能：设置上报消息的公共参数——电池电量
	说明：无。
	参数：@level: 电池电量（不应超过100%）。
	返回值：成功返回 0，失败返回错误码。
*/
void Put_BetteryLevel(uint8_t level);

/* 功能：设置上报消息的公共参数——电池电量
	说明：无。
	参数：@strength: 信号强度，格式由用户自定义。
	返回值：成功返回 0，失败返回错误码。
*/
void Put_SignalStrength(uint32_t strength);

/* 功能：设置上报消息的公共参数——时间戳
	说明：无。
	参数：@timestamp: 时间，格式由用户自定义。
	返回值：成功返回 0，失败返回错误码。
*/
void Put_TimeStamp(uint64_t timestamp);


/* 功能：增加一条等待发送的Coap流消息。
	说明：生成的Coap会加入发送列队等待发送，发送的时机由NB模组的模块决定。
	参数：@coap_type: Coap的类型（可选 上传消息 或 命令回复）；
				@message_id: 消息ID。
	返回值：成功返回 0，失败返回错误码。
*/
uint8_t IncrementSendCoapMessage(  uint8_t coap_type, uint8_t message_id );

/* 功能：减少一条等待处理的Coap流消息。
	说明：该函数为从接收列队中读取一条消息，并进行一些预处理。
	参数：@*coap_type: 【读出Coap类型的变量】的指针。
				@*message_id: 【读出消息ID的变量】的指针。
	返回值：成功返回 0，失败返回错误码。
*/
uint8_t DecrementReceiveCoapMessage( uint8_t *coap_type, uint8_t *message_id );

/* 功能：检查是否发送消息错误。
	说明：无。
	参数：无。
	返回值：发生错误返回1，其它返回0。
*/
uint8_t CheckSendMessageError(void);

/* 功能：检查是否接收到消息。
	说明：无。
	参数：无。
	返回值：接收到消息返回1，其它返回0。
*/
uint8_t CheckReceiveMessage(void);

// ——————————————   END   —————————————— //

#endif
