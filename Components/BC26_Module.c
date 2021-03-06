/* *Copyright(C), Haijie Lee. 2019. AllRightsReserved.
	*FileName:  BC26模组的通信处理 模块源文件.
	*Author:  Haijie Lee.
	*Version:  1.5.
	*Date:  2019.01.22.
	*Description:  实现通过串口与NB模组的通信过程。
							本模块依赖<hardware_interface>提供的硬件接口功能、
							依赖<common_function>提供的字符串转换功能。
	*Others:  用户不应随意修改IOT定义的数据，除非你知道你在做什么。
*/
#include "BC26_Module.h"
#include "hardware_interface.h"
#include "common_function.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>



// ——————————————    本地设置    —————————————— //

/*  发送消息最大错误的次数限制：当超过最大次数时表示消息无法发送 */
#define SEND_MESSAGE_ERROR_TIMES_MAX 5

/*  发送和接收消息相关的指令头/尾 */
const char *MESSAGE_HEAD_SEND = "AT+QLWDATASEND=19,0,0,%d,";
const char *MESSAGE_TAIL_SEND = ",0X0100\r\n";
const char *MESSAGE_HEAD_RECEIVE = "+QLWDATARECV:%s\r\n";
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

uint8_t Push_MessageQueue( MsgQueue_t *queue, uint8_t *buff, uint8_t data_length );
message_pack_t* Pop_MessageQueue( MsgQueue_t *queue );

uint8_t  TrySendMessage( message_pack_t*  message);
uint8_t TrySendCmd( CmdPack_t* cmd);
uint8_t TryReceiveResponse(  uint8_t  *buf_temp );
uint8_t TryReceiveMessage( uint8_t *rec_buf, uint16_t data_length );


// ——————————————    接口函数源    —————————————— //


uint8_t IF_ReceiveMessage(void)
{
	
	if( MsgDownload.queue_n > 0 ) {
		return 1;
	}
	else 
		return 0;
}

uint8_t IF_ReveivedCmdResponse(void)
{
	uint8_t ret;
	TimerDelay_ms(50);
	ret = ReadData_FromSerialPort();
	
	if( stat_cmd != NULL )
	if( stat_cmd->cmd_response_n >= stat_cmd->cmd_response_max ) {
		sending_cmd_flag = 0;
		stat_cmd = NULL;
		return 1;
	}
	
	if( ret )
		return ret+1;
	if( sending_cmd_flag ) {
		return 0;
	}
	else 
		return 1;
}

uint8_t IF_SendMessageError(void)
{
	if( send_message_error > SEND_MESSAGE_ERROR_TIMES_MAX ) {
		return 1;
	}
	else 
		return 0;
}


