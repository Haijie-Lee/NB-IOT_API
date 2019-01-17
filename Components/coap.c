/* 
	
*/
#include "iot_user.h"

#include "coap.h"
#include "BC26_Module.h"
#include "common.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


// ——————————————    本地设置    —————————————— //

/* 每一条消息的最大长度，该值应由IOT平台决定 */
#define COAP_LENGTH_MAX 512

// ——————————————    本地定义    —————————————— //

#define STANDARD_IMEI_LENGTH 15
#define STANDARD_IMSI_LENGTH 15

#define RESULT_LEN 4
#define CMD_HEAD_LEN 9
#define CMD_ACK_HEAD_LEN 12
#define MESSAGE_HEAD_LEN 52
#define ACK_HEAD_LEN 12 

// ——————————————    本地变量   —————————————— //

struct IOT_Message_t //52 Byte
{
	uint8_t version;
	uint8_t type;
	uint16_t length;
	uint16_t dtag;
	char bettery_level;
	uint32_t signal_strength;
	char imei[15];
	char imsi[15];
	uint64_t timestamp;
	uint8_t message_type;
	uint16_t message_length;
	//uint8_t* message;
	//uint8_t checksum;
}__attribute__((__packed__)) gl_IOT_Message;		/* IOT 上报消息 */


struct IOT_Message_ACK_t  // 8 byte
{
	uint8_t version;
	uint8_t type;
	uint16_t length;
	uint8_t ack;
	uint16_t dtag;
	uint8_t message_type;
// uint8_t* reserved;
	uint8_t checksum;
}__attribute__((__packed__)) gl_Message_ACK;	 /* IOT 上报消息回复 */


struct _IOT_CMD_t_
{
	uint8_t version;
	uint8_t type;
	uint16_t length;
	uint16_t dtag;
	uint8_t cmd_type;
	uint16_t cmd_length;
	uint8_t *tlv;
	uint8_t checksum;
}__attribute__((__packed__)) gl_IOT_CMD;	 	/* IOT 下发命令 */


struct IOT_CMD_ACK_t // 12 byte
{
	uint8_t version;
	uint8_t type;
	uint16_t length;
	uint16_t dtag;
	uint8_t cmd_type;
	uint16_t ack_length;
	uint8_t result_type; 	// default is 0
	uint16_t result_length;	// default is 1
	//uint8_t result;		// zero is successful , other means fail.
	//uint8_t* arg;			// other arg by tlv
	//uint8_t checksum;
}__attribute__((__packed__)) gl_CMD_ACK;	/* IOT 下发命令回复  */

// ——————————————    本地函数声明    —————————————— //

uint8_t check_sum(char* buffer, uint32_t len);
uint8_t Process_MessageACK( uint8_t *rec_buf, uint8_t message_type );
uint8_t Process_Cmd( uint8_t *rec_buf, uint8_t message_type );

// ——————————————    接口函数源    —————————————— //

uint8_t Init_Coap(const char* imei, const char* imsi)
{
	size_t i, data_length;
	
	// Initialize CoapHead.
	gl_IOT_Message.version = EASYIOT_COAP_VERSION;
	gl_IOT_Message.type = CMT_Message;
	gl_IOT_Message.length = 0;
	gl_IOT_Message.dtag = 0;
	gl_IOT_Message.bettery_level = 0;
	gl_IOT_Message.signal_strength = 0;
	gl_IOT_Message.timestamp = 0;
	
	gl_CMD_ACK.version = EASYIOT_COAP_VERSION;
	gl_CMD_ACK.type = CMT_CMD_ACK;
	gl_CMD_ACK.length = 0;
	gl_CMD_ACK.dtag = 0;
	gl_CMD_ACK.result_type = 0;
	gl_CMD_ACK.result_length = 0x0100;
	
	// Initialize IMEI/IMSI.
	if (strlen(imei) != STANDARD_IMEI_LENGTH) {
		LOGE("INIT_IOT_ERROR: IMEI length Error.\n");
	}else {
		for(i=0; i<STANDARD_IMEI_LENGTH; i++) {
			gl_IOT_Message.imei[i] = (imei[i]-48) | 0x30; 	// convert to ISO-8859-1 string
		}
	}
	if (strlen(imsi) != STANDARD_IMSI_LENGTH) {
		LOGE("INIT_IOT_ERROR: IMSI length Error.\n");
	}else {
		for(i=0; i<STANDARD_IMSI_LENGTH; i++) {
			gl_IOT_Message.imsi[i] = (imsi[i]-48) | 0x30; 	// convert to ISO-8859-1 string
		}
	}
	
	// Initialize TLV.
	for(i=0; i<TLV_MAX; i++) {
		if( (gl_TLV_table[i].flag & 0x0F) == 0x08)
			gl_TLV_table[i].value = malloc(8);
	}
	
	#ifdef USE_QUEUE
	LOGI("Use message queue.\n");
	#endif
	
	data_length = sizeof(gl_IOT_Message);
	if( data_length != MESSAGE_HEAD_LEN ) {
		LOGE("Init IOT finished, waring: sizeof gl_IOT_Message = %d, head_length = %d.\n", data_length, MESSAGE_HEAD_LEN);
		return 1;
	}
	else {
		LOGI("Init  IOT finished.\n");
		return 0;
	}
}


