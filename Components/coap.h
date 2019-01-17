/* 
	
*/
#ifndef IOT_API__COMPONENT__COAP_H
#define IOT_API__COMPONENT__COAP_H

#include <stdint.h>



// ——————————————    接口定义    —————————————— //


// ——————————————    接口函数    —————————————— //

/*  宏函数： 获取指定名称的 TLV 数值 */
#define Get_TlvValue( value_type, index ) 	( gl_TLV_table[index].flag > 0x04 ?	\
	*(value_type*)&gl_TLV_table[index].value)	:	*(value_type*)gl_TLV_table[index].value)	)

uint8_t Init_Coap(const char* imei, const char* imsi);
uint8_t Put_TlvValue(uint8_t tlv_index, void* value);
uint8_t Get_TlvFromCmd(void);

uint8_t Put_MessagePeriod( uint8_t message_type, uint16_t period );
uint16_t Get_MessagePeriod(uint8_t message_type);
void Put_BetteryLevel(uint8_t level);
void Put_SignalStrength(uint32_t strength);
void Put_TimeStamp(uint64_t timestamp);

uint8_t IncrementSendCoapMessage(  uint8_t coap_type, uint8_t message_id );
uint8_t DecrementReceiveCoapMessage( uint8_t *coap_type, uint8_t *message_id );
uint8_t CheckSendMessageError(void);
uint8_t CheckReceiveMessage(void);

// ——————————————   END   —————————————— //

#endif
