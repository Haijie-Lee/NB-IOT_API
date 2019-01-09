/* 静态版本的 【IOT 数据处理模块】 特点说明：
	
	相对于动态版本的 ，静态版本的代码更精简，
	代码效率会高很多，更稳定也更不易出错；
	同时也牺牲了一些动态调整的可能性，和些许内存空间。
	
	例如：静态版本中，引入了固定长度的消息和命令的数据流buffer，
	以全局变量的形态存在于整个程序的生命周期；
	以静态数组的方式存放TLV、上传消息、下发命令的格式内容，
	初始化之后，用户不能自行更改表格内容，否则容易出错。
	
	注意：
	（1）用户在使用前需根据平台的产品注册信息，自行填写一部分静态表的内容，
			除TLV表，其它静态表中TLV的顺序需与平台TLV的顺序一致。
	（2）本模块不提供列队缓冲机制，以及多线程适配。
	（3）消息buffer需用户清空。
	（4）适用于32位环境。
*/
#ifndef __STATIC_EASYIOT_H
#define __STATIC_EASYIOT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "iot_common.h"
#include "iot_user.h"

#define STANDARD_IMEI_LENGTH 15
#define STANDARD_IMSI_LENGTH 15

#define RESULT_LEN 4
#define CMD_ACK_HEAD_LEN 12
#define MESSAGE_HEAD_LEN 52
#define ACK_HEAD_LEN 12 




// ——————————————    函数    —————————————— //

// 步骤1：初始化
	// 注：初始化会按照上方静态表中的内容申请内存，初始化后不应添加或修改静态表中的内容。
void INIT_IOT(const char* imei, const char* imsi);


// 步骤2：设置命令回调函数, 或修改消息上报周期（单位：S）。
uint8_t PUT_IOT_CmdCallback(uint8_t cmd_type, CMD_Callback_t function);
	/* 注：回调函数的执行结果会保存到命令回复中的执行结果中，
			0为成功，其它为失败，不设置回调函数默认为1（失败）。
			回调函数的参数是 cmd_type。
	*/
uint8_t PUT_IOT_MessagePeriod(uint8_t message_type, uint16_t period);
uint32_t GET_IOT_MessagePeriod(uint8_t message_type);

	//可选项：
uint8_t GET_IOT_CmdTLV(void);
	// 如需在 回调函数中 自动解析命令的内容（TLV）请调用此函数，解析结果存放在 gl_TLV_table 中。


// 步骤3：使用API函数修改上报信息的数据内容。
	
uint8_t PUT_IOT_TLV_Value(uint8_t tlv_index, void* value);
/* @ value：4 byte及以下长度的数据,直接转换成空指针类型即可（例如 (void *) int8_arg），
	 			超过4 byte长的数据使用指针传达数据（例如 (void *) & int64_arg）。
*/
void PUT_IOT_BetteryLevel(uint8_t level);
void PUT_IOT_SignalStrength(uint32_t strength);
void PUT_IOT_TimeStamp(uint64_t timestamp);


/* 步骤4：
	（1）在运行程序中调用以下函数，可形成上报消息（字符串buffer），
			再把消息通进行AT指令封装，通过过串口发送到IOT模组，完成消息上报；
	（2）在模组接收到的信息流进行拆分成单行（命令或消息ack的）数据流buffer（不包括AT指令部分），
			传送到 ACK_IOT_Received 函数进行自动处理，即可完成整个流程。
*/
uint16_t GET_IOT_SendMessage(uint8_t message_type, uint8_t* out_buf);
	/*	功能：生成上传消息的数据流buffer。
		参数：message_type：若传入0则轮流获取已添加ID的消息（每调用一次获取一条消息），
				传入消息ID则生成指定ID的的消息。
		返回值：数据buffer的有效长度，即coap流长度，若为0则表示失败。
	*/

uint8_t ACK_IOT_Received(uint8_t* rec_buf, uint16_t rec_length, uint8_t* message_type); 
	/* 功能：当NB接收到数据流时请调用ACK_IOT_Received（函数），它会调用相关函数进行自动处理。
		参数：接收数据流的字节数。
	    返回值：IOT消息类型。
	    注：	如需查看详细的解析内容，请查看 gl_IOT_CMD 或 gl_Message_ACK结构体的内容。
	*/


// 其它：
	// 计算校验和
uint8_t checksum(char* buffer, uint32_t len);
	// 当NB接收到数据流时请调用ACK_IOT_Received（函数），它会调用以下函数进行自动处理。
	// 不建议用户额外调用以下函数。
uint8_t ACK_IOT_MessageACK(uint8_t message_type);		// 成功返回0，失败返回1
uint8_t GET_IOT_CmdAck(uint8_t cmd_type, uint8_t* out_buf);



#endif
