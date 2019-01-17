/* 
	
*/
#include "BC26_Module.h"
#include "hardware_interface.h"
#include "common_function.h"

#include <string.h>
#include <stdlib.h>



// ——————————————    本地设置    —————————————— //

/*  发送消息最大错误的次数限制：当超过最大次数时表示消息无法发送 */
#define SEND_MESSAGE_ERROR_TIMES_MAX 5

const char *MESSAGE_HEAD_SEND = “"AT+QLWDATASEND=19,0,0,%s,"”;
const char *MESSAGE_TAIL_SEND = ",0X0100\r\n";
const char *MESSAGE_HEAD_RECEIVE = "+QLWOBSERVE:%s\r\n";
const char *MESSAGE_SEND_OK = "SEND OK";
const char *MESSAGE_SEND_FAIL = "SEND FAILD";


// ——————————————    本地定义    —————————————— //

typedef struct message_pack_
{
	uint16_t message_length;
	uint8_t *message;
	struct message_pack_ *next;
}message_pack_t;


typedef struct 
{
	uint16_t queue_n;
	message_pack_t *member;
}MsgQueue_t;

#define  MSG_QUEUE_MAX  10		// 消息列队的最大队员数量

// ——————————————    本地变量   —————————————— //

static MsgQueue_t MsgUpLoad;	// 上传的消息列队
static MsgQueue_t MsgDownload;	// 下发的消息列队
/* 该值用于记录当前正在发送的命令 */
static CmdPack_t *stat_cmd;
/* 该值用于记录当前正在发送的消息 */
static message_pack_t *stat_message;

/* 当发送消息时，该值置1；直到发送消息成功时，该值再置0；*/
static uint8_t sending_message_flag;
/* 当发送命令时，该值置1；直到接收到完整的命令回应包时，该值再置0；*/
static uint8_t sending_cmd_flag;
/* 该值用于记录发送消息失败的次数 */
static uint8_t send_message_error;


// ——————————————    本地函数    —————————————— //

uint8_t Push_MessageQueue( uint8_t *buff, uint8_t data_length );
MsgQueue_t* Pop_MessageQueue( void );

uint8_t  TrySendMessage( message_pack_t*  message);
uint8_t TrySendCmd( CmdPack_t* cmd);
uint8_t TryReceiveResponse(  uint8_t  *buf_temp );
uint8_t TryReceiveMessage( uint8_t *rec_buf );


// ——————————————    接口函数源    —————————————— //


uint8_t IF_ReceiveMessage(void)
{
	ReadData_FromSerialPort();
	if( MsgDownload.queue_n > 0 ) {
		return 1;
	}
	else 
		return 0;
}

uint8_t IF_ReveivedCmdResponse(void)
{
	ReadData_FromSerialPort();
	if( sending_cmd_flag ) {
		return 0;
	}
	else 
		return 1;
}

uint8_t IF_SendMessageError(void)
{
	if( sending_message_error > SEND_MESSAGE_ERROR_TIMES_MAX ) {
		return 1;
	}
	else 
		return 0;
}


uint8_t SendCmd( CmdPack_t *cmd )
{
	if( stat_cmd != NULL )
		return 1;
	
	if( cmd == NULL )
		return 2;
	
	stat_cmd = cmd;
	return 0;
}


CmdPack_t* ReceiveCmd	(void)
{
	return stat_cmd;
}

uint8_t SendMessage( uint8_t *buff, uint8_t data_length )
{
	uint8_t ret;
	
	if( data_length == 0 ) return 1;
	if( buff == NULL ) return 2;
	
	ret = Push_MessageQueue( &MsgUpLoad,  buff, data_length );
	if( ret )	// If Error
		ret+=2;
	return ret;
}


uint8_t ReceiveMessage( uint8_t *out_buff, uint16_t buff_size, uint8_t *data_length )
{
	message_pack_t *p;
	
	if( buff == NULL )
		return 1;
	p = Pop_MessageQueue( &MsgDownload );
	if( p == NULL )
		return 2;
	if( buff_size <= p->message_length )
		return 3;
	
	memcpy( out_buff,  p->message, p->message_length );
	if( data_length != NULL)
		*data_length = p->message_length;
	
	return 0;
}


