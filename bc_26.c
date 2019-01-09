#include "bc_26.h"


#define AT_SEND_END 	",0X0100\r\n"
#define AT_SEND_HEAD_LEN 22



// ——————————————    AT 指令列表    —————————————— //

typedef enum
{
	AT_LOCK = 0,
	CLOSE_ECHO = 1,
	GET_IMEI,
	GET_IMSI,
	CHECK_WIRELESS_WORK,
	CHECK_NET,
	CHECK_SINGAL,
	GET_DEV_IP,	// local device's ip
	SHOW_ERROR_CODE,
	SERVER_IP,
	OBJ_UP,
	OBJ_DOWN,
	IOT_OPEN,
	DATA_FORMAT,
} AT_CMD_Index;

StringList  AT_CMD[AT_CMD_MAX] = {
	"AT+SM=LOCK\r\n",	// 锁定AT指令响应
	"ATE0\r\n",	// 关闭指令消息echo
	"AT+CGSN=1\r\n",
	"AT+CIMI\r\n",
	"AT+CFUN?\r\n",
	"AT+CGATT?\r\n",
	"AT+CSQ\r\n",
	"AT+CGPADDR?\r\n",
	"AT+CMEE=1\r\n",	// 显示错误码
	"AT+QLWSERV=117.60.157.137,5683\r\n",	// config IOT platform Server IP
	"AT+QLWADDOBJ=19,0,1,\"0\"\r\n",		// Add LWM2M Object (for upload message)
	"AT+QLWADDOBJ=19,1,1,\"0\"\r\n",		// Add LWM2M Object (for download message)
	"AT+QLWOPEN=0\r\n",	// Connect to IOT platform
	"AT+QLWCFG=\"dataformat\",1,1\r\n",	// config communication data format (HEX string)
};

// ——————————————    需填入内容的 AT 指令列表      —————————————— //

typedef enum 
{
	SET_IMEI = 0,
	SEN_MES,
}AT_Head_Index;

StringList AT_Head[AT_HEAD_MAX] = {
	"AT+QLWCONF=\"%s\"\r\n",		// config device name（IMEI）
	"AT+QLWDATASEND=19,0,0,%s,"	// Send message to IOT platform
};

// ——————————————    AT 指令的回复内容    —————————————— //

typedef enum 
{
	OBJ_IP = 0,
	REC_MES,
	REC_IMEI,
	REC_IMSI,
	WIRELESS_FLAG,
	NET_FLAG,
	SINGAL_STRENGTH,
	CON_OK,
	SEN_OK,
	SEN_FAIL,
	GEN_OK,
	GEN_ERROR,
}AT_Receive_Event_Define;

StringList AT_Receive_Head[NB_REC_MAX] = {
	"+QLWOBSERVE:%s\r\n",		// Receive Object Server IP(for send of receive coap message)
	"+QLWDATARECV:%s\r\n",	// Receive message head
	"+CGSN:%d\r\n",	//	Receive IMEI
	"460",	//	Receive IMSI
	"+CSQ:"	// Receive Singal strength
	"+CFUN: %d\r\n",	// Wireless work or not flag ( '1' is work)
	"+CGATT: %d\r\n",	// Network of communication to IOT platform work or not flag ('1' is work)
	"CONNECT OK\r\n",	// Connect to IOT platform succeed response
	"SEND OK",	// Send ok (of any message)  response
	"SEND FAILD",
	"OK",
	"ERROR",
};

// ——————————————    NB模组接收事件通知表    —————————————— //

typedef enum 
{
	rec_general_ok = 0,
	rec_general_error,
	rec_wireless_ok,
	rec_network_ok,
	rec_singal_strength,
	rec_connect_ok,
	rec_send_ok,
	rec_send_fail,
	rec_imei,
	rec_imsi,
	rec_obj_ip,
}AT_REC_event_notify;


// ——————————————    本地变量    —————————————— //

uint8_t REC_Event_Notification[16];
#define EventARG_LENGTH 32
uint8_t REC_Event_Arg[EventARG_LENGTH];



// ——————————————    本地函数    —————————————— //

