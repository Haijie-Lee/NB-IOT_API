#include "NB_Model.h"



// ——————————————    本地变量    —————————————— //

uint8_t coap_type_temp;
uint8_t message_type_temp;
NB_Port_Callback_t gl_CommandCallback;

// ——————————————    本地函数    —————————————— //

uint8_t received_callback(char *input);
uint8_t received2queue_callback(char *input);



// ——————————————    API函数    —————————————— //

uint8_t INIT_NB_IOT(void)
{
	uint8_t temp, imei[16]={0}, imsi[16]={0};
	
	temp = INIT_NB(imei, imsi, gl_CommandCallback);
	if(temp) return temp;
	
	INIT_IOT(imei, imsi);
	return 0;
}

	// --------------------- 若使用列队 -------------------- //
#ifdef USE_QUEUE 

uint8_t POST_NB_SendMessage_ToQueue(uint8_t coap_type, uint8_t message_type)
{
	size_t temp;
	uint8_t buf_temp[512] = {0};

	
	if( coap_type == CMT_Message ) {
		temp = GET_IOT_SendMessage(message_type, buf_temp);
	}else if( coap_type == CMT_CMD_ACK ) {
		temp = GET_IOT_CmdAck(message_type, buf_temp);
	}else {
		LOGE("POST_NB_SendMessage_ToQueue error: Not such coap_type.\n");
		return 1;
	}
	if(temp == 0) return 2;
	
	temp = Push_Queue(UploadQueue, (char*)buf_temp, temp);
	if(temp) return 3;
	return 0;
}


uint8_t GET_NB_SendMessage_FromQueue(uint8_t *out_buf, uint16_t *message_length)
{
	size_t i, temp, length;
	Message_pack *pack;
	
	pack = Read_Queue(UploadQueue);
	
	length = GET_SendMessage_Head(out_buf, pack->message_length);
	length += HexArray_2_HexString( pack->message, out_buf+length, pack->message_length );
	length += GET_SendMessage_End(out_buf+length, pack->message_length);
	
	*message_length = pack->message_length;
	
	Pop_Queue(UploadQueue);
	return 0;
}


uint8_t received2queue_callback(char *input)
{
	char buf_temp[512] = {0};
	size_t temp=0;
	
	temp = HexString_2_HexArray( input, buf_temp, strlen(input) );
	temp = Push_Queue( DownloadQueue, buf_temp, temp);
	if(temp) return 1;
	return 0;
}

uint8_t POST_NB_Received_ToQueue(uint8_t *rec_buf, uint16_t buf_max_length)
{
	size_t temp=0;
	
	temp = ACK_NB_REC(rec_buf, received2queue_callback);
	return temp;
}


uint8_t ACK_NB_Received_FromQueue(uint8_t *coap_type, uint8_t *message_type)
{
	size_t i, temp, length;
	Message_pack *pack;
	
	pack = Read_Queue(DownloadQueue);
	temp = ACK_IOT_Received(pack->message, pack->message_length, message_type);
	Pop_Queue(DownloadQueue);
	
	*coap_type = temp;
	if(temp == CMT_CMD) {
		LOGI("IOT Receive command type[%d]\n", *message_type);
	}
	else if(temp == CMT_Message_ACK) {
		LOGI("IOT Receive message_ack type[%d].\n", *message_type);
	}
	else return 1;
	
	return 0;
}

	// --------------------- 若不使用列队 -------------------- //
#else
	
uint8_t GET_NB_SendMessage(uint8_t *out_buf, uint16_t *message_length, uint8_t coap_type, uint8_t message_type)
{
	size_t temp, length;
	char buf_temp[512]={0};
	
	if(coap_type == CMT_Message)
	{
		temp = GET_IOT_SendMessage(message_type, buf_temp);
	}
	else if(coap_type == CMT_CMD_ACK)
	{
		temp  = GET_IOT_CmdAck((message_type, buf_temp);
	}
	else return 1;
	if( temp == 0 ) return 2; 
	
	length = GET_SendMessage_Head(out_buf, temp);
	length += HexArray_2_HexString( buf_temp, out_buf+length, temp );
	length += GET_SendMessage_End(out_buf+length, temp);
	
	if(message_length != NULL)
	*message_length = length;
	return 0;
}

uint8_t received_callback(char *input)
{
	size_t temp;
	char buf_temp[512]={0};
	
	temp = HexString_2_HexArray( input, buf_temp, strlen(input) );
	coap_type_temp = ACK_IOT_Received( buf_temp, temp, &message_type_temp );
	if(coap_type_temp == CMT_CMD) {
		LOGI("IOT Receive command type[%d]\n", message_type_temp);
	}else if(coap_type_temp == CMT_Message_ACK) {
		LOGI("IOT Receive message_ack type[%d].\n", *message_type);
	}else return 1;
	
	return 0;
}

uint8_t ACK_NB_Received(uint8_t *rec_buf, uint8_t *coap_type, uint8_t *message_type)
{
	size_t temp=0;
	coap_type_temp = 0;
	message_type_temp = 0;
	
	temp = ACK_NB_REC(rec_buf, received_callback);
	*coap_type = coap_type_temp;
	*message_type = message_type_temp;
	return 0;
}

#endif


void PUT_NB_CommandCallback(NB_Port_Callback_t funtion)
{
	gl_CommandCallback = funtion;
}


uint8_t POST_NB_Command(uint8_t cmd_name, void *arg)
{
	uint8_t temp;
	
	temp = Execute_NB_CMD(cmd_name, arg, gl_CommandCallback);
	
	return temp;
}