uint8_t WriteData_ToSerialPort( uint16_t *write_length );
{
	uint16_t  data_length;
	uint8_t *buf_temp, ret=0; 
	
	
	if( sending_message_flag && sending_cmd_flag )
			return 1;	// SerialPort busy now.
	else if( !sending_message_flag && send_message_error < SEND_MESSAGE_ERROR_TIMES_MAX)
	{	// send message
		if( stat_message == NULL )
			stat_message = Pop_MessageQueue( &MsgUpLoad );
		ret = TrySendMessage( stat_message );
		if( ret )
			ret += 3;	// Send failed.
	}
	else if( !sending_cmd_flag)
	{	// send cmd
		if( stat_cmd == NULL )
			return 2;	// No command is wating to be sent.
		else {
			ret = TrySendCmd( stat_cmd );
			if( ret ) {
				send_message_error ++;
				ret += 3;	// Send failed.
			}
		}
	}
	else if( end_message_error >= SEND_MESSAGE_ERROR_TIMES_MAX )
		return 3;	// Can't not send message
	
	return ret;
}


uint8_t ReadData_FromSerialPort( void )
{
	uint8_t ret=0, *rec_buf, *p, *buf_temp;
	uint16_t data_length, rec_buff_size;
	
	data_length = Read_SerialPort( &rec_buf, &rec_buff_size );
	if( data_length == 0 || data_length > rec_buff_size) {
		return 1; 	// Nothing Read out form serial port or Read Function error. 
	}
	
	p = strtok(rec_buf, "\r\n");	// Cut out a single data line form receive buffer.
	while(p != NULL) {
		buf_temp = NULL;
		
		data_length = strlen(p);
		if( data_length == 0)
			continue;	// Empty data line.
		else 
			buf_temp = calloc( data_length+1, 1);
		if( buf_temp == NULL ) {
			ret = 2;
			goto ReadData_FromSerialPort_exit;		// Calloc buffer error.
		}
		
		if( strncmp( p, MESSAGE_HEAD_RECEIVE, 6 ) == 0 )
		{	// Here is the process of receiving a message.
			ret = TryReceiveMessage( buf_temp );
			if( ret ) {
				ret += 3;
				goto ReadData_FromSerialPort_exit;	
			}
		}
		else if( strncmp( p, MESSAGE_SEND_OK, 6 ) == 0 )
		{	// Here is the process of sending a message succeed.
			sending_message_flag = 0;
			send_message_error = 0;
			if( stat_message != NULL )
				free( stat_message );
			stat_message = NULL;
		}
		else if( strncmp( p, MESSAGE_SEND_FAIL, 6 ) == 0 )
		{	// Here is the process of sending a message failed.
			if( send_message_error < 0xFF )
			send_message_error ++;
			if( send_message_error < SEND_MESSAGE_ERROR_TIMES_MAX ) {
				ret = TrySendMessage( stat_message ); 	// Try to resend it.
				if( ret ) send_message_error++;
			}
		}
		else 
		{	// Here is the process of Receive command.
			ret = TryReceiveResponse( buf_temp );
			if( ret ) {
				ret += 3;
				goto ReadData_FromSerialPort_exit;	
			}
		}
	
		free( buf_temp );
	}
	
ReadData_FromSerialPort_exit:
	if( buf_temp != NULL) free( buf_temp );
	return ret;
}


// ——————————————    本地函数源    —————————————— //

uint8_t Push_MessageQueue( MsgQueue_t *queue, uint8_t *buff, uint8_t data_length )
{
	message_pack_t *p;
	
	if( queue->queue_n >= MSG_QUEUE_MAX ) {
		return 1;	// queue full.
	}
	
	if( queue->member == NULL) {
		p = calloc(1, sizeof(message_pack_t));
		queue->member =  p;
	}
	else {
		for( p=queue->member;  p->next != NULL;  p=p->next );
		p->next = calloc( sizeof(message_pack_t) );
		p = p->next;
	}
	if( p == NULL) return 2;	// calloc failed.
	
	p->message = calloc(data_length, 1);
	if(p->message == NULL) return 3; 	// calloc failed.
	memcpy(p->message, buff, data_length);
	p->message_length = data_length;
	
	queue->queue_n++;
	return 0;
}