uint8_t Put_TlvValue(uint8_t tlv_index, void* value)
{
	uint8_t *buf1, *buf2, i, temp;
	
	if(tlv_index > TLV_MAX) {
		LOGE("Put_TlvValue error: TLV_index out of range.\n");
		return 0;
	}else if(value == NULL) {
		LOGE("Put_TlvValue error:  not value.\n");
		return 0;
	}
	
	if(gl_TLV_table[tlv_index].flag > 0x0F) 	// means need to be convert
	{
	#ifdef USE_ENDIAN_CONVERT
		switch( gl_TLV_table[tlv_index].length ) {
			case TLV_SHORT:  *(uint16_t *)(&gl_TLV_table[tlv_index].value) = host2NetInt16( *(uint16_t*)&value );
						break;
			case TLV_INT32: *(uint32_t *)(&gl_TLV_table[tlv_index].value) = host2NetInt32( *(uint32_t*)&value );
						break;
			case TLV_INT64: *(uint64_t *)gl_TLV_table[tlv_index].value = host2NetInt64( *((uint64_t*)value) );
						break;
			case TLV_FLOAT: *(float *)(&gl_TLV_table[tlv_index].value) = host2NetFloat( *(float*)&value );
						break;
			case TLV_DOUBLE: *(double *)gl_TLV_table[tlv_index].value = host2NetDouble( *(double*)value );
						break;
		}
	#else
		temp = gl_TLV_table[tlv_index].flag&0x0F;
		if( temp <= 4 ) {
			buf1 = (uint8_t *)&gl_TLV_table[tlv_index].value;
			buf2 = (uint8_t *)&value;
		}else {
			buf1 = (uint8_t *)gl_TLV_table[tlv_index].value;
			buf2 = (uint8_t *)value;
		}
		for(i=0; i<temp; i++)
			buf1[i] = buf2[i];
	#endif
	}
	else if(gl_TLV_table[tlv_index].flag == 0x0F && value == NULL)
	{
		free(gl_TLV_table[tlv_index].value);
		gl_TLV_table[tlv_index].value = NULL;
	}
	else if(gl_TLV_table[tlv_index].flag > 0x04)
	{
		gl_TLV_table[tlv_index].value = (void *)&value;
	}
	else {
		#ifdef USE_ENDIAN_CONVERT
		gl_TLV_table[tlv_index].value = value;
		#else
		/*  大端序的数据起始位置与小端序的相反，需要移动数据 */
		temp = gl_TLV_table[tlv_index].flag&0x0F;
		buf1 = (uint8_t *)&gl_TLV_table[tlv_index].value;
		buf2 = (uint8_t *)value;
		for( i = temp-1;  i >= 0;  i-- )
			buf1[i] = buf2[ sizeof(void *) - i ]; 
		#endif
	}
	
	return gl_TLV_table[tlv_index].type;
}


