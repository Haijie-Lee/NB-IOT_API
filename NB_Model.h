/*	【AT指令连接】 功能模块说明：
	
	功能：连接"IOT数据处理模块"与"NB模组的AT指令"，使用户能与NB模组通信。
	
	考虑到数据的高并发与单片机有限的内存可能的冲突，
	我们在此功能模块中，提供【环形列队】的缓存机制，
	故用户可根据实际需求，自行决定是否启用缓存机制。
	
	注意：
	（1）列队缓存仅是提高了单片机的缓存上限，若不及时读取，仍然可能导致数据丢失；
	（2）AT指令列表的内容，需根据NB模块型号手动更新。
*/
#ifndef __AT_command_h_
#define __AT_command_h_

#include <string.h>
#include <stdint.h>

#include "iot_coap.h"
#include "iot_common.h"
#include "iot_user.h"


#if NB_MODEL_VERSION == BC_26
	#include "bc_26.h"
#endif




uint8_t INIT_NB_IOT(void);
	// --------------------- 若使用列队 -------------------- //
#ifdef USE_QUEUE 
uint8_t POST_NB_SendMessage_ToQueue(uint8_t coap_type, uint8_t message_type);
uint8_t GET_NB_SendMessage_FromQueue(uint8_t *out_buf, uint16_t *message_length);
uint8_t POST_NB_Received_ToQueue(uint8_t *rec_buf, uint16_t buf_max_length);
uint8_t ACK_NB_Received_FromQueue(uint8_t *coap_type, uint8_t *message_type);
	// --------------------- 若不使用列队 -------------------- //
#else
uint8_t GET_NB_SendMessage(uint8_t *out_buf, uint16_t *message_length, uint8_t coap_type, uint8_t message_type);
uint8_t ACK_NB_Received(uint8_t *rec_buf, uint8_t *coap_type, uint8_t *message_type);
#endif

uint8_t POST_NB_Command(uint8_t cmd_name, void *arg);
void PUT_NB_CommandCallback(NB_Port_Callback_t funtion);


#endif
