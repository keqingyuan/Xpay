//
//  utils.h
//  
//
//  Created by axis on 15/11/27.
//
//

#ifndef ____utils__
#define ____utils__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// typedef
typedef unsigned char byte;

/**
 * 分析BER-TLV字符串，dsize是字符串数组的个数
 * 返回一个字符串数组
 */
typedef struct BERTLV{
	char t[5];
	char l[3];
	char v[513];
	short vlength;
}BerTlv;

// 16进制字符串转换成字节数组
void hexString2Bytes(const char* src, const int srclen, byte* dst);

// 字节数组转换成16进制字符串
void bytes2HexString(const byte* src, const int srclen, char* dst);

// 16进制字符串转换成字节数组
char *bytes2hexString(unsigned char *data, int size);
// 字节数组转换成16进制字符串
unsigned char *hexString2bytes(const char *data, int size, int *outlen);

/**
 * 解析BerTlv
 * 输入参数：bertlv－要解析的BerTlv字符串，length－要解析的字符串的长度
 * 输出参数：dest-解析后的BerTlv对象数组，dsize-对象的个数
 */
void splitBerTlv(const char* bertlv, const int length, BerTlv *dst, int *dstlen);

// 分析ber-tlv的TAG并返回Tag的长度
int getTagLength(const char* tag);

// 16进制字符串转换成long
long hexString2Long(const char* hex);


#ifdef __cplusplus
}
#endif
#endif /* defined(____utils__) */
