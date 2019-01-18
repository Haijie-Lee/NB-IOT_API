/* 
	
*/
#include "iot_user.h"

#if NB_MODEL_VERSION == BC_26	
	#include "BC26_AT.h"
	#include "BC26_Module.h"
#endif
 
#include <string.h>
#include <stdlib.h>
#include <stdio.h>



// ——————————————    本地设置    —————————————— //

// ——————————————    本地定义    —————————————— //

// ——————————————    本地变量   —————————————— //

// ——————————————    本地函数声明    —————————————— //

CmdPack_t * NewCmdPack( char *cmd, uint8_t response_n );
uint8_t FreeCmdPack( CmdPack_t *pack ); 
uint8_t RegisterCmdResponse( CmdPack_t *pack, char *response );
char* ReceiveCmdResponse( CmdPack_t *pack )

// ——————————————    接口函数源    —————————————— //

/* AT指令封装函数示范 */
uint8_t AT_SleepMode_Lock(void)
{
	uint8_t ret=0, count=0;
	char *buf;
	CmdPack_t *p;
	
	// 传入AT指令，【模组需要回复的数据行】的行数，生成指令包
	p = NewCmdPack( "AT+SM=LOCK\r\n", 1 );
	if( p == NULL)
		return 1;
	// 注册回复内容的头（当串口接收到的数据行与这个头的匹配，即会把整个数据行放入命令数据包）
	//	此命令有两种可能的结果（命令回复）—— 成功或失败
	ret = RegisterCmdResponse( p, "OK" );	
	ret = RegisterCmdResponse( p, "ERROR" );	
	if( ret ) 
		goto smlock_exit;
	
	do {
		ret = SendCmd( p, 5 );	// 发送命令成功之后延迟5毫秒
	}while( ret != 0 );	// 串口可能由于正在发送其它内容而需要等待，直到发送命令成功
	
	do{
		ret = IF_ReveivedCmdResponse();	// 如果接收到命令回复
		count++;
		if( count > 10 ) {
			ret = 3;	// 若接收不到回复超过10次，视为该命令执行失败
			goto smlock_exit;
		}
	}while( ret == 0 );	// 若不确定5毫秒内命令即可执行成功，可加入循环等待，确保模组执行命令完成
	
	// 此处仅有一行回复，故仅需要一次接收回复；
	// 若有多行回复数据，可使用循环处理，例如：for( i=0; i<p->cmd_response_maxr; i++ )
	buf = ReceiveCmdResponse( p );	// 使用字符指针接收命令的回复的内容
	if( buf = NULL ) {
		ret = 3;	// 如果命令回复内容为空
		goto smlock_exit;
	}
	
	if( strncmp( buf, "OK", 2) != 0 ) {	// 对回复内容进行处理（例如读出某个参数等）
		ret = 2;	// 如果命令执行不成功
	}

smlock_exit:
	FreeCmdPack( p );	// 释放掉此命令包
	return ret;	// 注：若命令执行成功返回值应为0
}


uint8_t AT_CloseEcho(void);
uint8_t AT_ReadIMEI( char *target );
uint8_t AT_ReadIMSI( char *target );
uint8_t AT_EnableWireless(void);
uint8_t AT_CheckAttachNetwork(void);
uint8_t AT_ReadSignalStrength( char *target );
uint8_t AT_ReadAttachIP( char *target );
uint8_t AT_ConfigPlatformIP( const char *source );
uint8_t AT_ConfigEndPoint( const char *source );
uint8_t AT_Add_LwM2M_Object( const char *sourcet );
uint8_t AT_OpenConnectPlatform(void);
uint8_t AT_Config_LwM2M_Dataformat(void);

// ——————————————    本地函数源    —————————————— //

CmdPack_t * NewCmdPack( char *cmd, uint8_t response_n )
{
	uint16_t data_length;
	CmdPack_t *p;
	
	if( cmd == NULL )
		return NULL;
	p = calloc( 1, sizeof(CmdPack_t) );
	if( p == NULL )
		return NULL;
	
	data_length = strlen( cmd );
	p->cmd = calloc( data_length+1, 1 );
	if( p->cmd == NULL ) {
		free( p );
		return NULL;
	}
	
	p->cmd_response_number = response_n;
	memcpy( p->cmd, cmd, data_length );
	return p;
}


uint8_t FreeCmdPack( CmdPack_t *pack );
{
	uint16_t data_length;
	cmd_response_t *p;
	
	if( pack == NULL )
		return 1;
	
	free( pack->cmd );
	
	p = pack->reg_response;
	while( p != NULL ) {
		pack->reg_response = p->next;
		free( p->response );
		free( p );
		p = pack->reg_response;
	}
	
	p = pack->cmd_response;
	while( p != NULL ) {
		pack->cmd_response = p->next;
		free( p->response );
		free( p );
		p = pack->cmd_response;
	}
	
	free( pack );
	return 0;
}


uint8_t RegisterCmdResponse( CmdPack_t *pack, char *response )
{
	uint16_t data_length;
	cmd_response_t *p;
	
	if( pack == NULL || response == NULL )
		return 1;
	
	p = pack->reg_response;
	if( p == 	NULL ) {
		p = calloc( 1, sizeof(cmd_response_t) );
	}else {
		for( ; p->next != NULL; p = p->next );
		p = calloc( 1, sizeof(cmd_response_t) );
	}
	if( p == 	NULL )
		return 2;	// calloc failed.
	
	data_length = strlen( response );
	p->response = calloc( data_length, 1 );
	if( p->response == 	NULL )
		return 2;	// calloc failed.
	
	return 0;
}


char* ReceiveCmdResponse( CmdPack_t *pack )
{
	static cmd_response_t *now_response = NULL;
	static CmdPack_t *now_cmd = NULL;
	cmd_response_t *p;
	
	if(now_cmd != pack ) {
		now_cmd = pack;
		now_response = now_cmd->cmd_response;
	}
	
	if( now_response == NULL )
		return NULL;
	
	p = now_cmd;
	now_response = p->next;
	
	return p->response;
}