uint8_t check_SendMessage_succeed(void);
uint8_t lock_AT_command( NB_Port_Callback_t funtion);
uint8_t close_AT_echo( NB_Port_Callback_t funtion );
uint8_t show_ErrorCode( NB_Port_Callback_t funtion );
uint8_t set_platform_ip(NB_Port_Callback_t funtion);
uint8_t check_WireLess_work(NB_Port_Callback_t funtion);
uint8_t check_AttachedNet_work(NB_Port_Callback_t funtion);
uint8_t set_platform_endpoint(char *imei, NB_Port_Callback_t funtion);
uint8_t read_IMEI(char *out_buf, NB_Port_Callback_t funtion);
uint8_t read_IMSI(char *out_buf, NB_Port_Callback_t funtion);
uint8_t read_SingalStrength(char *out_buf, NB_Port_Callback_t funtion);
uint8_t open_IOT_platform(char *out_buf, NB_Port_Callback_t funtion);


uint8_t check_SendMessage_failed(void)
{
	uint8_t ret=0;
	
	if( REC_Event_Notification[rec_send_ok] && REC_Event_Notification[rec_general_ok] )
	{
		REC_Event_Notification[rec_send_ok] = 0;
		REC_Event_Notification[rec_general_ok] = 0;
		ret = 0;
	}else if( REC_Event_Notification[rec_send_fail] && REC_Event_Notification[rec_general_ok] )
	{
		REC_Event_Notification[rec_general_ok] = 0;
		REC_Event_Notification[rec_send_fail] = 0;
		ret = 1;
	}else if( REC_Event_Notification[rec_general_error] )
	{
		REC_Event_Notification[rec_general_error] = 0;
		ret = 1;
	}
	
	return ret;
}

uint8_t lock_AT_command( NB_Port_Callback_t function)
{
	uint8_t ret=1;
	
	if(function != NULL) {
		function( AT_CMD[AT_LOCK] );
		if( REC_Event_Notification[rec_general_ok] ) {
			REC_Event_Notification[rec_general_ok] = 0;
			ret = 0;
		}
		else ret = 1;
	}
	
	return ret;
}

uint8_t close_AT_echo( NB_Port_Callback_t function )
{
	uint8_t ret=1;
	
	if(function != NULL) {
		function( AT_CMD[CLOSE_ECHO] );
		if( REC_Event_Notification[rec_general_ok] ) {
			REC_Event_Notification[rec_general_ok] = 0;
			ret = 0;
		}
		else ret = 1;
	}
	
	return ret;
}

uint8_t show_ErrorCode( NB_Port_Callback_t function )
{
	uint8_t ret=1;
	
	if(function != NULL) {
		function( AT_CMD[SHOW_ERROR_CODE] );
		if( REC_Event_Notification[rec_general_ok] ) {
			REC_Event_Notification[rec_general_ok] = 0;
			ret = 0;
		}
		else ret = 1;
	}
	
	return ret;
}


uint8_t set_platform_ip(NB_Port_Callback_t function)
{
	uint8_t ret=1;
	
	if(function != NULL) {
		function( AT_CMD[SERVER_IP] );
		if( REC_Event_Notification[rec_general_ok] ) {
			REC_Event_Notification[rec_general_ok] = 0;
			ret = 0;
		}
		else ret = 1;
	}
	
	return ret;
}

uint8_t check_WireLess_work(NB_Port_Callback_t function)
{
	uint8_t ret=1;
	
	if(function != NULL) {
		function( AT_CMD[CHECK_WIRELESS_WORK] );
		if( REC_Event_Notification[rec_network_ok] ) {
			REC_Event_Notification[rec_network_ok] = 0;
			ret = 0;
		}
		else ret = 1;
	}
	
	return ret;
}

uint8_t check_AttachedNet_work(NB_Port_Callback_t function)
{
	uint8_t ret=1;
	
	if(function != NULL) {
		function( AT_CMD[CHECK_NET] );
		if( REC_Event_Notification[rec_network_ok] ) {
			REC_Event_Notification[rec_network_ok] = 0;
			ret = 0;
		}
		else ret = 1;
	}
	
	return ret;
}

uint8_t set_platform_endpoint(char *imei, NB_Port_Callback_t function)
{
	uint8_t ret=1, buf[64]={0};
	
	if(function != NULL) {
		sprintf(buf, AT_Head[SET_IMEI], imei);
		function( buf );
		if( REC_Event_Notification[rec_general_ok] ) {
			REC_Event_Notification[rec_general_ok] = 0;
			ret = 0;
		}
		else ret = 1;
	}
	
	return ret;
}