uint8_t SendCmd( CmdPack_t *cmd, uint16_t delay )
{
	uint8_t ret;
	
	if( stat_cmd != NULL )
		return 1;
	if( cmd == NULL )
		return 2;
	
	if( delay < 10000 )
		TimerDelay_ms( delay );
	
	stat_cmd = cmd;
	ret = WriteData_ToSerialPort();
	if(ret)
		return 1;
	sending_cmd_flag = 1;
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


uint8_t ReceiveMessage( uint8_t *out_buff, uint16_t buff_size, uint16_t *data_length )
{
	message_pack_t *p;
	
	if( out_buff == NULL )
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


uint8_t WriteData_ToSerialPort( void )
{
	uint8_t ret=0; 
	
	//printf("Write data.\n");
	if( sending_message_flag && sending_cmd_flag )
			return 1;	// SerialPort busy now.
	if( !sending_message_flag && send_message_error < SEND_MESSAGE_ERROR_TIMES_MAX)
	{	// send message
		if( stat_message == NULL )
			stat_message = Pop_MessageQueue( &MsgUpLoad );
		ret = TrySendMessage( stat_message );
		if( ret )
			ret += 3;	// Send failed.
	}
	if( !sending_cmd_flag)
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
	if( send_message_error >= SEND_MESSAGE_ERROR_TIMES_MAX )
		return 3;	// Can't not send message
	
	return ret;
}


uint8_t ReadData_FromSerialPort( void )
{
	char ret=0, *rec_buf, *p;
	uint16_t data_length, rec_buff_size;
	
	data_length = Read_SerialPort( (uint8_t **)&rec_buf, &rec_buff_size );
	if( data_length == 0 || data_length > rec_buff_size) {
		return 1; 	// Nothing Read out form serial port or Read Function error. 
	}
	
	p = strtok( rec_buf, "\r\n");	// Cut out a single data line form receive buffer.
	while(p != NULL) {
		
		data_length = strlen(p);
		if( data_length == 0) {
			p = strtok( NULL, "\r\n");
			continue;	// Empty data line.
		}
		
		//printf("Receive dataline: %s.\n",p);
		if( strncmp( p, MESSAGE_HEAD_RECEIVE, 6 ) == 0 )
		{	// Here is the process of receiving a message.
			ret = TryReceiveMessage( (uint8_t *)p, data_length );
			if( ret ) {
				
				ret += 3;
				return ret;	
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
			if( sending_cmd_flag != 0 )
				ret = TryReceiveResponse( (uint8_t *)p );
			if( ret ) {
				ret += 3;
				return ret;	
			}
		}
	
		p = strtok( NULL, "\r\n");
	}
	return 0;
}


inline void RepealCmd(void)
{
	if(stat_cmd != NULL)
	stat_cmd = NULL;
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
		p->next = calloc( 1, sizeof(message_pack_t) );
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
	
	if(queue == NULL)
		return NULL;
	if( queue->queue_n == 0 ||  queue->queue_n  > MSG_QUEUE_MAX ) {
		return NULL;	// queue empty or error.
	}
	
	p = queue->member;
	queue->member = p->next;
	queue->queue_n--;
	return p;
}


uint8_t  TrySendMessage( message_pack_t*  message)
{
	uint16_t data_length, ret, buff_size;
	char *buf_temp, *rec_buf; 
	message_pack_t *p=NULL;
	
	p = message;
	if( p == NULL || p->message_length == 0 ) return 1;	// no message.
	
	buf_temp = calloc( (p->message_length*2+100) , 1);
	if( buf_temp == NULL ) return 2;	// calloc failed.
	
	sprintf( buf_temp, MESSAGE_HEAD_SEND, p->message_length );
	data_length = strlen( buf_temp );
	data_length += HexArray_2_HexString( (char *)p->message, (char *)buf_temp+data_length, p->message_length );
	strncat( buf_temp, MESSAGE_TAIL_SEND,  (p->message_length*2+100) );
	
	data_length = strlen(buf_temp);
	if(data_length < p->message_length * 2) return 3;		// buf error;
	
	ret = Write_SerialPort( (uint8_t *)buf_temp, data_length );
	if(  ret  != data_length )  return 4;	// serial port send error.
	
	sending_message_flag = 1;
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
	char ret =0, *response_buff;
	size_t data_length;
	cmd_response_t *response_reg, *response_rsp, *p;
	
	if( stat_cmd == NULL) {
		return 0;	// No command is waiting to receive a response.
	}else {
		response_reg = stat_cmd->reg_response;
	}
	if( stat_cmd->cmd_response_n >= stat_cmd->cmd_response_max ) {
		sending_cmd_flag = 0;
		stat_cmd = NULL;
		return 0;
	}
	
	while( response_reg != NULL )
	{
		response_buff = response_reg->response;
		
		if( strncmp( response_buff, (char *)rec_buf, 3) == 0 )
		{	// If received data is cmd response.
			if( stat_cmd->cmd_response == NULL ) {
				stat_cmd->cmd_response = calloc( 1, sizeof(cmd_response_t) );
				if( stat_cmd->cmd_response == NULL)
					return 2;	// Calloc buffer error.
				response_rsp = stat_cmd->cmd_response;
			}
			else {
				for( response_rsp = stat_cmd->cmd_response; response_rsp->next != NULL; response_rsp = response_rsp->next );
			}
			
			data_length = strlen((char *)rec_buf);
			response_rsp->response = calloc( data_length+1, 1);
			if( response_rsp->response == NULL) {	
				return 2;	// Calloc buffer error.
			}
			memcpy( response_rsp->response, rec_buf, data_length );
			stat_cmd->cmd_response_n++;
			
			if( stat_cmd->cmd_response_n >= stat_cmd->cmd_response_max ) {
				sending_cmd_flag = 0;
				stat_cmd = NULL;
				return 0;
			}
			break; 
		}
		response_reg = response_reg->next;
	}
	
	return ret;
}


uint8_t TryReceiveMessage( uint8_t *rec_buf, uint16_t data_length )
{
	char ret, *buf_coap, *buf_temp;
	
	buf_temp = calloc( data_length, 1 );
	buf_coap = calloc( data_length/2, 1 );
	if( buf_coap == NULL || buf_temp == NULL ) {
		return 1;		// Calloc buffer error.
	}
	
	sscanf( (char *)rec_buf, "%*[^,],%*[^,],%*[^,],%*[^,],%[^\r]", buf_temp );
	data_length = strlen( buf_temp );
	data_length = HexString_2_HexArray( buf_temp, buf_coap, data_length );
	ret = Push_MessageQueue( &MsgDownload, (uint8_t *)buf_coap, data_length );
	if( ret ) {
		return ret+1;		// Push queue failed.
	}
	
	free(buf_coap);
	return 0;
}
