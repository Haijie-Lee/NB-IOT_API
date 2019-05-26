/* *Copyright(C), Haijie Lee. 2019. AllRightsReserved.
	*FileName:  BC26模组的AT指令 模块源文件.
	*Author:  Haijie Lee.
	*Version:  1.5.
	*Date:  2019.01.22.
	*Description:  实现AT指令的发送、接收回复的过程。
							本模块依赖 <BC26_Module>实现AT指令数据与NB模组的交换过程。
	*Others:  用户不应随意修改此文件，除非你知道自己在做什么。
*/
#include "BC26_AT.h"
#include "BC26_Module.h"
 
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
char* ReceiveCmdResponse( CmdPack_t *pack );

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
	
	//do {
		
		ret = SendCmd( p, 50 );	// 发送命令成功之后延迟50毫秒
	//}while( ret != 0 );	// 串口可能由于正在发送其它内容而需要等待，直到发送命令成功
	
	do{
		ret = IF_ReveivedCmdResponse();	// 如果接收到命令回复
		count++;
		if( count > 10 ) {
			RepealCmd();
			ret = 3;	// 若接收不到回复超过10次，视为该命令执行失败
			goto smlock_exit;
		}
	}while( ret != 1 );	// 若不确定5毫秒内命令即可执行成功，可加入循环等待，确保模组执行命令完成
	ret = 0;
	// 此处仅有一行回复，故仅需要一次接收回复；
	// 若有多行回复数据，可使用循环处理，例如：for( i=0; i<p->cmd_response_maxr; i++ )
	buf = ReceiveCmdResponse( p );	// 使用字符指针接收命令的回复的内容
	if( buf == NULL ) {
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


uint8_t AT_CloseEcho(void)
{
	uint8_t ret=0, count=0;
	char *buf;
	CmdPack_t *p;
	
	// 传入AT指令，【模组需要回复的数据行】的行数，生成指令包
	p = NewCmdPack( "ATE0\r\n", 1 );
	if( p == NULL)
		return 1;
	// 注册回复内容的头（当串口接收到的数据行与这个头的匹配，即会把整个数据行放入命令数据包）
	//	此命令有两种可能的结果（命令回复）—— 成功或失败
	ret = RegisterCmdResponse( p, "OK" );	
	ret = RegisterCmdResponse( p, "ERROR" );	
	if( ret ) 
		goto closeecho_exit;
	
	//do {
		ret = SendCmd( p, 50 );	// 发送命令成功之后延迟50毫秒
	//}while( ret != 0 );	// 串口可能由于正在发送其它内容而需要等待，直到发送命令成功
	
	do{
		ret = IF_ReveivedCmdResponse();	// 如果接收到命令回复
		count++;
		if( count > 10 ) {
			RepealCmd();// 若接收不到回复超过10次，视为该命令执行失败，则应该撤销当前发送的命令。
			ret = 3;
			goto closeecho_exit;
		}
	}while( ret != 1 );	// 若不确定5毫秒内命令即可执行成功，可加入循环等待，确保模组执行命令完成
	ret = 0;
	// 此处仅有一行回复，故仅需要一次接收回复；
	// 若有多行回复数据，可使用循环处理，例如：for( i=0; i<p->cmd_response_maxr; i++ )
	buf = ReceiveCmdResponse( p );	// 使用字符指针接收命令的回复的内容
	if( buf == NULL ) {
		ret = 3;	// 如果命令回复内容为空
		goto closeecho_exit;
	}
	
	if( strncmp( buf, "OK", 2) != 0 ) {	// 对回复内容进行处理（例如读出某个参数等）
		ret = 2;	// 如果命令执行不成功
	}


closeecho_exit:
	FreeCmdPack( p );	// 释放掉此命令包
	return ret;	// 注：若命令执行成功返回值应为0
}


uint8_t AT_ReadIMEI( char *target )
{
	uint8_t ret=0, count=0;
	char *buf;
	CmdPack_t *p;
	
	// 传入AT指令，【模组需要回复的数据行】的行数，生成指令包
	p = NewCmdPack( "AT+CGSN=1\r\n", 1 );
	if( p == NULL)
		return 1;
	// 注册回复内容的头（当串口接收到的数据行与这个头的匹配，即会把整个数据行放入命令数据包）
	//	此命令有两种可能的结果（命令回复）—— 成功或失败
	ret = RegisterCmdResponse( p, "+CGSN:%s\r\n" );	
	if( ret ) 
		goto readimei_exit;
	
	do {
		ret = SendCmd( p, 50 );	// 发送命令成功之后延迟5毫秒
	}while( ret != 0 );	// 串口可能由于正在发送其它内容而需要等待，直到发送命令成功
	
	do{
		ret = IF_ReveivedCmdResponse();	// 如果接收到命令回复
		count++;
		if( count > 10 ) {
			RepealCmd();
			ret = 3;	// 若接收不到回复超过10次，视为该命令执行失败
			goto readimei_exit;
		}
	}while( ret != 1 );	// 若不确定5毫秒内命令即可执行成功，可加入循环等待，确保模组执行命令完成
	ret = 0;
	// 此处仅有一行回复，故仅需要一次接收回复；
	// 若有多行回复数据，可使用循环处理，例如：for( i=0; i<p->cmd_response_maxr; i++ )
	buf = ReceiveCmdResponse( p );	// 使用字符指针接收命令的回复的内容
	if( buf == NULL ) {
		ret = 3;	// 如果命令回复内容为空
		goto readimei_exit;
	}
	
	if( target != NULL )
	sscanf(buf, "%*[^:]: %[^\r]", target );	// 对回复内容进行处理（例如读出某个参数等）
	
	if( strlen(target) != 15 ) { 
		ret = 2;	// 如果命令执行不成功
	}

readimei_exit:
	FreeCmdPack( p );	// 释放掉此命令包
	return ret;	// 注：若命令执行成功返回值应为0
}


uint8_t AT_ReadIMSI( char *target )
{
	uint8_t ret=0, count=0;
	char *buf;
	CmdPack_t *p;
	
	// 传入AT指令，【模组需要回复的数据行】的行数，生成指令包
	p = NewCmdPack( "AT+CIMI\r\n", 1 );
	if( p == NULL)
		return 1;
	// 注册回复内容的头（当串口接收到的数据行与这个头的匹配，即会把整个数据行放入命令数据包）
	//	此命令有两种可能的结果（命令回复）—— 成功或失败
	ret = RegisterCmdResponse( p, "460" );	
	if( ret ) 
		goto readimsi_exit;
	
	do {
		ret = SendCmd( p, 40 );	// 发送命令成功之后延迟5毫秒
	}while( ret != 0 );	// 串口可能由于正在发送其它内容而需要等待，直到发送命令成功
	
	do{
		ret = IF_ReveivedCmdResponse();	// 如果接收到命令回复
		count++;
		
		if( count > 10 ) {
			RepealCmd();
			ret = 3;	// 接收不到回复, 若超过10次，视为该命令执行失败
			goto readimsi_exit;
		}
	}while( ret != 1 );	// 若不确定5毫秒内命令即可执行成功，可加入循环等待，确保模组执行命令完成
	ret = 0;
	// 此处仅有一行回复，故仅需要一次接收回复；
	// 若有多行回复数据，可使用循环处理，例如：for( i=0; i<p->cmd_response_maxr; i++ )
	buf = ReceiveCmdResponse( p );	// 使用字符指针接收命令的回复的内容
	if( buf == NULL ) {
		ret = 3;	// 如果命令回复内容为空
		goto readimsi_exit;
	}
	
	if( target != NULL )
	memcpy( target, buf, 15 );	// 对回复内容进行处理（例如读出某个参数等）
	
	if( strlen(target) != 15 ) { 
		ret = 2;	// 如果命令执行不成功
	}

readimsi_exit:
	FreeCmdPack( p );	// 释放掉此命令包
	return ret;	// 注：若命令执行成功返回值应为0
}


uint8_t AT_EnableWireless(void)
{
	uint8_t ret=0, count=0, flag=0;
	char *buf;
	CmdPack_t *p;
	
	// 传入AT指令，【模组需要回复的数据行】的行数，生成指令包
	p = NewCmdPack( "AT+CFUN?\r\n", 1 );
	if( p == NULL)
		return 1;
	// 注册回复内容的头（当串口接收到的数据行与这个头的匹配，即会把整个数据行放入命令数据包）
	//	此命令有两种可能的结果（命令回复）—— 成功或失败
	ret = RegisterCmdResponse( p, "+CFUN: %d\r\n" );	
	
	if( ret ) 
		goto enablewireless_exit;
	
	do {
		ret = SendCmd( p, 40 );	// 发送命令成功之后延迟5毫秒
	}while( ret != 0 );	// 串口可能由于正在发送其它内容而需要等待，直到发送命令成功
	
	do{
		ret = IF_ReveivedCmdResponse();	// 如果接收到命令回复
		count++;
		
		if( count > 10 ) {
			RepealCmd();
			ret = 3;	// 若接收不到回复超过10次，视为该命令执行失败
			goto enablewireless_exit;
		}
	}while( ret != 1 );	// 若不确定5毫秒内命令即可执行成功，可加入循环等待，确保模组执行命令完成
	ret = 0;
	// 此处仅有一行回复，故仅需要一次接收回复；
	// 若有多行回复数据，可使用循环处理，例如：for( i=0; i<p->cmd_response_maxr; i++ )
	buf = ReceiveCmdResponse( p );	// 使用字符指针接收命令的回复的内容
	if( buf == NULL ) {
		ret = 3;	// 如果命令回复内容为空
		goto enablewireless_exit;
	}
	
	sscanf( buf, "+CFUN: %c\r\n", &flag );
	flag -= '0';
	if( flag == 0 ) {	// 对回复内容进行处理（例如读出某个参数等）
		ret = 2;	// 如果命令执行不成功
	}

enablewireless_exit:
	FreeCmdPack( p );	// 释放掉此命令包
	return ret;	// 注：若命令执行成功返回值应为0
}


uint8_t AT_CheckAttachNetwork(void)
{
	uint8_t ret=0, count=0, flag=0;
	char *buf;
	CmdPack_t *p;
	
	// 传入AT指令，【模组需要回复的数据行】的行数，生成指令包
	p = NewCmdPack( "AT+CGATT?\r\n", 1 );
	if( p == NULL)
		return 1;
	// 注册回复内容的头（当串口接收到的数据行与这个头的匹配，即会把整个数据行放入命令数据包）
	//	此命令有两种可能的结果（命令回复）—— 成功或失败
	ret = RegisterCmdResponse( p, "+CGATT: %s\r\n" );	
	
	if( ret ) 
		goto attachnetwork_exit;
	
	do {
		ret = SendCmd( p, 20 );	// 发送命令成功之后延迟5毫秒
	}while( ret != 0 );	// 串口可能由于正在发送其它内容而需要等待，直到发送命令成功
	
	do{
		ret = IF_ReveivedCmdResponse();	// 如果接收到命令回复
		count++;
		if( count > 10 ) {
			RepealCmd();
			ret = 3;	// 若接收不到回复超过10次，视为该命令执行失败
			goto attachnetwork_exit;
		}
	}while( ret != 1 );	// 若不确定5毫秒内命令即可执行成功，可加入循环等待，确保模组执行命令完成
	ret = 0;
	// 此处仅有一行回复，故仅需要一次接收回复；
	// 若有多行回复数据，可使用循环处理，例如：for( i=0; i<p->cmd_response_maxr; i++ )
	buf = ReceiveCmdResponse( p );	// 使用字符指针接收命令的回复的内容
	if( buf == NULL ) {
		ret = 3;	// 如果命令回复内容为空
		goto attachnetwork_exit;
	}
	
	sscanf( buf, "+CGATT: %c\r\n", &flag );
	flag -= '0';
	if( flag == 0 ) {	// 对回复内容进行处理（例如读出某个参数等）
		ret = 2;	// 如果命令执行不成功
	}

attachnetwork_exit:
	FreeCmdPack( p );	// 释放掉此命令包
	return ret;	// 注：若命令执行成功返回值应为0
}


uint8_t AT_ReadSignalStrength( char *target )
{
	uint8_t ret=0, count=0;
	char *buf;
	CmdPack_t *p;
	
	// 传入AT指令，【模组需要回复的数据行】的行数，生成指令包
	p = NewCmdPack( "AT+CSQ\r\n", 1 );
	if( p == NULL)
		return 1;
	// 注册回复内容的头（当串口接收到的数据行与这个头的匹配，即会把整个数据行放入命令数据包）
	//	此命令有两种可能的结果（命令回复）—— 成功或失败
	ret = RegisterCmdResponse( p, "+CSQ:" );	
	
	if( ret ) 
		goto signalstrength_exit;
	
	do {
		ret = SendCmd( p, 40 );	// 发送命令成功之后延迟5毫秒
	}while( ret != 0 );	// 串口可能由于正在发送其它内容而需要等待，直到发送命令成功
	
	do{
		ret = IF_ReveivedCmdResponse();	// 如果接收到命令回复
		count++;
		if( count > 10 ) {
			RepealCmd();
			ret = 3;	// 若接收不到回复超过10次，视为该命令执行失败
			goto signalstrength_exit;
		}
	}while( ret != 1 );	// 若不确定5毫秒内命令即可执行成功，可加入循环等待，确保模组执行命令完成
	ret = 0;
	// 此处仅有一行回复，故仅需要一次接收回复；
	// 若有多行回复数据，可使用循环处理，例如：for( i=0; i<p->cmd_response_maxr; i++ )
	buf = ReceiveCmdResponse( p );	// 使用字符指针接收命令的回复的内容
	if( buf == NULL ) {
		ret = 3;	// 如果命令回复内容为空
		goto signalstrength_exit;
	}
	
	if( target != NULL )
	sscanf(buf, "%*[^:]: %[^\r]", target );	// 对回复内容进行处理（例如读出某个参数等）
	
signalstrength_exit:
	FreeCmdPack( p );	// 释放掉此命令包
	return ret;	// 注：若命令执行成功返回值应为0
}


uint8_t AT_ReadAttachIP( char *target )
{
	uint8_t ret=0, count=0;
	char *buf;
	CmdPack_t *p;
	
	// 传入AT指令，【模组需要回复的数据行】的行数，生成指令包
	p = NewCmdPack( "AT+CGPADDR?\r\n", 1 );
	if( p == NULL)
		return 1;
	// 注册回复内容的头（当串口接收到的数据行与这个头的匹配，即会把整个数据行放入命令数据包）
	//	此命令有两种可能的结果（命令回复）—— 成功或失败
	ret = RegisterCmdResponse( p, "+CGATT: %s\r\n" );	
	
	if( ret ) 
		goto attachip_exit;
	
	do {
		ret = SendCmd( p, 40 );	// 发送命令成功之后延迟5毫秒
	}while( ret != 0 );	// 串口可能由于正在发送其它内容而需要等待，直到发送命令成功
	
	do{
		ret = IF_ReveivedCmdResponse();	// 如果接收到命令回复
		count++;
		if( count > 10 ) {
			RepealCmd();
			ret = 3;	// 若接收不到回复超过10次，视为该命令执行失败
			goto attachip_exit;
		}
	}while( ret != 1 );	// 若不确定5毫秒内命令即可执行成功，可加入循环等待，确保模组执行命令完成
	ret = 0;
	// 此处仅有一行回复，故仅需要一次接收回复；
	// 若有多行回复数据，可使用循环处理，例如：for( i=0; i<p->cmd_response_maxr; i++ )
	buf = ReceiveCmdResponse( p );	// 使用字符指针接收命令的回复的内容
	if( buf == NULL ) {
		ret = 3;	// 如果命令回复内容为空
		goto attachip_exit;
	}
	
	if( target != NULL )
	sscanf(buf, "%*[^:]: %[^\r]", target );	// 对回复内容进行处理（例如读出某个参数等）
	
attachip_exit:
	FreeCmdPack( p );	// 释放掉此命令包
	return ret;	// 注：若命令执行成功返回值应为0
}


uint8_t AT_ConfigPlatformIP( const char *source )
{
	uint8_t ret=0, count=0, buf_temp[64] = {0};
	char *buf;
	CmdPack_t *p;
	
	if( source == NULL )
		return 1;
	snprintf( (char *)buf_temp, 64, "AT+QLWSERV=%s\r\n", source );
	// 传入AT指令，【模组需要回复的数据行】的行数，生成指令包
	p = NewCmdPack( (char *)buf_temp, 1 );
	if( p == NULL)
		return 1;
	// 注册回复内容的头（当串口接收到的数据行与这个头的匹配，即会把整个数据行放入命令数据包）
	//	此命令有两种可能的结果（命令回复）—— 成功或失败
	ret = RegisterCmdResponse( p, "OK" );	
	ret = RegisterCmdResponse( p, "ERROR" );
	if( ret ) 
		goto configip_exit;
	
	do {
		ret = SendCmd( p, 30 );	// 发送命令成功之后延迟5毫秒
	}while( ret != 0 );	// 串口可能由于正在发送其它内容而需要等待，直到发送命令成功
	
	do{
		ret = IF_ReveivedCmdResponse();	// 如果接收到命令回复
		count++;
		if( count > 10 ) {
			RepealCmd();
			ret = 3;	// 若接收不到回复超过10次，视为该命令执行失败
			goto configip_exit;
		}
	}while( ret != 1 );	// 若不确定5毫秒内命令即可执行成功，可加入循环等待，确保模组执行命令完成
	ret = 0;
	// 此处仅有一行回复，故仅需要一次接收回复；
	// 若有多行回复数据，可使用循环处理，例如：for( i=0; i<p->cmd_response_maxr; i++ )
	buf = ReceiveCmdResponse( p );	// 使用字符指针接收命令的回复的内容
	if( buf == NULL ) {
		ret = 3;	// 如果命令回复内容为空
		goto configip_exit;
	}
	
	if( strncmp( buf, "OK", 2) != 0 ) {	// 对回复内容进行处理（例如读出某个参数等）
		ret = 2;	// 如果命令执行不成功
	}
	
configip_exit:
	FreeCmdPack( p );	// 释放掉此命令包
	return ret;	// 注：若命令执行成功返回值应为0
}


uint8_t AT_ConfigEndPoint( const char *source )
{
	uint8_t ret=0, count=0, buf_temp[64] = {0};
	char *buf;
	CmdPack_t *p;
	
	if( source == NULL )
		return 1;
	snprintf( (char *)buf_temp, 64, "AT+QLWCONF=\"%s\"\r\n", source );
	// 传入AT指令，【模组需要回复的数据行】的行数，生成指令包
	p = NewCmdPack( (char *)buf_temp, 1 );
	if( p == NULL)
		return 1;
	// 注册回复内容的头（当串口接收到的数据行与这个头的匹配，即会把整个数据行放入命令数据包）
	//	此命令有两种可能的结果（命令回复）—— 成功或失败
	ret = RegisterCmdResponse( p, "OK" );	
	ret = RegisterCmdResponse( p, "ERROR" );
	if( ret ) 
		goto configendpoint_exit;
	
	do {
		ret = SendCmd( p, 20 );	// 发送命令成功之后延迟5毫秒
	}while( ret != 0 );	// 串口可能由于正在发送其它内容而需要等待，直到发送命令成功
	
	do{
		ret = IF_ReveivedCmdResponse();	// 如果接收到命令回复
		count++;
		if( count > 10 ) {
			RepealCmd();
			ret = 3;	// 若接收不到回复超过10次，视为该命令执行失败
			goto configendpoint_exit;
		}
	}while( ret != 1 );	// 若不确定5毫秒内命令即可执行成功，可加入循环等待，确保模组执行命令完成
	ret = 0;
	// 此处仅有一行回复，故仅需要一次接收回复；
	// 若有多行回复数据，可使用循环处理，例如：for( i=0; i<p->cmd_response_maxr; i++ )
	buf = ReceiveCmdResponse( p );	// 使用字符指针接收命令的回复的内容
	if( buf == NULL ) {
		ret = 3;	// 如果命令回复内容为空
		goto configendpoint_exit;
	}
	
	if( strncmp( buf, "OK", 2) != 0 ) {	// 对回复内容进行处理（例如读出某个参数等）
		ret = 2;	// 如果命令执行不成功
	}
	
configendpoint_exit:
	FreeCmdPack( p );	// 释放掉此命令包
	return ret;	// 注：若命令执行成功返回值应为0
}


uint8_t AT_Add_LwM2M_Object( const char *source )
{
	uint8_t ret=0, count=0, buf_temp[64] = {0};
	char *buf;
	CmdPack_t *p;
	
	if( source == NULL )
		return 1;
	snprintf( (char *)buf_temp, 64, "AT+QLWADDOBJ=%s\r\n", source );
	// 传入AT指令，【模组需要回复的数据行】的行数，生成指令包
	p = NewCmdPack( (char *)buf_temp, 1 );
	if( p == NULL)
		return 1;
	// 注册回复内容的头（当串口接收到的数据行与这个头的匹配，即会把整个数据行放入命令数据包）
	//	此命令有两种可能的结果（命令回复）—— 成功或失败
	ret = RegisterCmdResponse( p, "OK" );	
	ret = RegisterCmdResponse( p, "ERROR" );
	if( ret ) 
		goto addobject_exit;
	
	do {
		ret = SendCmd( p, 20 );	// 发送命令成功之后延迟5毫秒
	}while( ret != 0 );	// 串口可能由于正在发送其它内容而需要等待，直到发送命令成功
	
	do{
		ret = IF_ReveivedCmdResponse();	// 如果接收到命令回复
		count++;
		if( count > 10 ) {
			RepealCmd();
			ret = 3;	// 若接收不到回复超过10次，视为该命令执行失败
			goto addobject_exit;
		}
	}while( ret != 1 );	// 若不确定5毫秒内命令即可执行成功，可加入循环等待，确保模组执行命令完成
	ret = 0;
	// 此处仅有一行回复，故仅需要一次接收回复；
	// 若有多行回复数据，可使用循环处理，例如：for( i=0; i<p->cmd_response_maxr; i++ )
	buf = ReceiveCmdResponse( p );	// 使用字符指针接收命令的回复的内容
	if( buf == NULL ) {
		ret = 3;	// 如果命令回复内容为空
		goto addobject_exit;
	}
	
	if( strncmp( buf, "OK", 2) != 0 ) {	// 对回复内容进行处理（例如读出某个参数等）
		ret = 2;	// 如果命令执行不成功
	}
	
addobject_exit:
	FreeCmdPack( p );	// 释放掉此命令包
	return ret;	// 注：若命令执行成功返回值应为0
}


uint8_t AT_OpenConnectPlatform(void)
{
	uint8_t ret=0, count=0;
	char *buf;
	CmdPack_t *p;
	
	// 传入AT指令，【模组需要回复的数据行】的行数，生成指令包
	p = NewCmdPack( "AT+QLWOPEN=0\r\n", 1 );
	if( p == NULL)
		return 1;
	// 注册回复内容的头（当串口接收到的数据行与这个头的匹配，即会把整个数据行放入命令数据包）
	//	此命令有两种可能的结果（命令回复）—— 成功或失败
	ret = RegisterCmdResponse( p, "OK" );	
	ret = RegisterCmdResponse( p, "ERROR" );
	if( ret ) 
		goto openconnect_exit;
	
	do {
		ret = SendCmd( p, 3000 );	// 发送命令成功之后延迟5毫秒
	}while( ret != 0 );	// 串口可能由于正在发送其它内容而需要等待，直到发送命令成功
	
	do{
		ret = IF_ReveivedCmdResponse();	// 如果接收到命令回复
		count++;
		if( count > 10 ) {
			RepealCmd();
			ret = 3;	// 若接收不到回复超过10次，视为该命令执行失败
			goto openconnect_exit;

		}
	}while( ret != 1 );	// 若不确定5毫秒内命令即可执行成功，可加入循环等待，确保模组执行命令完成
	ret = 0;
	// 此处仅有一行回复，故仅需要一次接收回复；
	// 若有多行回复数据，可使用循环处理，例如：for( i=0; i<p->cmd_response_maxr; i++ )
	buf = ReceiveCmdResponse( p );	// 使用字符指针接收命令的回复的内容
	if( buf == NULL ) {
		ret = 3;	// 如果命令回复内容为空
		goto openconnect_exit;
	}
	
	if( strncmp( buf, "OK", 2) != 0 ) {	// 对回复内容进行处理（例如读出某个参数等）
		ret = 2;	// 如果命令执行不成功
	}
	
openconnect_exit:
	FreeCmdPack( p );	// 释放掉此命令包
	return ret;	// 注：若命令执行成功返回值应为0
}


uint8_t AT_Config_LwM2M_Dataformat(void)
{
	uint8_t ret=0, count=0;
	char *buf;
	CmdPack_t *p;
	
	// 传入AT指令，【模组需要回复的数据行】的行数，生成指令包
	p = NewCmdPack( "AT+QLWCFG=\"dataformat\",1,1\r\n", 1 );
	if( p == NULL)
		return 1;
	// 注册回复内容的头（当串口接收到的数据行与这个头的匹配，即会把整个数据行放入命令数据包）
	//	此命令有两种可能的结果（命令回复）—— 成功或失败
	ret = RegisterCmdResponse( p, "OK" );	
	ret = RegisterCmdResponse( p, "ERROR" );
	if( ret ) 
		goto dataformat_exit;
	
	do {
		ret = SendCmd( p, 200 );	// 发送命令成功之后延迟5毫秒
	}while( ret != 0 );	// 串口可能由于正在发送其它内容而需要等待，直到发送命令成功
	
	do{
		ret = IF_ReveivedCmdResponse();	// 如果接收到命令回复
		count++;
		if( count > 10 ) {
			RepealCmd();
			ret = 3;	// 若接收不到回复超过10次，视为该命令执行失败
			goto dataformat_exit;
		}
	}while( ret != 1 );	// 若不确定5毫秒内命令即可执行成功，可加入循环等待，确保模组执行命令完成
	ret = 0;
	// 此处仅有一行回复，故仅需要一次接收回复；
	// 若有多行回复数据，可使用循环处理，例如：for( i=0; i<p->cmd_response_maxr; i++ )
	buf = ReceiveCmdResponse( p );	// 使用字符指针接收命令的回复的内容
	if( buf == NULL ) {
		ret = 3;	// 如果命令回复内容为空
		goto dataformat_exit;
	}
	
	if( strncmp( buf, "OK", 2) != 0 ) {	// 对回复内容进行处理（例如读出某个参数等）
		ret = 2;	// 如果命令执行不成功
	}
	
dataformat_exit:
	FreeCmdPack( p );	// 释放掉此命令包
	return ret;	// 注：若命令执行成功返回值应为0
}


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
	
	p->cmd_response_max = response_n;
	memcpy( p->cmd, cmd, data_length );
	return p;
}


uint8_t FreeCmdPack( CmdPack_t *pack )
{
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
	cmd_response_t *p=NULL;
	
	if( pack == NULL || response == NULL )
		return 1;
	
	if( pack->reg_response == NULL ) {
		pack->reg_response = calloc( 1, sizeof(cmd_response_t) );
		p = pack->reg_response;
	}else {
		for( p = pack->reg_response; p->next != NULL; p = p->next );
		p->next = calloc( 1, sizeof(cmd_response_t) );
		p = p->next;
	}
	if( p == NULL )
		return 2;	// calloc failed.
	
	data_length = strlen( response );
	p->response = calloc( data_length+1, 1 );
	if( p->response == 	NULL )
		return 2;	// calloc failed.
	memcpy(p->response, response, data_length);
	
	return 0;
}


char* ReceiveCmdResponse( CmdPack_t *pack )
{
	static cmd_response_t *now_response;
	cmd_response_t *p;
	
	if( pack == NULL ) {
		if(now_response != NULL)
			now_response = now_response->next;
	}else
		now_response = pack->cmd_response;
	
	if( now_response == NULL )
		return NULL;
	
	return now_response->response;
}