uint8_t Get_TlvFromCmd(void)
{
	uint8_t i, j, index , *tlv, *tlv_use=NULL, *buf;
	uint16_t length, subscript;
	
	for(i=0; i<CMD_MAX; i++)
		if(gl_IOT_CMD.cmd_type == gl_CMD_table[i].cmd_type) break;
	if(i == CMD_MAX) {
		LOGE("Get_TlvFromCmd error: cmd_type not found.\n");
		return 1;
	}
	if(gl_CMD_table[i].tlv_use != NULL) {
		tlv = gl_IOT_CMD.tlv;
		tlv_use = gl_CMD_table[i].tlv_use;
		subscript=0;
	}
	
	if(tlv_use != NULL)
	for(i=1; i<=tlv_use[0]; i++)
	{
		index = tlv_use[i];
		if( gl_TLV_table[index].type != tlv[subscript] ) {
			LOGE("Get_TlvFromCmd error: tlv type[%d] not equal tlv_t %d.\n", gl_TLV_table[index].type,tlv[subscript]);
			return 2;
		}
		subscript += 3;
		
		length = gl_TLV_table[index].flag&0x0F;
		if( length <= 4 )
		{
			buf = (uint8_t*)( &gl_TLV_table[index].value );
			for( j=0 ; j < length; j++ )
				buf[ j ] = tlv[ j+subscript ];
			subscript += length;
		}
		else if( length == 8 )
		{
			*( (uint64_t *)gl_TLV_table[index].value ) = *( (uint64_t *)(tlv+subscript) );
			subscript += length;
		}
		else {
			length = *(uint16_t *)( &tlv[subscript-2] ) ;
		#ifdef USE_ENDIAN_CONVERT
			length = net2HostInt16( length );
		#endif
			if( gl_TLV_table[index].value != NULL) {
				free(gl_TLV_table[ index].value);
				gl_TLV_table[ index].value = NULL;
			}
			gl_TLV_table[index].value = malloc(length+1);
			if( gl_TLV_table[index].value == NULL ) {
				LOGE("Get_TlvFromCmd error: tlv type[%d] malloc value size[%d] failed.\n", gl_TLV_table[index].type, length+1 );
				return 3;
			}
			buf = gl_TLV_table[index].value;
			buf[length] = '\0';
			memcpy( gl_TLV_table[index].value, tlv+subscript, length );
			subscript += length;
		}
		
		#ifdef USE_ENDIAN_CONVERT
		if (gl_TLV_table[index].flag > 0x0F )
		{
			switch(gl_TLV_table[index].length)
			{
				case TLV_SHORT: *(uint16_t *)(&gl_TLV_table[index].value) = net2HostInt16( *(uint16_t*)(&gl_TLV_table[index].value) );
					break;
				case TLV_INT32: *(uint32_t*)(&gl_TLV_table[index].value) = net2HostInt32( *(uint32_t*)(&gl_TLV_table[index].value) );
					break;
				case TLV_INT64: *(uint64_t *)gl_TLV_table[index].value = net2HostInt64( *(uint64_t *)gl_TLV_table[index].value );
					break;
				case TLV_FLOAT: *(float*)(&gl_TLV_table[index].value) = net2HostInt16( *(float*)(&gl_TLV_table[index].value) );
					break;
				case TLV_DOUBLE: *(double *)gl_TLV_table[index].value = net2HostInt16( *(double *)gl_TLV_table[index].value );
					break;
			}
		}
		#endif
	}
	else {
		LOGI("GET_IOT_CMD_TLV warning: cmd_type[%d] not any TLV had been used,\n", gl_IOT_CMD.cmd_type);
	}
	return 0;
}

uint8_t Put_MessagePeriod( uint8_t message_type, uint16_t period )
{
	int i;
	
	for(i=0; i<MESSAGE_MAX; i++) {
		if(message_type == gl_Message_table[i].message_type) break;
	}
	if(i == MESSAGE_MAX) {
		LOGE("POST_IOT_Message_Period error: ‘message_type’ not found.\n");
		return 1;
	}
	
	gl_Message_table[i].period = period;
	return 0;
}


uint16_t Get_MessagePeriod(uint8_t message_type)
{
	int i;
	
	for(i=0; i<MESSAGE_MAX; i++) {
		if(message_type == gl_Message_table[i].message_type)
			break;
	}
	if(i == MESSAGE_MAX) {
		LOGE("POST_IOT_Message_Period error: ‘message_type’ not found.\n");
		return 1;
	}
	
	return gl_Message_table[i].period;
}


void Put_BetteryLevel(uint8_t level)
{
	if( level > 100) {
		LOGI("PUT_IOT_BetteryLevel waring: level shouldn't over than 100%%.\n");
		gl_IOT_Message.bettery_level = level%100;
		return ;
	}
	gl_IOT_Message.bettery_level = level;
}


void Put_SignalStrength(uint32_t strength)
{
	#ifdef USE_ENDIAN_CONVERT
	gl_IOT_Message.signal_strength = host2NetInt32(strength);
	#else
	gl_IOT_Message.signal_strength = strength;
	#endif
}


void Put_TimeStamp(uint64_t timestamp)
{
	#ifdef USE_ENDIAN_CONVERT
	gl_IOT_Message.timestamp = host2NetInt64( timestamp );
	#else
	gl_IOT_Message.timestamp = timestamp;
	#endif
}