uint8_t read_IMEI(char *out_buf, NB_Port_Callback_t function)
{
	uint8_t ret=1;
	
	if(function != NULL) {
		function( AT_CMD[GET_IMEI] );
		if( REC_Event_Notification[rec_imei] ) {
			REC_Event_Notification[rec_imei] = 0;
			ret = 0;
			strcpy( out_buf, REC_Event_Arg );	// Load out IMEI.
		}
		else ret = 1;
	}
	
	return ret;
}

uint8_t read_IMSI(char *out_buf, NB_Port_Callback_t function)
{
	uint8_t ret=1;
	
	if(function != NULL) {
		function( AT_CMD[GET_IMSI] );
		if( REC_Event_Notification[rec_imsi] ) {
			REC_Event_Notification[rec_imsi] = 0;
			ret = 0;
			strcpy( out_buf, REC_Event_Arg );	// Load out IMSI.
		}
		else ret = 1;
	}
	
	return ret;
}

uint8_t read_SingalStrength(char *out_buf, NB_Port_Callback_t function)
{
	uint8_t ret=1;
	
	if(function != NULL) {
		function( AT_CMD[SINGAL_STRENGTH] );
		if( REC_Event_Notification[rec_singal_strength] ) {
			REC_Event_Notification[rec_singal_strength] = 0;
			ret = 0;
			strcpy( out_buf, REC_Event_Arg );	// Load out Singal strength.
		}
		else ret = 1;
	}
	
	return ret;
}


uint8_t open_IOT_platform(char *out_buf, NB_Port_Callback_t function)
{
	uint8_t ret=1;
	size_t i;
	
	if(function != NULL) {
		function( AT_CMD[OBJ_UP] );	
		function( AT_CMD[OBJ_DOWN] );
		function( AT_CMD[IOT_OPEN] );
		
		for(i=0; i<65535; i++);	// more delay
		if( REC_Event_Notification[rec_connect_ok] ) {
			REC_Event_Notification[rec_connect_ok] = 0;
			ret = 0;
		}
		else ret = 1;
		
		for(i=0; i<1024; i++);	// more delay
		if( REC_Event_Notification[rec_obj_ip] ) {
			REC_Event_Notification[rec_obj_ip] = 0;
			ret = 0;
			strcpy( out_buf, REC_Event_Arg );	// Load out Singal strength.
		}
		
		function(  AT_CMD[DATA_FORMAT] );	// set transmission data format
		REC_Event_Notification[rec_general_ok] = 0;
	}
	
	return ret;
}



// ——————————————    公开函数    —————————————— //

uint8_t INIT_NB(uint8_t *out_imei, uint8_t *out_imsi, NB_Port_Callback_t function )
{
	uint8_t temp;
	
	if( function == NULL) {
		LOGE("INIT_NB error: nb serial port callback empty, please regiseter callback function.\n");
		return 1;
	}
		
	Execute_NB_CMD( AT_CLOSE_ECHO, NULL, function );
	Execute_NB_CMD( AT_LOCK_Freqency, NULL, function );
	Execute_NB_CMD( AT_SHOW_ErrorCode, NULL, function );
	
	temp = Execute_NB_CMD( AT_CHECK_WireLess, NULL, function );
	if( temp ) { LOGE("INIT_NB error: Wireless not work.\n"); return temp; }
	temp = Execute_NB_CMD( AT_CHECK_AttachedNet, NULL, function );
	if( temp ) { LOGE("INIT_NB error: can't not attach to network.\n"); return temp; }
	
	//Execute_NB_CMD( AT_READ_SingalStrength, NULL, function );
	temp = Execute_NB_CMD( AT_READ_IMEI, out_imei, function );
	if( temp ) { LOGE("INIT_NB error: can't not read IMEI.\n"); return temp; }
	temp = Execute_NB_CMD( AT_READ_IMSI, out_imsi, function );
	if( temp ) { LOGE("INIT_NB error: can't not read IMSI.\n"); return temp; }
	
	Execute_NB_CMD( AT_SET_PlatformIP, NULL, function );
	temp = Execute_NB_CMD( AT_OPEN_Platform, NULL, function );
	if( temp ) LOGE("INIT_NB error: can't regiseter to IOT platform.\n");
	return temp;
}


uint16_t GET_SendMessage_Head(uint8_t *out_buf, uint16_t message_length)
{
	uint8_t temp, buf[48] = {0}, coap_length[8]={0};
	
	Hex_2_DecString( (char*)coap_length, message_length );
	sprintf(buf, AT_Head[SEN_MES], coap_length);
	temp = strlen(buf);
	memcpy(out_buf, buf, temp);
	return temp;
}


