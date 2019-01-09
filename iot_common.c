#include "iot_common.h"


#ifdef USE_QUEUE

static MsgQueue Upload_Queue;
static MsgQueue Download_Queue;
MsgQueue* which_queue(uint8_t queue_name);

#endif



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

#ifdef USE_QUEUE

MsgQueue* which_queue(uint8_t queue_name)
{
	switch(queue_name) {
		case UploadQueue: return &Upload_Queue;
		case DownloadQueue: return &Download_Queue;
		default : LOGE("queue_name error");
			return NULL;
	}
}

uint8_t Push_Queue(uint8_t queue_name, char *buf, uint16_t buf_length)
{
	uint8_t i, temp=0, *buffer;
	size_t *pos1 = (size_t *)buf, *pos2 = (size_t *)buffer;
	Message_pack *q;
	MsgQueue* queue;
	
	queue = which_queue(queue_name);
	if(queue == NULL) return 2;
	if( queue->queue_n+1 >= MES_Queue_MAX ) {
		LOGE("PUT_NB_Queue error : queue size not enough.\n");
		return 1;
	}
	
	for(q=queue->member; q->next != NULL; q=q->next);
	q->next = malloc(sizeof(Message_pack));
	q = q->next;
	
	q->message_length =  buf_length;
	q->message = malloc(buf_length);
	q->next = NULL;
	
	buffer = q->message;
	
	if(buf_length < 64)
		for(i=0; i<buf_length; i++)
			buffer[i] = buf[i];
	else
	{		// 若数据较长，利用系统长度的指针加速的数据拷贝
		temp = buf_length % sizeof(size_t);
		for(i=0; i<buf_length/sizeof(size_t); i++)
			pos2[i] = pos1[i];
	}
	if( temp >0 )
		for(i=buf_length-temp; i<buf_length; i++)
			buffer[i] = buf[i];
	
	queue->queue_n++;
	
	return 0;
}


uint8_t Pop_Queue(uint8_t queue_name)
{
	Message_pack *q;
	MsgQueue* queue;
	
	queue = which_queue(queue_name);
	if(queue == NULL) return 2;
	if(queue->queue_n <= 0) {
		LOGE("Queue is empty so can't pop!\n");
		return 1;
	}
	queue->queue_n;
	
	q = queue->member;
	free(q->message);
	queue->member = q->next;
	free(q);
	
	return 0;
}

Message_pack* Read_Queue(uint8_t queue_name)
{
	MsgQueue* queue;
	
	queue = which_queue(queue_name);
	if(queue == NULL) return NULL;
	return queue->member;
}

uint8_t IsEmpty_Queue(uint8_t queue_name)
{
	MsgQueue* queue;
	
	queue = which_queue(queue_name);
	if(queue == NULL) return 0xFF;
	if( queue->queue_n <= 0 ) {
		return 1;
	}
	else return 0;
}

uint8_t IsFull_Queue(uint8_t queue_name)
{
	MsgQueue* queue;
	
	queue = which_queue(queue_name);
	if(queue == NULL) return 0xFF;
	if( queue->queue_n >= MES_Queue_MAX ) {
		return 1;
	}
	else return 0;
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

#endif
