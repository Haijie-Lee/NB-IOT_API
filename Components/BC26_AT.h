/* 
	
*/
#ifndef IOT_API__COMPONENT__BC26_AT_H
#define IOT_API__COMPONENT__BC26_AT_H

#include <stdint.h>



// ——————————————    接口定义    —————————————— //


// ——————————————    接口函数    —————————————— //

uint8_t AT_SleepMode_Lock(void);
uint8_t AT_CloseEcho(void);
uint8_t AT_ReadIMEI( char *target );
uint8_t AT_ReadIMSI( char *target );
uint8_t AT_EnableWireless(void);
uint8_t AT_CheckAttachNetwork(void);
uint8_t AT_ReadSignalStrength( char *target );
uint8_t AT_ReadAttachIP( char *target );
uint8_t AT_ConfigPlatformIP( const char *source );
uint8_t AT_ConfigEndPoint( const char *source );
uint8_t AT_Add_LwM2M_Object( const char *source );
uint8_t AT_OpenConnectPlatform(void);
uint8_t AT_Config_LwM2M_Dataformat(void);

// ——————————————   END   —————————————— //

#endif
