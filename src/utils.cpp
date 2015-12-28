/*
 * utils.cpp
 *
 *  Created on: Nov 28, 2015
 *      Author: axis
 */

#include "utils.h"
#include "DES.h"

//十六进制字符串转换为字节流
void hexString2Bytes(const char* source, const int sourceLen, byte* dest) {
	short i;
	unsigned char highByte, lowByte;

	for (i = 0; i < sourceLen; i += 2) {
		highByte = toupper(source[i]);
		lowByte = toupper(source[i + 1]);

		if (highByte > 0x39)
			highByte -= 0x37;
		else
			highByte -= 0x30;

		if (lowByte > 0x39)
			lowByte -= 0x37;
		else
			lowByte -= 0x30;

		dest[i / 2] = (highByte << 4) | lowByte;
	}
	return;
}
//字节流转换为十六进制字符串
void bytes2HexString(const byte* source, const int sourceLen, char* dest) {
	short i;
	unsigned char highByte, lowByte;

	for (i = 0; i < sourceLen; i++) {
		highByte = source[i] >> 4;
		lowByte = source[i] & 0x0f;

		highByte += 0x30;

		if (highByte > 0x39)
			dest[i * 2] = highByte + 0x07;
		else
			dest[i * 2] = highByte;

		lowByte += 0x30;
		if (lowByte > 0x39)
			dest[i * 2 + 1] = lowByte + 0x07;
		else
			dest[i * 2 + 1] = lowByte;
	}
	return;
}

void splitBerTlv(const char* bertlv, const int length, BerTlv *dest, int *dsize) {
	int position = 0;
	int i = 0;
	for (; position < length;i++) {
		// tag
		char tagbuf[5] = { 0 };
		memcpy(tagbuf, bertlv + position, 2);
		if (1 == getTagLength(tagbuf))
			position += 2;
		else if (2 == getTagLength(tagbuf)) {
			memcpy(tagbuf, bertlv + position, 4);
			position += 4;
		} else
			return;
		// length
		char lengthbuf[3] = { 0 };
		memcpy(lengthbuf, bertlv + position, 2);
		position += 2;
		short valuelength = (int) hexString2Long(lengthbuf) * 2;
		// value
		char valuebuf[513] = { 0 };
		memcpy(valuebuf, bertlv + position, valuelength);
		position += valuelength;

		memset(dest[i].t, '\0', sizeof(dest[i].t));
		memset(dest[i].l, '\0', sizeof(dest[i].l));
		memset(dest[i].v, '\0', sizeof(dest[i].v));
		dest[i].vlength = 0;

		memcpy(dest[i].t,tagbuf,strlen(tagbuf));
		memcpy(dest[i].l,lengthbuf,strlen(lengthbuf));
		memcpy(dest[i].v, valuebuf, strlen(valuebuf));
		dest[i].vlength = valuelength;
	}
	*dsize = i;
	return;
}

int getTagLength(const char* tag) {
	//判断是否有后续tag字节

	char buf1[2];
	char buf2[2];

	int buf;

	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));

	memcpy(buf1, tag, 1);
	memcpy(buf2, tag + 1, 1);

	buf = (int) hexString2Long(buf1);
	//第一位不能被2整除，第二位为F，则符合TLV规定的“11111”实际标签值表示在后续字节中
	if (buf % 2 != 0 && memcmp(buf2, "F", 1) == 0) { //此tag为两字节tag
		return 2;
	} else { //此tag为一字节tag
		return 1;
	}
	return 0;
}

