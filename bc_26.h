#ifndef __BC_26_H_
#define __BC_26_H_

#include <string.h>

#include "iot_common.h"
#include "iot_user.h"



typedef char* StringList;
typedef uint8_t (*NB_Port_Callback_t)(char *arg);


#define AT_CMD_MAX 14
extern StringList AT_CMD[AT_CMD_MAX];

#define AT_HEAD_MAX 2
extern StringList AT_Head[AT_HEAD_MAX];

#define NB_REC_MAX 12
extern StringList AT_Receive_Head[NB_REC_MAX];



// ——————————————    NB_命令列表    —————————————— //

typedef enum
{
	AT_CLOSE_ECHO = 0X01,
	AT_LOCK_Freqency,	// 锁频
	AT_SHOW_ErrorCode,
	AT_CHECK_WireLess,
	AT_CHECK_AttachedNet,
	AT_READ_SingalStrength,
	AT_READ_IMEI,
	AT_READ_IMSI,
	AT_SET_PlatformIP,
	AT_OPEN_Platform,
	CHECK_SendMessage_Failed,
}NB_CMD_List;


// ——————————————    API 函数    —————————————— //

uint8_t INIT_NB(uint8_t *out_imei, uint8_t *out_imsi, NB_Port_Callback_t function);

uint16_t GET_SendMessage_Head(uint8_t *out_buf, uint16_t message_length);
uint16_t GET_SendMessage_End(uint8_t *out_buf, uint16_t message_length);


uint8_t Execute_NB_CMD(uint8_t cmd_name, void *arg, NB_Port_Callback_t function);

uint8_t ACK_NB_REC(uint8_t *rec_buf, NB_Port_Callback_t function);




#endif
