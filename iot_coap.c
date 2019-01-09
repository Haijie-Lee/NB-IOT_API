#include "iot_coap.h"


// ——————————————    IOT 上报消息      —————————————— //

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
	//uint8_t* message;	// 除去指针，不考虑对齐，其它数据长度 53
	//uint8_t checksum;
}__attribute__((__packed__)) gl_IOT_Message;


// ——————————————    IOT 上报消息回复      —————————————— //

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
}__attribute__((__packed__)) gl_Message_ACK;


// ——————————————    IOT 下发命令    —————————————— //

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
}__attribute__((__packed__)) gl_IOT_CMD;


// ——————————————    IOT 下发命令回复    —————————————— //

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
}__attribute__((__packed__)) gl_CMD_ACK;


// ——————————————   函数源   —————————————— //

void INIT_IOT(const char* imei, const char* imsi)
{
	int i;
	
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
	
	// Initialize IMEI/IMSI
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
	
	// Initialize TLV
	for(i=0; i<TLV_MAX; i++) {
		if( (gl_TLV_table[i].flag & 0x0F) == 0x08)
			gl_TLV_table[i].value = malloc(8);
	}
	
	#ifdef USE_QUEUE
	LOGI("Use message queue.\n");
	#endif
	
	i=sizeof(gl_IOT_Message);
	if(i != MESSAGE_HEAD_LEN)
	LOGE("Init IOT finished, waring: sizeof gl_IOT_Message = %d, head_length = %d.\n", i, MESSAGE_HEAD_LEN);
	else LOGI("Init  IOT finished.\n");
	
}


uint8_t PUT_IOT_CmdCallback(uint8_t cmd_type, CMD_Callback_t function)
{
	int i;
	
	for(i=0; i<CMD_MAX; i++) {
		if(cmd_type == gl_ACK_table[i].cmd_type) break;
	}
	if(i == CMD_MAX) {
		LOGE("PUT_IOT_CmdCallback error: ‘cmd_type’ not found.\n");
		return 1;
	}
	
	gl_ACK_table[i].function = function;
	return 0;
}


uint8_t PUT_IOT_MessagePeriod(uint8_t message_type, uint16_t period)
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

uint32_t GET_IOT_MessagePeriod(uint8_t message_type)
{
	int i;
	
	for(i=0; i<MESSAGE_MAX; i++) {
		if(message_type == gl_Message_table[i].message_type) break;
	}
	if(i == MESSAGE_MAX) {
		LOGE("POST_IOT_Message_Period error: ‘message_type’ not found.\n");
		return 1;
	}
	
	return gl_Message_table[i].period;
}

