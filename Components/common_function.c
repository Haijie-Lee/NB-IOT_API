/* *Copyright(C), Haijie Lee. 2019. AllRightsReserved.
	*FileName:  用户编程入口文件.
	*Author:  Haijie Lee.
	*Version:  1.5.
	*Date:  2019.01.22.
	*Description:  初始化IOT通信的数据内容, 以供用户编程·。
							本模块依赖<iot_user>中的功能开关决定是否编译对应的功能。
	*Others:  用户不应随意修改IOT定义的数据，除非你知道你在做什么。
*/
#include "iot_user.h"
#include "common_function.h"

#include <stdlib.h>
#include <stdio.h>


// ——————————————    本地定义    —————————————— //

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

// ——————————————    本地变量   —————————————— //


// ——————————————    本地函数    —————————————— //


// ——————————————    接口函数源    —————————————— //

uint8_t Hex_2_DecString(char *output, uint16_t hex)
{
	uint8_t i, temp, set[10] = {0};
	uint32_t n=hex;
	
	for(i=0; i<10; i++) {
		if(n == 0) break;
		set[i] = n % 10;
		n /= 10;
	}
	
	temp = i;
	for(i=0; i<temp; i++) {
		output[i] = set[temp-i-1]+'0';
	}
	output[i] = '\0';
	return temp;		// string_length
}


uint16_t DecString_2_Hex(char* input, uint8_t string_length)
{
	uint16_t i, temp=0;
	uint8_t set[10] = {0};
	uint32_t n=1;
	
	for(i=0; i<string_length; i++) {
		set[i] = input[string_length - 1 - i];
	}
	for(i=0; i<string_length; i++) {
		temp += (set[i]-'0') * n;
		n *= 10;
	}
	return temp;		// hex
}


uint16_t HexArray_2_HexString(char* input, char* output, uint16_t conver_length)
{
	uint8_t temp;
	uint16_t i;
	// The output buffer size must be twice the input buffer size.
	for(i=0; i<conver_length; i++)
	{
		temp = (input[i]&0xF0)>>4;
		output[2*i] = temp > 9 ?   temp+0x37 : temp + 0x30;
		temp = input[i]&0x0F;
		output[2*i+1] = temp > 9 ?  temp+0x37 : temp + 0x30;
	}
	output[2*i] = '\0';
	return conver_length*2;
}


uint16_t HexString_2_HexArray(char* input, char* output, uint16_t conver_length)
{
	uint16_t i;
	
	if( conver_length%2 != 0 )
		LOGE("HexString_2_HexArray Warning: conver_length must be a multiple of 2.\n");
	for(i=0; i<conver_length; i+=2)
	{
		output[i/2] = 16 * ( input[i] >= 'A' ?  input[i] - 0x37 : input[i] - 0x30 );
		output[i/2] += input[i+1] >= 'A' ? input[i+1] - 0x37 : input[i+1] - 0x30 ;
	}
	return conver_length/2;
}


uint16_t DecString_2_HexArray(char* input, char* output, uint16_t conver_length)
{
	int i;
	
	for(i=0; i<conver_length; i++)
		output[i] = input[i] - '0';
	
	return conver_length;
}

#ifdef USE_ENDIAN_CONVERT

// ——————————————  host to net —————————————— //

// 本地字节序转网络字节序 float 版本
float host2NetFloat(float value)
{
	floatConv f1, f2;
	int size = sizeof(float);
	int i = 0;
	if (4 == size)
	{
		f1.f = value;
		f2.c[0] = f1.c[3];
		f2.c[1] = f1.c[2];
		f2.c[2] = f1.c[1];
		f2.c[3] = f1.c[0];
	}
	else {
		f1.f = value;
		for (; i < size; i++) {
			f2.c[size - 1 - i] = f1.c[i];
		}
	}
	return f2.f;
}


// 本地字节序转网络字节序 double 版本
double host2NetDouble(double value)
{
	doubleConv d1, d2;
	int size = sizeof(double);
	int i = 0;
	d1.d = value;
	for (; i < size; i++) {
		d2.c[size - 1 - i] = d1.c[i];
	}
	return d2.d;
}

// 本地字节序转网络字节序 int16 版本
__inline int16_t host2NetInt16(int16_t value)
{
	return (value & 0x00ff) << 8 | value >> 8 ;
}


// 本地字节序转网络字节序 int32 版本
__inline int32_t host2NetInt32(int32_t value)
{
	return  (value & 0x000000ff) << 24 | (value & 0x0000ff00) << 8 |
		(value & 0x00ff0000) >> 8 | (value & 0xff000000) >> 24;
}


// 本地字节序转网络字节序 int64 版本
__inline int64_t host2NetInt64(int64_t value)
{
	return 		(value & 0x00000000000000FF) << 56 | (value & 0x000000000000FF00) << 40 |
			(value & 0x0000000000FF0000) << 24 | (value & 0x00000000FF000000) << 8 |
			(value & 0x000000FF00000000) >> 8 | (value & 0x0000FF0000000000) >> 24 |
			(value & 0x00FF000000000000) >> 40 | (value & 0xFF00000000000000) >> 56;
}

// ——————————————  net to host —————————————— //

// 网络字节序转本地字节序 float 版本
float net2HostFloat(float value)
{
	floatConv f1, f2;
	int size = sizeof(float);
	int i = 0;
	if (4 == size)
	{
		f1.f = value;
		f2.c[0] = f1.c[3];
		f2.c[1] = f1.c[2];
		f2.c[2] = f1.c[1];
		f2.c[3] = f1.c[0];
	}
	else {
		f1.f = value;
		for (; i < size; i++) {
			f2.c[size - 1 - i] = f1.c[i];
		}
	}
	return f2.f;
}


// 网络字节序转本地字节序 double 版本
double net2HostDouble(double value)
{
	doubleConv d1, d2;
	int size = sizeof(double);
	int i = 0;
	d1.d = value;
	for (; i < size; i++) {
		d2.c[size - 1 - i] = d1.c[i];
	}
	return d2.d;
}


// 网络字节序转本地字节序 int16 版本
__inline int16_t net2HostInt16(int16_t value)
{
	return ((uint16_t)((((value) & 0xff) << 8) | (((value) >> 8) & 0xff)));
}


// 网络字节序转本地字节序 int32 版本
__inline int32_t net2HostInt32(int32_t value)
{
	return ((uint32_t)((((value) & 0xff) << 24) | (((value) & 0xff00) << 8) | (((value) >> 8) & 0xff00) | (((value) >> 24) & 0xff)));
}


// 网络字节序转本地字节序 int64 版本
__inline int64_t net2HostInt64(int64_t value)
{
	return (value & 0x00000000000000FF) << 56 | (value & 0x000000000000FF00) << 40 |
		(value & 0x0000000000FF0000) << 24 | (value & 0x00000000FF000000) << 8 |
		(value & 0x000000FF00000000) >> 8 | (value & 0x0000FF0000000000) >> 24 |
		(value & 0x00FF000000000000) >> 40 | (value & 0xFF00000000000000) >> 56;
}

#endif