uint8_t IncrementSendCoapMessage(  uint8_t coap_type, uint8_t message_id )
{
	uint8_t ret, *buf, buf_coap[COAP_LENGTH_MAX]={0};
	uint16_t message_index, length, i, j, data_length;
	static uint16_t get_message_sequence = 0;
	IOT_TlvCollector_t p;
	
	if( coap_type != CMT_Message || coap_type !=  CMT_CMD_ACK ) {
		LOGE("IncrementSendCoapMessage error: coap_type not exist.\n", message_id);
		return 0;
	}
	if( message_id > 0 ) {
		for( i=0; i<MESSAGE_MAX; i++ ) {
			if(message_id == gl_Message_table[i].message_type) break;
		}
		if(i == MESSAGE_MAX) {
			LOGE("IncrementSendCoapMessage error: message_id[] not found.\n", message_id);
			return 0;
		}
		else 
			message_index = i;
	}
	else {
		message_index = get_message_sequence++;
		if (get_message_sequence >= MESSAGE_MAX)
			get_message_sequence = 0;
	}

	if( gl_Message_table[message_index].send_ack ) {
		LOGI("GET_IOT_SendMessage warning: Message[%d] had not been ack.\n",gl_Message_table[message_index].message_type);
	}
	length = MESSAGE_HEAD_LEN;
	
	for( i=1; i<=gl_Message_table[message_index].tlv_use[0]; i++ )
	{
		p = gl_TLV_table[ gl_Message_table[message_index].tlv_use[i] ];
		//LOGE("type :%d, length: %d, value:%p \n", p.type, p.flag, p.value);
		data_length = p.flag&0x0F;
		
		buf_coap[length++] = p.type;	// copy type of tlv
		
		if( data_length<=4 ) {
			#ifdef USE_ENDIAN_CONVERT
			*(uint16_t *)( buf_coap + length ) = host2NetInt16( data_length );
			#endif
			buf = (uint8_t *)(&p.value);
			for(j=0; j<data_length; j++)
				buf_coap[ length + j ] = buf[j];
		}
		else if(data_length == 8) {
			#ifdef USE_ENDIAN_CONVERT
			*(uint16_t *)( buf_coap + length ) = host2NetInt16( data_length );
			#endif
			*(uint64_t *)( buf_coap + length ) = *(uint64_t *)p.value;
		}else {
			if( p.value != NULL ) {
				data_length = strlen(p.value);
				#ifdef USE_ENDIAN_CONVERT
				*(uint16_t *)(buf_coap+length) = host2NetInt16( data_length );
				#endif
				memcpy( buf_coap+length+2, p.value,  data_length );
			}else {
				*(uint16_t *)(buf_coap+length) = 0;
				data_length = 0;
			}
		}
		length = length+data_length+2;
	}
	
	#ifdef USE_ENDIAN_CONVERT
	gl_IOT_Message.message_length = host2NetInt16( length - MESSAGE_HEAD_LEN );
	gl_IOT_Message.length = host2NetInt16( length - 4 + 1 );	// coap length
	gl_IOT_Message.dtag = host2NetInt16( net2HostInt16(gl_IOT_Message.dtag) +1);
	#else
	gl_IOT_Message.message_length = length - MESSAGE_HEAD_LEN;
	gl_IOT_Message.length = length - 4 + 1;	// coap length
	gl_IOT_Message.dtag = gl_IOT_Message.dtag +1;
	#endif
	
	gl_IOT_Message.message_type = gl_Message_table[message_index].message_type;
	*(struct IOT_Message_t *)buf_coap = gl_IOT_Message;
	
	buf_coap[length] = check_sum( buf_coap, length );
	gl_Message_table[message_index].send_ack = 1;
	length += 1;
	
	ret = SendMessage( buf_coap, length );
	if( ret ) {
		LOGE(" IncrementSendCoapMessage error: send message failed[%d].\n", ret );
		return 0;
	}
	
	return length;
}


