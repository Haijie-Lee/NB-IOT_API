/* *Copyright(C), Haijie Lee. 2019. AllRightsReserved.
	*FileName: NB模组命令 模块源文件.
	*Author:  Haijie Lee.
	*Version:  1.5.
	*Date:  2019.01.22.
	*Description:  实现对于NB模组发送某些用户需要的的命令。
							本模块依赖 <common>提供错误信息输出功能、
							依赖<BCXX_AT>提供的NB模组的AT指令。
	*Others:  用户不应随意修改IOT定义的数据，除非你知道你在做什么。
*/
#include "nb_cmd.h"
#include "common_function.h"

#if NB_MODEL_VERSION == BC_26	
	#include "BC26_AT.h"
#endif
 
#include <string.h>
#include <stdlib.h>
#include <stdio.h>



// ——————————————    本地设置    —————————————— //

// ——————————————    本地定义    —————————————— //

// ——————————————    本地变量   —————————————— //

// ——————————————    本地函数    —————————————— //

/* 检查函数的返回值，若返回值不等于0则表示函数执行出错，并返回当前的错误码 */
#define CHECK_RETURN(ret)	do{\
	if( ret != 0 )	{	\
		LOGE("AT_[%s] error[%d]\n", __func__, ret);	\
		return ret;	\
	}	\
}while(0)

// ——————————————    接口函数源    —————————————— //

uint8_t Init_NB(char *output_imei, char *output_imsi )
{
	uint8_t ret;
	char imei[16]={0}, imsi[16]={0};
	
	ret = AT_SleepMode_Lock();
	CHECK_RETURN( ret );
	
	ret = AT_CloseEcho();
	CHECK_RETURN( ret );
	
	ret = AT_ReadIMEI( imei );
	CHECK_RETURN( ret );
	memcpy( output_imei, imei, 15);
	
	ret = AT_ReadIMSI( imsi );
	CHECK_RETURN( ret );
	memcpy( output_imsi, imsi, 15);
	
	ret = AT_EnableWireless();
	CHECK_RETURN( ret );
	
	ret = AT_CheckAttachNetwork();
	CHECK_RETURN( ret );
	
	ret = Connnect_IOT_Platfrom( (const char *)imei, NULL );
	CHECK_RETURN( ret );
	
	return 0;
}

uint8_t Check_Network(void)
{
	uint8_t ret;
	
	return 0;
}

uint8_t Read_IMEI( char *destination )
{
	uint8_t ret;
	
	return 0;
}

uint8_t Read_IMSI( char *destination )
{
	uint8_t ret;
	
	return 0;
}

uint8_t Read_SingalQuality( char *destination )
{
	uint8_t ret;
	
	return 0;
}

uint8_t Connnect_IOT_Platfrom( const char *imei, char *obj_server )
{
	uint8_t ret;
	const char *platformIP = "117.60.157.137,5683";
	const char *upload_object = "19,0,1,\"0\"";
	const char *download_object = "19,1,1,\"0\"";
	
	ret = AT_ConfigPlatformIP( platformIP );
	CHECK_RETURN( ret );
	
	ret = AT_ConfigEndPoint( imei );
	CHECK_RETURN( ret );
	
	ret = AT_Add_LwM2M_Object( upload_object );
	CHECK_RETURN( ret );
	
	ret =  AT_Add_LwM2M_Object( download_object );
	CHECK_RETURN( ret );
	
	ret  = AT_OpenConnectPlatform();
	CHECK_RETURN( ret );
	
	ret = AT_Config_LwM2M_Dataformat();
	CHECK_RETURN( ret );
	
	return 0;
}


uint8_t Disconnnect_IOT_Platfrom(void)
{
	uint8_t ret;
	
	return 0;
}

/*
uint8_t Check_SendCmdError(void)
{
	uint8_t ret;
	
	ret = ReadData_FromSerialPort();
	if( ret > 1 ) {
		LOGE("ReadData_FromSerialPort function error[%d].\n", ret);
	}
	ret = IF_ReceiveMessage();
	return ret;
}*/

// ——————————————    本地函数源    —————————————— //