long hexString2Long(const char* hex) {
	int nFlag = 0;
	long lVal;
	int nLen;
	int i;

	nLen = (int) strlen(hex);
	if (nLen > 8) {
		return -1;
	}

	for (i = 0; i < nLen; i++) {
		if (!(isxdigit(hex[i]))) {
			nFlag = 1;
		}
	}

	if (nFlag) {
		return -2;
	}

	lVal = strtol(hex, NULL, 16);

	return lVal;
}
// 异或运算
// p1 p2 是进行异或运算的两个参数
// length 是进行异或运算的数据长度
int calXOR(char *p1, char *p2, const int length, char *dest) {
	int i = 0;

	if (length <= 0) {
		return -1;
	}

	for (i = 0; i < length; i++) {
		dest[i] = p1[i] ^ p2[i];
	}

	return EXIT_SUCCESS;
}
// ASCII码转成BCD码
int ascii2Bcd(const char *ascstr, char *dest, int length) {
	unsigned char hi, lo;
	int i, n;
	int nLen;

	// 获取ASCII长度
	nLen = (int) strlen(ascstr);

	// 检查要转换的ASCII字符串是否合法，合法的ASCII字符为：‘0’~‘9’、	'A'~'F'、	'a'~'f'
	for (i = 0; i < nLen; i++) {
		// 检查传入的字符是否在合法的BCD码字符范围内
		if (!((ascstr[i] >= '0' && ascstr[i] <= '9')
				|| (ascstr[i] >= 'A' && ascstr[i] <= 'F')
				|| (ascstr[i] >= 'a' && ascstr[i] <= 'f'))) {
			return -1;
		}
	}

	// 检查要转换的ASCII字符串长度和要生成的BCD码长度是否合法
	if (nLen < length * 2) {
		return -2;
	}

	for (i = n = 0; n < length;) {
		hi = toupper(ascstr[i++]);
		lo = toupper(ascstr[i++]);
		dest[n++] = (((hi >= 'A') ? (hi - 'A' + 10) : (hi - '0')) << 4)
				| ((lo >= 'A') ? (lo - 'A' + 10) : (lo - '0'));
	}

	return EXIT_SUCCESS;
}

int bcd2Ascii(const char *hexstr, char *dest, int length) {
	int h, a;
	unsigned char uc;

	dest[0] = 0x0;
	if (length < 1)
		return 0;

	h = length - 1;
	a = length + length - 1;
	dest[a + 1] = '\0';
	while (h >= 0) {
		uc = hexstr[h] & 0x0f;
		dest[a--] = uc + ((uc > 9) ? ('A' - 10) : '0');
		uc = (hexstr[h--] & 0xf0) >> 4;
		dest[a--] = uc + ((uc > 9) ? ('A' - 10) : '0');
	}

	dest[length * 2] = '\0';

	return length * 2;
}

/************************************************************************
 * 二进制字节数组转换十六进制字符串函数
 * 输入：
 *       data 二进制字节数组
 *       size 二进制字节数组长度
 * 输出：
 *       十六进制字符串，需要free函数释放空间，失败返回NULL
 *
 * author: tonglulin@gmail.com by www.qmailer.net
 ************************************************************************/
char *bytes2hexString(unsigned char *data, int size)
{
    int  i = 0;
    int  v = 0;
    char *p = NULL;
    char *buf = NULL;
    char base_char = 'A';

    buf = p = (char *)malloc(size * 2 + 1);
    for (i = 0; i < size; i++) {
        v = data[i] >> 4;
        *p++ = v < 10 ? v + '0' : v - 10 + base_char;

        v = data[i] & 0x0f;
        *p++ = v < 10 ? v + '0' : v - 10 + base_char;
    }

    *p = '\0';
    return buf;
}

/************************************************************************
 * 十六进制字符串转换二进制字节数组
 * 输入：
 *       data 十六进制字符串
 *       size 十六进制字符串长度，2的倍数
 *       outlen 转换后的二进制字符数组长度
 * 输出：
 *       二进制字符数组，需要free函数释放空间，失败返回NULL
 *
 * author: qingyuan.ke@foxmail.com
 ************************************************************************/
unsigned char *hexString2bytes(const char *data, int size, int *outlen)
{
    int i = 0;
    int len = 0;
    char char1 = '\0';
    char char2 = '\0';
    unsigned char value = 0;
    unsigned char *out = NULL;

    if (size % 2 != 0) {
        return NULL;
    }

    len = size / 2;
    out = (unsigned char *)malloc(len * sizeof(char) + 1);
    if (out == NULL) {
        return NULL;
    }

    while (i < len) {
        char1 = *data;
        if (char1 >= '0' && char1 <= '9') {
            value = (char1 - '0') << 4;
        }
        else if (char1 >= 'a' && char1 <= 'f') {
            value = (char1 - 'a' + 10) << 4;
        }
        else if (char1 >= 'A' && char1 <= 'F') {
            value = (char1 - 'A' + 10) << 4;
        }
        else {
            free(out);
            return NULL;
        }
        data++;

        char2 = *data;
        if (char2 >= '0' && char2 <= '9') {
            value |= char2 - '0';
        }
        else if (char2 >= 'a' && char2 <= 'f') {
            value |= char2 - 'a' + 10;
        }
        else if (char2 >= 'A' && char2 <= 'F') {
            value |= char2 - 'A' + 10;
        }
        else {
            free(out);
            return NULL;
        }

        data++;
        *(out + i++) = value;
    }
    *(out + i) = '\0';

    if (outlen != NULL) {
        *outlen = i;
    }

    return out;
}