uint8_t DecrementReceiveCoapMessage( uint8_t *coap_type, uint8_t *message_id )
{
	uint16_t data_length=0;
	uint8_t i,*buf, ret=0, rec_buf[COAP_LENGTH_MAX]={0};
	
	if( coap_type == NULL || message_id == NULL ) {
		LOGE("DecrementReceiveCoapMessage error: coap_type or message_id can't be empty.\n", ret);
		return 1;
	}
	ret = ReceiveMessage( rec_buf, &data_length );
	if( ret ) {
		LOGE("DecrementReceiveCoapMessage error[%d]: can not receive message.\n", ret);
		return 2;
	}
	if( check_sum(rec_buf, data_length-1) != rec_buf[data_length-1] )
	{
		LOGE("DecrementReceiveCoapMessage error: check sum error.\n");
		return 3;
	}
	
	if( rec_buf[1] == CMT_Message_ACK)
	{
		ret = Process_MessageACK( rec_buf, gl_Message_ACK.message_type );
		if( ret ) {
			LOGE("DecrementReceiveCoapMessage error: recived message_type not found..\n");
			return 4;
		}
		*message_id = gl_Message_ACK.message_type;
		*coap_type = CMT_Message_ACK;
	}
	else if( rec_buf[1] == CMT_CMD) 
	{
		ret = Process_Cmd( rec_buf, gl_Message_ACK.message_type );
		if( ret>1 ) {
			LOGE("DecrementReceiveCoapMessage error: recived message_type not found..\n");
			return 4;
		}
		*coap_type = CMT_CMD;
		*message_id = gl_IOT_CMD.cmd_type;
	}
	else {
		LOGE("DecrementReceiveCoapMessage error: no such coap type.\n");
		return 1;
	}
	
	return 0;
}


uint8_t CheckSendMessageError(void)
{
	uint8_t ret;
	ret = WriteData_ToSerialPort( void );
	if( ret > 1 ) {
		LOGE("WriteData_ToSerialPort function error[%d].\n", ret);
	}
	ret = IF_SendMessageError(void);
	return ret;
}


uint8_t CheckReceiveMessage(void)
{
	uint8_t ret;
	ret = ReadData_FromSerialPort();
	if( ret > 1 ) {
		LOGE("ReadData_FromSerialPort function error[%d].\n", ret);
	}
	ret = IF_ReceiveMessage();
	return ret;
}


// ——————————————    本地函数源    —————————————— //

uint8_t check_sum(char* buffer, uint32_t len)
{
	uint16_t i;
	uint8_t temp=0;
	for(i=0; i<len; i++)
	{
		temp += buffer[i];
	}
	return temp;
}


uint8_t Process_MessageACK( uint8_t *rec_buf, uint8_t message_type )
{
	int i;
	for(i=0; i<MESSAGE_MAX; i++) {
		if(message_type == gl_Message_table[i].message_type) break;
	}
	if(i == MESSAGE_MAX) {
		LOGE("Process_MessageACK error: ‘message_type’ not found.\n");
		return 1;
	}
	
	gl_Message_ACK = *( (struct IOT_Message_ACK_t *)rec_buf );		// copy message ack to gl_Message_Ack
	#ifdef USE_ENDIAN_CONVERT
	gl_Message_ACK.dtag = net2HostInt16( gl_Message_ACK.dtag );
	gl_Message_ACK.length = net2HostInt16( gl_Message_ACK.length );
	#endif
	gl_Message_table[i].send_ack = 0;
	return 0;
}


uint8_t Process_Cmd( uint8_t *rec_buf, uint8_t message_type )
{
	uint8_t *buf;
	uint16_t i, data_length;
	
	buf = (uint8_t *)(&gl_IOT_CMD);
	for( i=0; i<CMD_HEAD_LEN; i++ )		// copy cmd head to gl_IOT_CMD
		buf[i] = rec_buf[i];
	if( gl_IOT_CMD.tlv != NULL )
		free(gl_IOT_CMD.tlv);		// free old tlv space
	#ifdef USE_ENDIAN_CONVERT
	gl_IOT_CMD.cmd_length = net2HostInt16( gl_IOT_CMD.cmd_length );
	gl_IOT_CMD.dtag = net2HostInt16( gl_IOT_CMD.dtag );
	gl_IOT_CMD.length = net2HostInt16( gl_IOT_CMD.length );
	#endif
	data_length = gl_IOT_CMD.cmd_length;	// load cmd_tlv length
	if( data_length > 0) {
		gl_IOT_CMD.tlv = malloc( data_length+1 );		// malloc space for new tlv
		if( gl_IOT_CMD.tlv == NULL ) {
			LOGE("Process_Cmd error: command tlv malloc failed.\n");
			return 1;
		}
		memcpy( gl_IOT_CMD.tlv, rec_buf+CMD_HEAD_LEN, data_length );	// copy new tlv to gl_IOT_CMD
		buf = gl_IOT_CMD.tlv;
		buf[data_length] = '\0';
	}
	gl_IOT_CMD.checksum = rec_buf[ data_length+CMD_HEAD_LEN ];	// copy checksum to gl_IOT_CMD
	
	for( i=0; i<CMD_MAX; i++) {
		if( gl_IOT_CMD.cmd_type == gl_CMD_table[i].cmd_type ) break;
	}
	if( i == CMD_MAX ) {
		return 2;
	}
	gl_CMD_table[i].send_ack = 1;
	
	return 0;
}