message_pack_t* Pop_MessageQueue( MsgQueue_t *queue )
{
	message_pack_t *p;
	
	if( queue->queue_n == 0 ||  queue->queue_n  > MSG_QUEUE_MAX ) {
		return NULL;	// queue empty or error.
	}
	
	p = queue->member;
	queue->member = p->next;
	return p;
}


uint8_t  TrySendMessage( message_pack_t*  message)
{
	uint16_t data_length, ret, buff_size, response_buffer[16]={0};
	uint8_t *buf_temp, *rec_buf; 
	message_pack_t *p=NULL;
	
	p = message;
	if( p == NULL ) return 1;	// no message.
	
	buf_temp = calloc( (p->message_length*2+100) , 1);
	if( buf_temp == NULL ) return 2;	// calloc failed.
	
	sprintf( buf_temp, MESSAGE_HEAD_SEND, p->message_length );
	data_length = strrlen( buf_temp );
	data_length += HexArray_2_HexString( (char *)p->message, (char *)buf_temp+data_length, p->message_length );
	strncat( buf_temp, MESSAGE_TAIL_SEND,  (p->message_length*2+100) );
	
	data_length = strlen(buf_temp);
	if(data_length < p->message_length *2) return 3;		// buf error;
	
	ret = Write_SerialPort( buf_temp, data_length );
	if(  ret  != data_length )  return 4;	// serial port send error.
	
	ret = ReadData_FromSerialPort();	// Empty serial port receive buffer.
	TimerDelay_ms( 10 );
	ret = Read_SerialPort( &rec_buf, &buff_size );
	if( strncmp( rec_buf, "OK", 2 ) != 0 ) {
		return 5; 	// NB module can't send message.
	}
	return 0;
}


uint8_t TrySendCmd( CmdPack_t* cmd)
{
	uint16_t data_length,temp;
	
	if( cmd == NULL ) return 1;
	
	data_length = strlen( cmd->cmd );
	temp = Write_SerialPort( (uint8_t *)cmd->cmd, data_length );
	if( temp != data_length ) return 2;
	
	return 0;
}


uint8_t TryReceiveResponse(  uint8_t  *rec_buf )
{
	uint8_t ret =0, *response_buff;
	size_t i, data_length;
	cmd_response_t *response_reg, *response_rsp;
	
	if( stat_cmd == NULL) {
		return ret;	// No command is waiting to receive a response.
	}else {
		response_reg = stat_cmd->reg_response;
		response_rsp = stat_cmd->cmd_response;
		response_buff = response_reg->response;
	}
	
	for( i=0; i < stat_cmd->cmd_response_number; i++ )
	{
		if( strncmp( rec_buf, response_buff, 6) == 0 )
		{	// If received data is cmd response.
			if( response_rsp == NULL ) {
				response_rsp = calloc( 1, sizeof(cmd_response_t) );
				if( response_rsp == NULL)
					return 2;	// Calloc buffer error.
			}
			else {
				for( ; response_rsp->next != NULL; response_rsp = response_rsp->next );
			}
			
			data_length = strlen(rec_buf);
			response_rsp->response = calloc( data_length+1, 1);
			if( response_rsp->response == NULL) {	
				return 2;	// Calloc buffer error.
			}
			memcpy( response_rsp->response, rec_buf, data_length );
			break; 
		}
		response_reg = response_reg->next;
		response_buff = response_reg->response;
	}
	
	return ret;
}


uint8_t TryReceiveMessage( uint8_t *rec_buf )
{
	uint8_t ret, *buf_coap;
	uint16_t data_length;
	
	buf_coap = calloc( data_length/2, 1 );
	if( buf_coap == NULL ) {
		ret = 2;
		return ret;		// Calloc buffer error.
	}
	
	sscanf(p, "%*[^,],%*[^,],%*[^,],%*[^,],%[^\r]", rec_buf);
	data_length =  strlen(rec_buf);
	data_length = HexString_2_HexArray( rec_buf, buf_coap, data_length );
	ret = Push_MessageQueue( &MsgDownload, buf_coap, data_length );
	if( ret ) {
		ret = 3;
		return ret;		;		// Push queue failed.
	}
	
	free(buf_coap);
	return ret;
}
