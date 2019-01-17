/* 
	
*/
#ifndef   IOT_API__COMPONENT__COMMON_FUNCTION_H
#define  IOT_API__COMPONENT__COMMON_FUNCTION_H

#include <stdint.h>

// ——————————————    接口定义    —————————————— //

#ifndef LOGI
#define LOGI printf
#endif

#ifndef LOGE
#define LOGE printf
#endif


// ——————————————    接口函数    —————————————— //

uint8_t Hex_2_DecString(char *output, uint16_t hex);
uint16_t DecString_2_Hex(char* input, uint8_t string_length);
uint16_t HexArray_2_HexString(char* input, char* output, uint16_t conver_length);
uint16_t HexString_2_HexArray(char* input, char* output, uint16_t conver_length);
uint16_t DecString_2_HexArray(char* input, char* output, uint16_t conver_length);


#ifdef USE_ENDIAN_CONVERT

float host2NetFloat(float value);
double host2NetDouble(double value);
int16_t host2NetInt16(int16_t value);
int32_t host2NetInt32(int32_t value);
int64_t host2NetInt64(int64_t value);

float net2HostFloat(float value);
double net2HostDouble(double value);
int16_t net2HostInt16(int16_t value);
int32_t net2HostInt32(int32_t value);
int64_t net2HostInt64(int64_t value);

#endif

// ——————————————   END    —————————————— //

#endif
