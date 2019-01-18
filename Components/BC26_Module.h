/* 
	
*/
#ifndef IOT_API__COMPONENT__BC26_MODULE_H
#define IOT_API__COMPONENT__BC26_MODULE_H

#include <stdint.h>



// ——————————————   接口数据格式     —————————————— //

/* 发送吗命令时，需要把命令的内容，和该命令（NB模组的）回复内容，
	打包成以下的数据格式。
*/

typedef struct cmd_response_
{
	char *response;
	struct cmd_response_ *next;
}cmd_response_t;	// “命令回复”数据包

typedef struct cmd_pack_
{
	char *cmd;
	uint8_t cmd_response_max;
	uint8_t cmd_response_n;
	cmd_response_t *reg_response;
	cmd_response_t *cmd_response;
}CmdPack_t;	// “命令”数据包


// ——————————————    接口变量    —————————————— //



// ——————————————   接口函数     —————————————— //


uint8_t IF_ReceiveMessage(void);
uint8_t IF_ReveivedCmdResponse(void);
uint8_t IF_SendMessageError(void);

uint8_t SendMessage( uint8_t *buff, uint8_t data_length );
uint8_t ReceiveMessage( uint8_t *out_buff, uint8_t *data_length );
uint8_t SendCmd( CmdPack_t *cmd, uint16_t delay );
CmdPack_t* ReceiveCmd	(void);

uint8_t WriteData_ToSerialPort( void );
uint8_t ReadData_FromSerialPort( void );

// ——————————————    END    —————————————— //


#endif