uint16_t GET_SendMessage_End(uint8_t *out_buf, uint16_t message_length)
{
	uint8_t temp;
	
	temp = strlen(AT_SEND_END);
	memcpy(out_buf, AT_SEND_END, temp);
	return temp;
}



uint8_t ACK_NB_REC(uint8_t *rec_buf, NB_Port_Callback_t function)
{
	char *p, buf[512], rec_evt;
	size_t i, temp, *pos;
	
	p = strtok(rec_buf, "\r\n");
	while(p != NULL) {
		
		for(i=0; i<NB_REC_MAX; i++)
			if ( strncmp(p, AT_Receive_Head[i], 6) == 0 ) break;
		
		if(i == NB_REC_MAX) {
			//LOGE("ACK_NB_REC error: can't match any receive enent.\n");
			rec_evt = 0xFF;
		}
		else rec_evt = i;
		//LOGI("evt[%d] \n",rec_evt);
		
		pos = (size_t *)buf;
		if(rec_evt < CON_OK) {
			if(rec_evt == REC_MES)	{
				for(i=0; i<512/sizeof(size_t); pos[i++]=0);
				sscanf(p, "%*[^,],%*[^,],%*[^,],%*[^,],%[^\r]", buf);
			}else {
				for(i=0; i<64/sizeof(size_t); pos[i++]=0);
				sscanf(p, "%*[^:]: %[^\r]", buf);
			}
			//printf("buf[%d]: %s\n", strlen(buf),buf);
		}
		
		switch(rec_evt)  {
			case OBJ_IP:
				memset(REC_Event_Arg, 0, EventARG_LENGTH);
				strcpy(REC_Event_Arg, buf);
				REC_Event_Notification[rec_obj_ip] = 1;
				break;
			case REC_IMEI:
				memset(REC_Event_Arg, 0, EventARG_LENGTH);
				memcpy(REC_Event_Arg, buf, 15);
				REC_Event_Notification[rec_imei] = 1;
				break;
			case REC_IMSI:
				memset(REC_Event_Arg, 0, EventARG_LENGTH);
				memcpy(REC_Event_Arg, buf, 15);
				REC_Event_Notification[rec_imsi] = 1;
				break;
			case SINGAL_STRENGTH:
				memset(REC_Event_Arg, 0, EventARG_LENGTH);
				memcpy(REC_Event_Arg, buf, 15);
				REC_Event_Notification[rec_singal_strength] = 1;
				break;
				
			case REC_MES:
				if( function != NULL) function(buf);
				break;
				
			case WIRELESS_FLAG: 
			case NET_FLAG: 
				REC_Event_Notification[rec_network_ok] = buf[0] > '0' ? 1 : 0;
				break;
				
			case CON_OK: REC_Event_Notification[rec_connect_ok] = 1;
				break;
			case SEN_OK: REC_Event_Notification[rec_send_ok] = 1;
				break;
			case SEN_FAIL: REC_Event_Notification[rec_send_fail] = 1;
				break;
			case GEN_OK: REC_Event_Notification[rec_general_ok] = 1;
				break;
			case GEN_ERROR: REC_Event_Notification[rec_general_error] = 1;
				break;
		}
		p = strtok(NULL, "\n");
	}
	
	return 0;
}


uint8_t Execute_NB_CMD(uint8_t cmd_name, void *arg, NB_Port_Callback_t function)
{
	switch(cmd_name) {
		case AT_CLOSE_ECHO:
			return close_AT_echo( function );
		case AT_LOCK_Freqency:
			return  lock_AT_command( function );
		case AT_SHOW_ErrorCode:
			return show_ErrorCode( function );
		case AT_CHECK_WireLess:
			return check_WireLess_work( function );
		case AT_CHECK_AttachedNet:
			return check_AttachedNet_work( function );
		case AT_READ_SingalStrength:
			return read_SingalStrength( (char *)arg, function );
		case AT_READ_IMEI:
			return read_IMEI( (char *)arg, function );
		case AT_READ_IMSI:
			return read_IMSI( (char *)arg, function );
		case AT_SET_PlatformIP:
			return set_platform_endpoint( (char *)arg, function );
		case AT_OPEN_Platform:
			return open_IOT_platform( (char *)arg, function );
		case CHECK_SendMessage_Failed:
			return check_SendMessage_failed();
	}
}
