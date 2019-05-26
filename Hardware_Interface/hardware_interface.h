/* *Copyright(C), Haijie Lee. 2019. AllRightsReserved.
	*FileName:  硬件功能接口 模块头文件.
	*Author:  Haijie Lee.
	*Version:  1.5.
	*Date:  2019.01.22.
	*Description:  用户可通过调用此文件中的接口，调用对应的硬件功能。
	*Others:  用户不应随意修改此文件，除非你知道自己在做什么。
*/
#ifndef    IOT_API__HARDWARE_INERFACE__HARDWARE_INTERFACE_H
#define   IOT_API__HARDWARE_INERFACE__HARDWARE_INTERFACE_H

#include <stdint.h>


/* 功能：（写）串口驱动的入口函数
	说明：在此函数中写入当前单片机的（连接到NB模组的）写数据的串口驱动。
	参数：@*input_buff: 将要写入的数据缓冲区的首地址；
				@data_length: 将要写入的数据的总长度。
	返回值：成功写入到串口的所有数据的总长度。
*/
uint16_t Write_SerialPort( uint8_t *input_buff, uint16_t data_length );

/* 功能：（读）串口驱动的入口函数
	说明：在此函数中写入当前单片机的（连接到NB模组的）读数据的串口驱动。
	参数：@**output_buff: 读出接收数据的缓冲区的首地址；
				@data_length: 读出数据缓冲区的最大长度。
	返回值：读取到的所有数据的总长度。
*/
uint16_t Read_SerialPort( uint8_t **output_buff, uint16_t *buff_size );

/* 功能：定时器的入口函数。
	说明：在此函数中写入当前单片机的（连接到NB模组的）定时器功能，用于提供延迟。
	参数：@time: 需要延迟的毫秒数。
	返回值：无。
*/
void TimerDelay_ms( uint32_t time );



#endif
