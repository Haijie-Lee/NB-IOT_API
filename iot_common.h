#ifndef __IOT_COMMON_H
#define __IOT_COMMON_H

#include <stdint.h>
#include "iot_user.h"


#ifndef LOGI
#define LOGI printf
#endif

#ifndef LOGE
#define LOGE printf
#endif



#ifdef USE_QUEUE

typedef struct message_pack
{
	uint16_t message_length;
	uint8_t *message;
	struct message_pack *next;
}Message_pack;

typedef struct 
{
	uint16_t queue_n;
	Message_pack *member;
}MsgQueue;

/*
typedef enum {
	UploadQueue = 0x01,
	DownloadQueue,
}Queue_name_define;
*/

#define MES_Queue_MAX 10	// 列队的最大队员数量

#endif


#ifdef USE_ENDIAN_CONVERT

// 对齐访问 Float
typedef union FLOAT_CONV
{
	float f;
	char c[4];
}floatConv;

// 对齐访问 Double
typedef union DOUBLE_CONV
{
	double d;
	char c[8];
}doubleConv;

#endif

// ——————————————    函数    —————————————— //

uint8_t Hex_2_DecString(char *output, uint16_t hex);
uint16_t DecString_2_Hex(char* input, uint8_t string_length);
uint16_t HexArray_2_HexString(char* input, char* output, uint16_t conver_length);
uint16_t HexString_2_HexArray(char* input, char* output, uint16_t conver_length);
uint16_t DecString_2_HexArray(char* input, char* output, uint16_t conver_length);

#ifdef USE_QUEUE
uint8_t Push_Queue(uint8_t queue_name, char *buf, uint16_t buf_length);
uint8_t Pop_Queue(uint8_t queue_name);
Message_pack* Read_Queue(uint8_t queue_name);
uint8_t IsEmpty_Queue(uint8_t queue_name);
uint8_t IsFull_Queue(uint8_t queue_name);
#endif


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
