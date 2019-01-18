/* 
	
*/
#ifndef IOT_API__COMPONENT__NB_CMD_H
#define IOT_API__COMPONENT__NB_CMD_H

#include <stdint.h>



// ——————————————    接口定义    —————————————— //


// ——————————————    接口函数    —————————————— //

uint8_t Init_NB(char *output_imei, char *output_imsi );
uint8_t Check_Network(void);
uint8_t Read_IMEI( char *destination );
uint8_t Read_IMSI( char *destination );
uint8_t Read_SingalQuality( char *destination );
uint8_t Connnect_IOT_Platfrom( const char *imei, char *obj_server );
uint8_t Disconnnect_IOT_Platfrom(void);

uint8_t Check_SendCmdError();

// ——————————————   END   —————————————— //

#endif