uint8_t PUT_IOT_TLV_Value(uint8_t tlv_index, void* value)
{
	uint8_t *buf1, *buf2, i, temp;
	
	if(tlv_index > TLV_MAX) {
		LOGE("PUT_IOT_TLV_value error: TLV_index out of range.\n");
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
	else {
		gl_TLV_table[tlv_index].value = value;
	}
	
	return gl_TLV_table[tlv_index].type;
}


void PUT_IOT_BetteryLevel(uint8_t level)
{
	if( level > 100) {
		LOGE("PUT_IOT_BetteryLevel waring: level shouldn't over than 100%%.\n");
		gl_IOT_Message.bettery_level = level%100;
		return;
	}
	gl_IOT_Message.bettery_level = level;
}

void PUT_IOT_SignalStrength(uint32_t strength)
{
	gl_IOT_Message.signal_strength = host2NetInt32(strength);
}

void PUT_IOT_TimeStamp(uint64_t timestamp)
{
	gl_IOT_Message.timestamp = host2NetInt64(timestamp);
}


uint8_t checksum(char* buffer, uint32_t len)
{
	uint16_t i;
	uint8_t temp=0;
	for(i=0; i<len; i++)
	{
		temp += buffer[i];
	}
	return temp;
}


uint16_t GET_IOT_SendMessage(uint8_t message_type, uint8_t *out_buf)
{
	uint8_t *buf;
	uint16_t message_index, length, i, j, temp;
	static uint16_t get_message_sequence = 0;
	IOT_TlvCollector_t p;
	
	if(message_type > 0) {
		for(i=0; i<MESSAGE_MAX; i++) {
			if(message_type == gl_Message_table[i].message_type) break;
		}
		if(i == MESSAGE_MAX) {
			LOGE("GET_IOT_SendMessage error: ‘message_type’ not found.\n");
			return 0;
		}
		else message_index = i;
	}
	else {
		message_index = get_message_sequence++;
		if (get_message_sequence >= MESSAGE_MAX)
			get_message_sequence = 0;
	}
	//LOGE("Message index: %d \n", message_index);

	if( gl_Message_table[message_index].send_ack ) {
		LOGE("GET_IOT_SendMessage warning: Message[%d] had not been ack.\n",gl_Message_table[message_index].message_type);
	}
	length = MESSAGE_HEAD_LEN;
	
	for(i=1; i<=gl_Message_table[message_index].tlv_use[0]; i++)
	{
		
		p = gl_TLV_table[ gl_Message_table[message_index].tlv_use[i] ];
		//LOGE("type :%d, length: %d, value:%p \n", p.type, p.flag, p.value);
		temp = p.flag&0x0F;
		
		out_buf[length++] = p.type;	// copy type of tlv
		
		if( temp<=4 ) {
			*(uint16_t *)(out_buf+length) = host2NetInt16( temp );
			buf = (uint8_t *)(&p.value);
			for(j=0; j<temp; j++)
				out_buf[ length + j ] = buf[j];
		}
		else if(temp == 8) {
			*(uint16_t *)(out_buf+length) = host2NetInt16( temp );
			*(uint64_t *)(out_buf + length) = *(uint64_t *)p.value;
		}else {
			if( p.value != NULL ) {
				temp = strlen(p.value);
				*(uint16_t *)(out_buf+length) = host2NetInt16( temp );
				memcpy( out_buf+length+2, p.value,  temp );
			}else {
				*(uint16_t *)(out_buf+length) = 0;
				temp = 0;
			}
		}
		length = length+temp+2;
	}
	
	//LOGE("length: %d \n", length);
	//*( (uint16_t*)(out_buf+MESSAGE_HEAD_LEN) ) = length - MESSAGE_HEAD_LEN-2;	// message_length
	gl_IOT_Message.message_length = host2NetInt16( length - MESSAGE_HEAD_LEN );
	gl_IOT_Message.length = host2NetInt16( length - 4 + 1 );	// coap length
	gl_IOT_Message.dtag = host2NetInt16( net2HostInt16(gl_IOT_Message.dtag) +1);
	gl_IOT_Message.message_type = gl_Message_table[message_index].message_type;
	*(struct IOT_Message_t *)out_buf = gl_IOT_Message;
	
	out_buf[length] = checksum(out_buf, length);
	gl_Message_table[message_index].send_ack = 1;
	return length+1;
}


uint8_t ACK_IOT_MessageACK(uint8_t message_type)
{
	int i;
	for(i=0; i<MESSAGE_MAX; i++) {
		if(message_type == gl_Message_table[i].message_type) break;
	}
	if(i == MESSAGE_MAX) {
		LOGE("GET_IOT_SendMessage error: ‘message_type’ not found.\n");
		return 1;
	}
	
	gl_Message_table[i].send_ack = 0;
	return 0;
}

uint8_t GET_IOT_CmdAck(uint8_t cmd_type, uint8_t* out_buf)
{
	uint8_t i, j, temp, index, *tlv_use=NULL, *tlv;
	uint16_t length;
	
	for(i=0; i<CMD_MAX; i++) {
		if(cmd_type == gl_ACK_table[i].cmd_type) break;
	}
	if(i == CMD_MAX) {
		LOGE("ACK_IOT_CMD error: cmd_type not found.\n");
		return 1;
	}
	if( gl_ACK_table[i].function != NULL)
	gl_ACK_table[i].callback_result = gl_ACK_table[i].function(cmd_type);
	else gl_ACK_table[i].callback_result = 1;	// default result is fail.
	gl_CMD_table[i].send_ack = 0;
	
	out_buf[ACK_HEAD_LEN] = gl_ACK_table[i].callback_result;
	length = ACK_HEAD_LEN + 1;
	tlv_use = gl_ACK_table[i].tlv_use;
	
	if( tlv_use != NULL )
	for(i=1; i<=tlv_use[0]; i++)
	{
		index = tlv_use[i];
		tlv = (uint8_t*)(&gl_TLV_table[index].value);
		out_buf[length++] = gl_TLV_table[index].type;	// copy type of tlv
		
		temp = gl_TLV_table[index].flag&0x0F;
		if( temp<=4 ) {
			*(uint16_t*)(out_buf+length) = host2NetInt16( temp );
			for(j=0; j<temp; j++)
				out_buf[ length + j ] = tlv[j];
		}
		else if(temp == 8) {
			*(uint16_t*)(out_buf+length) = host2NetInt16( temp );
			*(uint64_t *)(out_buf + length) = *(uint64_t *)gl_TLV_table[index].value;
		}else {
			if(tlv!=NULL) {
				temp = strlen(gl_TLV_table[index].value);
				memcpy( out_buf+length+2, (char*)gl_TLV_table[index].value,  temp );
			}else {
				*(uint16_t*)(out_buf+length) = 0;
				temp = 0;
			}
		}
		length = length+2+temp;
	}
	
	gl_CMD_ACK.length = host2NetInt16( length - 3 );
	gl_CMD_ACK.ack_length = host2NetInt16( length - 9);
	gl_CMD_ACK.cmd_type = cmd_type;
	gl_CMD_ACK.dtag = host2NetInt16( net2HostInt16( gl_CMD_ACK.dtag)+1 );
	*(struct IOT_CMD_ACK_t *)out_buf = gl_CMD_ACK;
	
	out_buf[length] = checksum(out_buf, length);
	
	return length+1;
}


uint8_t ACK_IOT_Received(uint8_t* rec_buf, uint16_t rec_length, uint8_t* message_type)
{
	uint16_t temp=0, ret=0;
	uint8_t i,*buf;
	
	if( checksum(rec_buf, rec_length-1) != rec_buf[rec_length-1] )
	{
		LOGE("ACK_IOT_Received error: 'rec_buf' error.\n");
		return 0;
	}
	
	if( rec_buf[1] == CMT_Message_ACK)
	{
		gl_Message_ACK = *( (struct IOT_Message_ACK_t *)rec_buf );		// copy message ack to gl_Message_Ack
		gl_Message_ACK.dtag = net2HostInt16( gl_Message_ACK.dtag );
		gl_Message_ACK.length = net2HostInt16( gl_Message_ACK.length );
		ACK_IOT_MessageACK( gl_Message_ACK.message_type );
		*message_type = gl_Message_ACK.message_type;
		ret = CMT_Message_ACK;
	}
	else if( rec_buf[1] == CMT_CMD) 
	{
		buf = (uint8_t *)(&gl_IOT_CMD);
		for(i=0; i<9; i++)		// copy cmd head to gl_IOT_CMD
			buf[i] = rec_buf[i];
		
		if(gl_IOT_CMD.tlv != NULL) free(gl_IOT_CMD.tlv);		// free old tlv space
		gl_IOT_CMD.cmd_length = net2HostInt16( gl_IOT_CMD.cmd_length );
		gl_IOT_CMD.dtag = net2HostInt16( gl_IOT_CMD.dtag );
		gl_IOT_CMD.length = net2HostInt16( gl_IOT_CMD.length );
		temp = gl_IOT_CMD.cmd_length;	// load cmd_tlv length
		if( temp > 0) {
			gl_IOT_CMD.tlv = malloc(temp);		// malloc space for new tlv
			memcpy((char *)gl_IOT_CMD.tlv, rec_buf+9, temp);	// copy new tlv to gl_IOT_CMD
		}
		gl_IOT_CMD.checksum = rec_buf[temp+9];	// copy checksum to gl_IOT_CMD
		for(i=0; i<CMD_MAX; i++) {
			if(gl_IOT_CMD.cmd_type == gl_CMD_table[i].cmd_type) break;
		}
		if(i == CMD_MAX) {
			LOGE("ACK_IOT_Received error: recived cmd_type not found.\n");
			return 1;
		}
		gl_CMD_table[i].send_ack = 1;
		ret =  CMT_CMD;
	}
	else {
		LOGE("ACK_IOT_Received error: no such coap type.\n");
	}
	
	return ret;
}


uint8_t GET_IOT_CmdTLV(void)
{
	uint8_t i, j, index, temp2, *tlv, *tlv_use=NULL, *buf;
	uint16_t length;
	
	for(i=0; i<CMD_MAX; i++)
		if(gl_IOT_CMD.cmd_type == gl_CMD_table[i].cmd_type) break;
	if(i == CMD_MAX) {
		LOGE("GET_IOT_CMD_TLV error: cmd_type not found.\n");
		return 1;
	}
	if(gl_CMD_table[i].tlv_use != NULL) {
		tlv = gl_IOT_CMD.tlv;
		tlv_use = gl_CMD_table[i].tlv_use;
		temp2=0;
	}
	
	if(tlv_use != NULL)
	for(i=1; i<=tlv_use[0]; i++)
	{
		index = tlv_use[i];
		if( gl_TLV_table[index].type != tlv[temp2] ) {
			LOGE("GET_IOT_CMD_TLV error: tlv type[%d] not equal tlv_t %d.\n", gl_TLV_table[index].type,tlv[temp2]);
			return 0;
		}
		temp2 += 3;
		
		length = gl_TLV_table[index].flag&0x0F;
		if( length <= 4)
		{
			buf = (uint8_t*)(&gl_TLV_table[index].value);
			for(j=0; j<length; j++)
				buf[ j ] = tlv[ j+temp2 ];
			temp2 += length;
		}
		else if(length == 8) {
			*( (uint64_t *)gl_TLV_table[index].value ) = *( (uint64_t *)(tlv+temp2) );
			temp2 += length;
		}
		else {
			length = *( (uint16_t*)(&tlv[temp2-2]) );
			length = net2HostInt16( length );
			if( gl_TLV_table[index].value != NULL) {
				free(gl_TLV_table[ index].value);
				gl_TLV_table[ index].value = NULL;
			}
			gl_TLV_table[index].value = malloc(length+1);
			memcpy((char *)gl_TLV_table[index].value, tlv+temp2, length);
			((uint8_t *)gl_TLV_table[index].value)[length] = 0;
			temp2 += length;
		}
		
		if (gl_TLV_table[index].flag > 0x0F )  {
		#ifdef USE_ENDIAN_CONVERT
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
		#endif
		}
	}
	else LOGE("GET_IOT_CMD_TLV warning: cmd_type[%d] not any tlv had been used,\n", gl_IOT_CMD.cmd_type);
	
	return 0;
}


