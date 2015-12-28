/******************************************************************************************
 ** 文件名称：publicFunc.cpp                                                             **
 ** 文件描述：公共函数                                                                   **
 **--------------------------------------------------------------------------------------**
 ** 创 建 人：智能卡业务部                                                               **
 ** 创建日期：2011-7-27                                                                  **
 **--------------------------------------------------------------------------------------**
 ** 修 改 人：智能卡业务部                                                               **
 ** 修改日期：2013-05-15                                                                 **
 ** 修改说明：增加PBOC2.0相关函数														 **
 **--------------------------------------------------------------------------------------**
 ** 修 改 人：智能卡业务部                                                               **
 ** 修改日期：2014-04-20                                                                 **
 ** 修改说明：增加规范类相关函数														 **
 **--------------------------------------------------------------------------------------**
 ** 版 本 号： V1.0 -- 基础公共函数                                                      **
 **            V2.0 -- PBOC2.0函数														 **
 **            V3.0 -- 规范类函数														 **
 **--------------------------------------------------------------------------------------**
 **                     Copyright (c) 2011  ftsafe                                       **
 ******************************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
//#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include "DES.h"
#include "publicFunc.h"

using namespace std;

//全局变量定义
/* 定义月最大天数*/
int gMaxDayMonth[] = { -1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
char *gWeekDay[] = { "Sun", "Mon", "Tue", "Wed", "Thur", "Fri", "Sat" };
char *gChNum[] = { "零", "壹", "贰", "叁", "肆", "伍", "陆", "柒", "捌", "玖" };
char *gNum[] = { "零", "一", "二", "三", "四", "五", "六", "七", "八", "九" };
static char* szDataEncKey = "6131673464386B37693066366D327565";

// 定义函数调用成功返回值
#define FTOK 0

// ASCII码的TLV缓冲区最大长度
#define FTTLVMAXLEN 1024

//----------------------------------------------------------------------------------------
//第一部分: 基础公共函数
//----------------------------------------------------------------------------------------

/******************************************************************************************
 *******	函数名称：ftAtoh		-序号：1.1-										*******
 *******	函数功能：ASCII 码转换成 BCD码 即16进制码								*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：																*******
 *******			 char *ascstr:需转换的ASCII字符串								*******
 *******			 char *bcdstr:转换生成的BCD码									*******
 *******			 int  bcdlen:生成的BCD码长度									*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：FTOK(0) ,失败返回错误代码，错误代码如下：			*******
 *******                            -1：传入的ASCII码字符串包含非法字符				*******
 *******                            -2：ASCII码无足够字符生成指定长度的BCD码		*******
 ******************************************************************************************/
int ftAtoh(char *ascstr, char *bcdstr, int bcdlen) {
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
	if (nLen < bcdlen * 2) {
		return -2;
	}

	for (i = n = 0; n < bcdlen;) {
		hi = toupper(ascstr[i++]);
		lo = toupper(ascstr[i++]);
		bcdstr[n++] = (((hi >= 'A') ? (hi - 'A' + 10) : (hi - '0')) << 4)
				| ((lo >= 'A') ? (lo - 'A' + 10) : (lo - '0'));
	}

	return NO_ERROR;
}

/******************************************************************************************
 *******	函数名称：ftHtoa		-序号：1.2-										*******
 *******	函数功能：BCD码(即16进制)转换成ASCII									*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：																*******
 *******			char *hexstr:需转换的BCD码										*******
 *******			char *ascstr:生成的ASCII码										*******
 *******			int  length:需要转换的BCD码的长度								*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功：返回生成的ASCII长度；失败返回：0						*******
 ******************************************************************************************/
int ftHtoa(char *hexstr, char *ascstr, int length) {
	int h, a;
	unsigned char uc;

	ascstr[0] = 0x0;
	if (length < 1)
		return 0;

	h = length - 1;
	a = length + length - 1;
	ascstr[a + 1] = '\0';
	while (h >= 0) {
		uc = hexstr[h] & 0x0f;
		ascstr[a--] = uc + ((uc > 9) ? ('A' - 10) : '0');
		uc = (hexstr[h--] & 0xf0) >> 4;
		ascstr[a--] = uc + ((uc > 9) ? ('A' - 10) : '0');
	}

	ascstr[length * 2] = '\0';
	return length * 2;
}

/******************************************************************************************
 *******	函数名称：ftHexToLong	   -序号：1.3-									*******
 *******	函数功能：将16进制的金额转换成长整型									*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：																*******
 *******			char *hexstr:十六进制金额值字符串(如："65F")					*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：长整型的金额值	,失败返回错误代码，错误代码如下：	*******
 *******                        -1：传入16进制金额超范围							*******
 *******                        -2：传入参数非法，字符必须为：0~9，A~F，a~f			*******
 ******************************************************************************************/
long ftHexToLong(char *HexStr) {
	int nFlag = 0;
	long lVal;
	int nLen;
	int i;

	nLen = (int) strlen(HexStr);
	if (nLen > 8) {
		return -1;
	}

	for (i = 0; i < nLen; i++) {
		if (!(isxdigit(HexStr[i]))) {
			nFlag = 1;
		}
	}

	if (nFlag) {
		return -2;
	}

	lVal = strtol(HexStr, NULL, 16);
	return lVal;
}

/******************************************************************************************
 *******	函数名称：ftLongToHex	   -序号：1.4-									*******
 *******	函数功能：十进制金额值转换成16进制金额字符串							*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：																*******
 *******			long Val：要转换的十进制金额值									*******
 *******			char *hexstr:(输出)转换生成的16进制金额值字符串					*******
 *******			int Flag：金额值输出类型标志；缺省值为1：BCD码；0：16进制字符串	*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功：失败返回：-1；成功返回：生成的16进制金额值字符串长度	*******
 ******************************************************************************************/
int ftLongToHex(long Val, char *HexStr, int Flag) {
	int nLen;
	long MaxVal = 2147483647;
	long aHexVal[10];
	long lVal;
	long lInVal;
	char szHexStr[10 + 1];
	char szAscStr[10 + 1];
	int i = 0;

	memset(szHexStr, 0x00, sizeof(szHexStr));
	memset(szAscStr, 0x00, sizeof(szAscStr));

	if ((Val > MaxVal) || Val < 0) {
		return -1;
	}

	for (i = 0; i < 10; i++) {
		aHexVal[i] = 0L;
	}

	i = 0;
	lInVal = Val;
	while (1) {
		lVal = lInVal % 16;
		aHexVal[i] = lVal;
		i++;
		lInVal = lInVal / 16;
		if (lInVal == 0) {
			break;
		}
	}

	nLen = i;			// ASCII码数据长度

	for (i = 0; i < nLen; i++) {
		if (aHexVal[i] >= 0 && aHexVal[i] <= 9) {
			szAscStr[i] = (int) aHexVal[i] + 48;
		} else {
			szAscStr[i] = ((int) aHexVal[i] - 10) + 'A';
		}
	}

	memset(szHexStr, 0x00, sizeof(szHexStr));

	for (i = 0; i < nLen; i++) {
		szHexStr[nLen - 1 - i] = szAscStr[i];
	}

	memset(szAscStr, 0x00, sizeof(szAscStr));

	i = nLen % 2;

	if (i > 0) {
		memcpy(szAscStr, "0", 1);
		memcpy(szAscStr + 1, szHexStr, nLen);
		nLen += 1;
	} else {
		memcpy(szAscStr, szHexStr, nLen);
	}

	// Flag 参数为：0 返回的金额为16进制的ASCII字符串金额
	if (Flag == 0) {
		memcpy(HexStr, szAscStr, nLen);
		memcpy(HexStr + nLen, "\x00", 1);
		return nLen;
	}

	// Flag 参数为：1，返回的金额为BCD码的金额
	nLen = nLen / 2;
	memset(szHexStr, 0x00, sizeof(szHexStr));

	ftAtoh(szAscStr, szHexStr, nLen);
	memcpy(HexStr, szHexStr, nLen);

	return nLen;
}

/******************************************************************************************
 *******	函数名称：ftFillAmount	   -序号：1.5-									*******
 *******	函数功能：将BCD码的金额字符串填充到4字节长度(前补0x00)					*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：																*******
 *******			char *Str：(输入/输出)要进行填充的16进制金额字符串				*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功：失败返回：-1；成功返回：返回：0或8						*******
 ******************************************************************************************/
int ftFillAmount(char *Str) {
	char sFill[8 + 1];
	char sAmount[8 + 1];
	int nLen;
	int nFillLen;

	memset(sFill, 0x00, sizeof(sFill));
	memset(sAmount, 0x00, sizeof(sAmount));

	memcpy(sFill, "00000000", 8);

	nLen = (int) strlen(Str);

	if (nLen > 8) {
		return -1;
	}

	// 检查金额的合法性
	// 检查要转换的ASCII字符串是否合法，合法的ASCII字符为：‘0’~‘9’、	'A'~'F'、	'a'~'f'
	for (int i = 0; i < nLen; i++) {
		// 检查传入的字符是否在合法的BCD码字符范围内
		if (!((Str[i] >= '0' && Str[i] <= '9')
				|| (Str[i] >= 'A' && Str[i] <= 'F')
				|| (Str[i] >= 'a' && Str[i] <= 'f'))) {
			return -2;
		}
	}

	if (nLen == 8) {
		return FTOK;
	}

	// 计算要填充的字符数
	nFillLen = 8 - nLen;
	memcpy(sAmount, sFill, nFillLen);
	memcpy(sAmount + nFillLen, Str, nLen);

	memcpy(Str, sAmount, 8);
	memcpy(Str + 8, "\x00", 1);

	return NO_ERROR;
}

/******************************************************************************************
 *******	函数名称：ftFillString	   -序号：1.6-									*******
 *******	函数功能：MAC/TAC计算的字符串填充函数(填充80或8000..到8的倍数长度)		*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：																*******
 *******			char *Str：(输入/输出)要进行填充的16进制字符串					*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功：失败返回：-1；成功返回：填充后的ASCII数据长度			*******
 ******************************************************************************************/
int ftFillString(char *Str) {
	char sFill[8 * 2 + 1];
	int nLen;
	int nFillLen;

	nLen = (int) strlen(Str);
	if (nLen <= 0) {
		return -1;
	}

	nFillLen = nLen % 16;
	nFillLen = 16 - nFillLen;

	// 填充字符串
	memset(sFill, 0x00, sizeof(sFill));
	memcpy(sFill, "8000000000000000", 16);

	memcpy(Str + nLen, sFill, nFillLen);

	memcpy(Str + nLen + nFillLen, "\x00", 1);

	return nLen + nFillLen;
}

/******************************************************************************************
 *******	函数名称：ftFillString	   -序号：1.7-									*******
 *******	函数功能：将字符串填充指定字符到指定长度,大于此长度则截取到此长度		*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：																*******
 *******			char *Str：(输入/输出)要进行填充的字符串						*******
 *******            int   Len：指定填充后的长度										*******
 *******            char  Val：指定填充的字符										*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：FTOK(0);失败返回小于零；-1：传入长度参数非法		*******
 ******************************************************************************************/
int ftFillStringF(char *Str, int Len, char Val) {
	int nLen;
	int nFillLen;
	int i;

	nLen = (int) strlen(Str);

	// 字符串长度大于长度参数，截取到指定长度
	if (nLen > Len) {
		Str[Len] = 0x00;
		return NO_ERROR;
	}

	// 字符串长度等于长度参数，直接返回
	if (nLen == Len) {
		return NO_ERROR;
	}

	// 计算要填充的字节数
	nFillLen = Len - nLen;

	for (i = 0; i < nFillLen; i++) {
		Str[nLen + i] = Val;
	}

	Str[Len] = 0x00;

	return NO_ERROR;
}

/******************************************************************************************
 *******	函数名称：ftStringToUpper	   -序号：1.8-								*******
 *******	函数功能：将字符串中的字母转换成大写									*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *Str:(输入/输出)需要进行转换的字符						*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：无															*******
 ******************************************************************************************/
void ftStringToUpper(char *Str) {
	int nLen;

	nLen = (int) strlen(Str);

	for (int i = 0; i < nLen; i++) {
		if (Str[i] >= 'a' && Str[i] <= 'z') {
			Str[i] -= 32;
		}
	}
}

/******************************************************************************************
 *******	函数名称：ftStringToLower	   -序号：1.9-								*******
 *******	函数功能：将字符串中的字母转换成小写									*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *Str:(输入/输出)需要进行转换的字符						*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功：返回生成的ASCII长度；失败返回：0						*******
 ******************************************************************************************/
void ftStringToLower(char *Str) {
	int nLen;

	nLen = (int) strlen(Str);

	for (int i = 0; i < nLen; i++) {
		if (Str[i] >= 'A' && Str[i] <= 'Z') {
			Str[i] += 32;
		}
	}

}

/******************************************************************************************
 *******	函数名称：ftCharToBitString	   -序号：1.10-								*******
 *******	函数功能：将字符转换成BIT字符串1字节转换成8个0、1的字符函数				*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：unsigned char DataBuf：要进行转换的字符						*******
 *******              int   Len:数据长度											*******
 *******			  char *OutBuf：转换生成的BIT字符数组							*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回：-1		          						*******
 ******************************************************************************************/
int ftCharToBitString(unsigned char Val, char *OutBuf) {

	char szBitString[8 + 1];
	unsigned int nVal;

	memset(szBitString, 0x00, sizeof(szBitString));
	memcpy(szBitString, "00000000", 8);

	for (int i = 0; i < 8; i++) {
		nVal = (unsigned int) pow((float) 2, (int) 7 - i);
		if (Val & nVal) {
			szBitString[i] = '1';
		}
	}

	memcpy(OutBuf, szBitString, 8);
	memcpy(OutBuf + 8, "\x00", 1);

	return NO_ERROR;

}

/******************************************************************************************
 *******	函数名称：ftDataToBitString	   -序号：1.11-								*******
 *******	函数功能：将BCD码数据转换成BIT字符串1字节转换成8个0、1的字符函数		*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *InData：要进行转换的数据指针							*******
 *******              int   Len:数据长度											*******
 *******			  char *OutBuf：转换生成的BIT字符数组							*******
 *******            例如：01—>0011 0000 0011 0001                                   *******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回：-1		          						*******
 ******************************************************************************************/
int ftDataToBitString(unsigned char *InData, int Len, char *OutBuf) {
	char szCharStr[8 + 1];

	int nOutLen;

	// 检查传入的Len的合法性
	if (Len <= 0) {
		return -1;
	}

	nOutLen = 8 * Len + 1;

	//  分配输出指针
	for (int i = 0; i < Len; i++) {
		memset(szCharStr, 0x00, sizeof(szCharStr));

		ftCharToBitString(InData[i], szCharStr);

		memcpy(OutBuf + i * 8, szCharStr, 8);
	}

	OutBuf[nOutLen - 1] = 0x00;
	return NO_ERROR;
}
/******************************************************************************************
 *******	函数名称：ftDataToBitString	   -序号：1.11.1-								*******
 *******	函数功能：将BCD码数据转换成BIT字符串1字节转换成8个0、1的字符函数		            *******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *OutBuf：转换生成的BIT字符数组							        *******
 *******            char *InData：要进行转换的数据指针							        *******
 *******			int   Len:数据长度                                               *******
 *******            例如：01—>0000 0001                                              *******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回：-1		          						    *******
 ******************************************************************************************/
int ftDataToBitStringNoASCII(char *OutBuf, unsigned char *InData, int Len) {
	char szCharStr[8 + 1];

	int nOutLen;

	// 检查传入的Len的合法性
	if (Len <= 0) {
		return -1;
	}

	nOutLen = 8 * Len + 1;

	//  分配输出指针
	for (int i = 0; i < Len; i++) {
		memset(szCharStr, 0x00, sizeof(szCharStr));

		ftCharToBitString(InData[i], szCharStr);

		memcpy(OutBuf + i * 4, szCharStr + 4, 4);
	}

	OutBuf[nOutLen - 1] = 0x00;
	return NO_ERROR;
}

/******************************************************************************************
 *******	函数名称：ftBitStringToLong	   -序号：1.12-								*******
 *******	函数功能：将BIT数组转换成整型值											*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *BitString：要进行转换的BIT指针							*******
 *******              int   Len:BIT指针长度,最长度2字节(即16个bit)					*******
 *******			  long *Val：(输出)	Bit转换生成的INT值							*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：FTOK(0)；失败返回错误代码,错误代码定义如下：		*******
 *******                      -1：传入的BIT数组及长度参数错误或不匹配				*******
 *******                      -2：传入的BIT数组字符值非法(必须是0/1字符)			*******
 ******************************************************************************************/
int ftBitStringToLong(char *BitString, int Len, long *Val) {
	int i;
	int nLen;
	long nVal = 0;
	long nReturn = 0;

	// 获取BIT数组长度
	nLen = (int) strlen(BitString);

	// 检查传入长度合法性
	if (Len <= 0 || Len > nLen || Len > 16) {
		return -1;
	}

	// 检查传入的BIT 数组的合法性，BIT数组字符必须为：0,1 代码
	for (i = 0; i < Len; i++) {
		if (!(BitString[i] == '0' || BitString[i] == '1')) {
			return -2;
		}
	}

	for (i = 0; i < Len; i++) {
		if (BitString[i] == '1') {
			nVal = (long) pow((double) 2, (int) Len - 1 - i);
			if (nVal > 0) {
				nReturn += nVal;
			}
		}
	}

	*Val = nReturn;
	return NO_ERROR;
}

/******************************************************************************************
 *******	函数名称：ftBitStringToData	   -序号：1.13-								*******
 *******	函数功能：将BIT数组转换成16进制数据										*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *BitString：要进行转换的BIT指针							*******
 *******              int   Len:BIT指针长度,必须为8的倍数							*******
 *******			  int  *Val：(输出)	Bit转换生成的INT值							*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回错误信息，错误信息如下：					*******
 *******                     -1：BIT数组长度 和传入的长度参数不匹配					*******
 *******                     -2：BIT数组必须为8的倍数								*******
 *******                     -3：调用ftBitStringToLong函数失败						*******
 ******************************************************************************************/
int ftBitStringToData(char *BitString, int Len, unsigned char *OutData) {
	int nRet;
	int nCharNum;
	char szBit[9];
	long lVal;

	// 检查传入的BIT数组是否和传入字符长度是否一致；
	if (strlen(BitString) != Len) {
		return -1;
	}

	// 检查长度是否为8的倍数
	nRet = Len % 8;
	if (nRet != 0) {
		return -2;
	}

	// 计算转换生成的数据项数
	nCharNum = Len / 8;

	for (int i = 0; i < nCharNum; i++) {
		memset(szBit, 0x00, sizeof(szBit));

		memcpy(szBit, BitString + i * 8, 8);

		// 将8字节的BIT编码转换成长整型
		nRet = ftBitStringToLong(szBit, 8, &lVal);
		if (nRet != FTOK) {
			return -3;
		}

		// 将长整型值保持输出的16进制编码数组
		OutData[i] = (unsigned char) lVal;
	}

	// 增加输出变量的结束字符
	OutData[nCharNum] = 0x00;

	return NO_ERROR;
}

/******************************************************************************************
 *******	函数名称：ftCalXOR	   -序号：1.14-										*******
 *******	函数功能：异或计算函数(函数参数为Hex编码)								*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：																*******
 *******			 char *Param1：要进行异或计算的第一个参数						*******
 *******			 char *Param2：要进行异或计算的第二个参数						*******
 *******			 int   Len：   要进行异或计算的数据长度							*******
 *******			 char *Out：   (输出)异或计算返回值								*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功：成功返回：0；失败返回：-1								*******
 ******************************************************************************************/
int ftCalXOR(char *Param1, char *Param2, int Len, char *Out) {
	int i = 0;

	if (Len <= 0) {
		return -1;
	}

	for (i = 0; i < Len; i++) {
		Out[i] = Param1[i] ^ Param2[i];
	}

	return NO_ERROR;
}

/******************************************************************************************
 *******	函数名称：ftStrXOR	   -序号：1.14-										*******
 *******	函数功能：异或计算函数(函数参数为字符串)								*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：																*******
 *******			 char *Param1：要进行异或计算的第一个参数						*******
 *******			 char *Param2：要进行异或计算的第二个参数						*******
 *******			 int   Len：   要进行异或计算的数据长度							*******
 *******			 char *Out：   (输出)异或计算返回值								*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功：成功返回：0；失败返回：-1								*******
 ******************************************************************************************/
int ftStrXOR(char *Param1, char *Param2, char *Out) {
	char szBuf[256 + 1];
	char szData1[128 + 1];
	char szData2[128 + 1];
	int nLen1;
	int nLen2;
	int nRet;

	memset(szBuf, 0x00, sizeof(szBuf));
	memset(szData1, 0x00, sizeof(szData1));
	memset(szData2, 0x00, sizeof(szData2));

	// 获取进行异或计算的ASCII码长度
	nLen1 = (int) strlen(Param1);
	nLen2 = (int) strlen(Param2);

	// ASCII码长度不一致返回失败
	if (nLen1 != nLen2) {
		return FTPROTHERERR;
	}

	// BCD码长度
	nLen1 = nLen1 / 2;

	// 支持计算的Hex数据长度为128
	if (nLen1 > 128) {
		return FTPROTHERERR;
	}

	// ASCII码转BCD码
	ftAtoh(Param1, szData1, nLen1);
	ftAtoh(Param2, szData2, nLen1);

	// 进行HEX数据异或计算
	nRet = ftCalXOR(szData1, szData2, nLen1, szBuf);
	if (nRet != NO_ERROR) {
		return nRet;
	}

	memset(szData1, 0x00, sizeof(szData1));
	memcpy(szData1, szBuf, nLen1);
	memset(szBuf, 0x00, sizeof(szBuf));

	// 将计算的HEX结果转换成ASCII码
	ftHtoa(szData1, szBuf, nLen1);

	// ASCII码长度
	nLen1 = nLen1 * 2;
	memcpy(Out, szBuf, nLen1);
	memcpy(Out + nLen1, "\x00", 1);

	return NO_ERROR;

}

/******************************************************************************************
 *******	函数名称：ftCalLRC	   -序号：1.15-										*******
 *******	函数功能：数据异或和计算函数											*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *DataBuf：(输入输出)要进行异或和计算的缓冲区				*******
 *******              int   Len:数据长度											*******
 *******			  int Flag：数据格式标准；1：BCD码；0:ASCII(缺省值)	;			*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回：-1		          						*******
 ******************************************************************************************/
int ftCalLRC1(char *DataBuf, int Len) {
	char *pBuf;
	char cVal;

	int i;
	int nBcdLen = 0;

	nBcdLen = Len;

	cVal = DataBuf[0];

	for (i = 1; i < nBcdLen; i++) {
		cVal = cVal ^ pBuf[i];
	}

	DataBuf[Len] = cVal;

	DataBuf[Len + 1] = 0x00;

	return NO_ERROR;
}

/******************************************************************************************
 *******	函数名称：ftCalLRC	   -序号：1.15-										*******
 *******	函数功能：数据异或和计算函数											*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *DataBuf：(输入输出)要进行异或和计算的缓冲区				*******
 *******              int   Len:数据长度											*******
 *******              char *RetVal：(输出)异或计算结果，在第1字节					*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回：-1		          						*******
 ******************************************************************************************/
int ftCalLRC(char *DataBuf, int Len, char *RetVal) {
	char cVal;
	int i;
	int nBcdLen = 0;

	nBcdLen = Len;

	cVal = DataBuf[0];

	for (i = 0; i < nBcdLen; i++) {
		cVal = cVal ^ DataBuf[i];
	}

	RetVal[0] = cVal;
	RetVal[1] = 0x00;

	return FTOK;
}

/******************************************************************************************
 *******	函数名称：ftDataEnc	   -序号：1.16-										*******
 *******	函数功能：数据加密函数(用于密码的简单加密)								*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pIn:要加密的数据										*******
 *******              int   Len:要加密的数据长度									*******
 *******			  char *pOut:(输出)	加密生成数据								*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回：-1		          						*******
 ******************************************************************************************/
int ftDataEncL(char *pIn, int pLen, char *pOut) {
	int nKeyLen;
	int nFillLen;
	int nLen;
	char szData[600 + 1];
	char szTmp[600 + 1];
	char szFill[16 + 1];

	pOut[0] = 0x00;

	nLen = (int) strlen(pIn);
	if (pLen < 0 || pLen > 99 || nLen != pLen) {
		return -1;
	}

	memset(szData, 0x00, sizeof(szData));
	memset(szFill, 0x00, sizeof(szFill));
	memset(szTmp, 0x00, sizeof(szTmp));

	// 将输入的加密数据转换成ASCII码
	ftHtoa(pIn, szTmp, nLen);

	memcpy(szFill, "0000000000000000", 16);

	//053132333435  000000000000
	sprintf(szData, "%02.2d%s", pLen, szTmp);

	nLen = (int) strlen(szData);

	nFillLen = nLen % 16;
	if (nFillLen > 0) {
		nFillLen = 16 - nFillLen;
		memcpy(szData + nLen, szFill, nFillLen);

		nLen += nFillLen;
	}

	nKeyLen = (int) strlen(szDataEncKey);

	int nRet = ft3DesEncL(szDataEncKey, nKeyLen, szData, pOut, nLen, 1);

	pOut[nLen] = 0x00;

	return nRet;

}

//数据加密函数[传入参数为ASCII字符串]
int ftDataEnc(char *pIn, char *pOut) {
	int nLen;
	int nRet;

	nLen = (int) strlen(pIn);

	nRet = ftDataEncL(pIn, nLen, pOut);

	return nRet;

}

/******************************************************************************************
 *******	函数名称：ftDataDec	   -序号：1.17-										*******
 *******	函数功能：数据解密函数(用于密码的简单解密)								*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pIn:要解密的数据										*******
 *******              int   pLen:要解密的数据长度									*******
 *******			  int  *pOut：(输出)解密生成数据								*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回：-1		          						*******
 ******************************************************************************************/
int ftDataDecL(char *pIn, int pLen, char *pOut) {
	int nKeyLen;
	char szOut[1024 + 1];
	char szTmp[1024 + 1];
	char szLen[3];
	int nLen;

	memset(szOut, 0x00, sizeof(szOut));
	memset(szLen, 0x00, sizeof(szLen));
	memset(szTmp, 0x00, sizeof(szTmp));

	// 检查数据长度是否和传入的长度匹配
	if (strlen(pIn) != pLen) {
		return -1;
	}

	// 检查数据长度是否为：16或32个字符
	if (strlen(pIn) % 16 != 0) {
		return -2;
	}

	// 检查处理数据的最大长度为：1024 个字符
	if (pLen > 1024) {
		return -3;
	}

	nKeyLen = (int) strlen(szDataEncKey);

	// DES解密
	ft3DesDecL(szDataEncKey, nKeyLen, pIn, szOut, pLen, 1);

	// 取2字节长度；
	memcpy(szLen, szOut, 2);

	nLen = (int) atoi(szLen);

	ftAtoh(szOut + 2, szTmp, nLen);

	memcpy(pOut, szTmp, nLen);
	pOut[nLen] = 0x00;

	return NO_ERROR;
}

//数据解密函数
int ftDataDec(char *pIn, char *pOut) {
	int nLen;
	int nRet;

	nLen = (int) strlen(pIn);

	nRet = ftDataDecL(pIn, nLen, pOut);

	return nRet;
}

/******************************************************************************************
 *******	函数名称：ftDesEnc   -序号：1.18-										*******
 *******    函数功能：DES数据加密													*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pKey   :要进行DES加密的密钥,ASCII码16字节，BCD为8字节	*******
 *******			  int   pKeyLen:密钥长度,值为8:BCD码;16:ASCII码					*******
 *******			  char *pIn    :加密数据，ASCII为16字节、BCD码为8字节的倍数 	*******
 *******			  char *pOut   :(输出)加密生成的数据							*******
 *******              int   pLen   :要加密的数据长度必须是8的倍数					*******
 *******              int   pFlag  :数据格式标志，缺省为：0，BCD码；其他值为ASCII   *******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回<0	;错误代码如下：						*******/
/******************************************************************************************/
int ftDesEncL(char *pKey, int pKeyLen, char *pIn, char *pOut, int Len,
		int pFlag) {
	unsigned char szHexKey[8 + 1];		// 十六进制的KEY
	unsigned char *pHexData;
	unsigned char szTmpInData[8 + 1];		// 加密用输入变量
	unsigned char szTmpOutData[8 + 1];	// 加密临时输出变量
	unsigned char *pOutData;			// 临时输出的ASCII数据
	unsigned char *pHexOutData;			// 临时输出的十六进制数据

	int nHexDataLen;					// 十六进制的数据长度
	int nDataItem;					// 数据项个数
	int i;

	memset(szHexKey, 0x00, sizeof(szHexKey));

	// 根据传入的数据类型检查传入数据的合法性
	if (pFlag == 0)						// 传入数据为BCD码
			{
		if ((pKeyLen != 8) || ((Len % 8) != 0) || (Len == 0)) {
			return -1;
		}

		memcpy(szHexKey, pKey, 8);		// 加密密钥
		nHexDataLen = Len;

		// 分配指针
		pHexData = new unsigned char[Len + 1];
		pHexOutData = new unsigned char[Len + 1];
		pOutData = new unsigned char[Len + 1];
		if ((pHexData == NULL) || (pHexOutData == NULL) || (pOutData == NULL)) {
			return -2;
		}

		memset(pHexData, 0x00, Len + 1);
		memset(pHexOutData, 0x00, Len + 1);
		memset(pOutData, 0x00, Len + 1);

		memcpy(pHexData, pIn, Len);
	} else								// 传入数据为ASCII码
	{
		if ((pKeyLen != 16) || ((Len % 16) != 0) || (Len == 0)) {
			return -1;
		}

		// 将ASCII码的密钥转换成BCD码的密钥
		ftAtoh(pKey, (char *) szHexKey, 8);

		// 将ASCII码的加密数据转换成BCD码的数据
		nHexDataLen = Len / 2;

		// 分配指针
		pHexData = new unsigned char[nHexDataLen + 1];
		pHexOutData = new unsigned char[nHexDataLen + 1];
		pOutData = new unsigned char[Len + 1];

		if ((pHexData == NULL) || (pHexOutData == NULL) || (pOutData == NULL)) {
			return -2;
		}

		memset(pHexData, 0x00, nHexDataLen + 1);
		memset(pHexOutData, 0x00, nHexDataLen + 1);
		memset(pOutData, 0x00, Len + 1);

		// 将传入的ASCII码数据转换成
		ftAtoh(pIn, (char *) pHexData, nHexDataLen);

	}

	// 计算用进行计算的次数
	nDataItem = nHexDataLen / 8;

	for (i = 0; i < nDataItem; i++) {
		memset(szTmpInData, 0x00, sizeof(szTmpInData));
		memset(szTmpOutData, 0x00, sizeof(szTmpOutData));

		//memcpy(pHexData + i*8,szTmpInData,8);
		memcpy(szTmpInData, pHexData + i * 8, 8);
		// DES数据加密
		CDES::RunDes(0, (char *) szTmpInData, 8, (char *) szHexKey, 8,
				(char *) szTmpOutData);

		//保存加密的数据
		memcpy(pHexOutData + i * 8, szTmpOutData, 8);
	}

	if (pFlag == 0)						// 传入数据为BCD码
			{
		memcpy(pOut, pHexOutData, nHexDataLen);

	} else {
		// 将加密的16进制数据转换成ASCII
		ftHtoa((char *) pHexOutData, (char *) pOutData, nHexDataLen);
		memcpy(pOut, pOutData, nHexDataLen * 2);

		//Add-2013-08-14 ,增加输出字符串参数的结束字符
		nHexDataLen = nHexDataLen * 2;
		memcpy(pOut + nHexDataLen, "\x00", 1);

	}

	// 释放指针
	free(pHexData);
	free(pHexOutData);
	free(pOutData);

	pHexData = NULL;
	pHexOutData = NULL;
	pOutData = NULL;

	return 0;
}

// ASCII码参数的DES加密函数
int ftDesEnc(char *pKey, char *pIn, char *pOut) {
	int nKeyLen;
	int nInLen;
	int nRet;

	nKeyLen = (int) strlen(pKey);
	nInLen = (int) strlen(pIn);

	//int ftDesEnc(char *pKey,int pKeyLen,char *pIn,char *pOut,int Len,int pFlag=0);
	nRet = ftDesEncL(pKey, nKeyLen, pIn, pOut, nInLen, 1);

	return nRet;
}

/******************************************************************************************
 *******	函数名称：ft3DesEnc   -序号：1.19-										*******
 *******	函数功能：3DES数据加密													*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pKey   :要进行3DES加密的密钥,ASCII码32字节，BCD为16字节	*******
 *******			  int   pKeyLen:密钥长度,值为16:BCD码;32:ASCII码				*******
 *******			  char *pIn    :加密数据，ASCII为16字节、BCD码为8字节的倍数 	*******
 *******			  char *pOut   :(输出)加密生成的数据							*******
 *******              int   pLen   :要加密的数据长度必须是8的倍数					*******
 *******              int   pFlag  :数据格式标志，缺省为：0，BCD码；其他值为ASCII   *******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回<0	;错误代码如下：						*******
 *******								-1：输入密钥错误或输入加密数据错误			*******
 *******								-2：内部分配指针失败						*******
 ******************************************************************************************/
int ft3DesEncL(char *pKey, int pKeyLen, char *pIn, char *pOut, int Len,
		int pFlag) {
	unsigned char szHexKey[16 + 1];		// 十六进制的KEY
	unsigned char *pHexData;
	unsigned char szTmpInData[8 + 1];		// 加密用输入变量
	unsigned char szTmpOutData[8 + 1];	// 加密临时输出变量
	unsigned char *pOutData;			// 临时输出的ASCII数据
	unsigned char *pHexOutData;			// 临时输出的十六进制数据

	int nHexDataLen;					// 十六进制的数据长度
	int nDataItem;					// 数据项个数
	int i;

	memset(szHexKey, 0x00, sizeof(szHexKey));

	// 根据传入的数据类型检查传入数据的合法性
	if (pFlag == 0)						// 传入数据为BCD码
			{
		if ((pKeyLen != 16) || ((Len % 8) != 0) || (Len == 0)) {
			return -1;
		}

		memcpy(szHexKey, pKey, 16);		// 加密密钥
		nHexDataLen = Len;

		// 分配指针
		pHexData = new unsigned char[Len + 1];
		pHexOutData = new unsigned char[Len + 1];
		pOutData = new unsigned char[Len + 1];
		if ((pHexData == NULL) || (pHexOutData == NULL) || (pOutData == NULL)) {
			return -2;
		}

		memset(pHexData, 0x00, Len + 1);
		memset(pHexOutData, 0x00, Len + 1);
		memset(pOutData, 0x00, Len + 1);

		memcpy(pHexData, pIn, Len);
	} else								// 传入数据为ASCII码
	{
		if ((pKeyLen != 32) || ((Len % 16) != 0) || (Len == 0)) {
			return -1;
		}

		// 将ASCII码的密钥转换成BCD码的密钥
		ftAtoh(pKey, (char *) szHexKey, 16);

		// 将ASCII码的加密数据转换成BCD码的数据
		nHexDataLen = Len / 2;

		// 分配指针
		pHexData = new unsigned char[nHexDataLen + 1];
		pHexOutData = new unsigned char[nHexDataLen + 1];
		pOutData = new unsigned char[Len + 1];

		if ((pHexData == NULL) || (pHexOutData == NULL) || (pOutData == NULL)) {
			return -2;
		}

		memset(pHexData, 0x00, nHexDataLen + 1);
		memset(pHexOutData, 0x00, nHexDataLen + 1);
		memset(pOutData, 0x00, Len + 1);

		// 将传入的ASCII码数据转换成
		ftAtoh(pIn, (char *) pHexData, nHexDataLen);

	}

	// 计算用进行计算的次数
	nDataItem = nHexDataLen / 8;

	for (i = 0; i < nDataItem; i++) {
		memset(szTmpInData, 0x00, sizeof(szTmpInData));
		memset(szTmpOutData, 0x00, sizeof(szTmpOutData));

		//memcpy(pHexData + i*8,szTmpInData,8);
		memcpy(szTmpInData, pHexData + i * 8, 8);
		// DES数据加密
		CDES::RunDes(0, (char *) szTmpInData, 8, (char *) szHexKey, 16,
				(char *) szTmpOutData);

		//保存加密的数据
		memcpy(pHexOutData + i * 8, szTmpOutData, 8);
	}

	if (pFlag == 0)						// 传入数据为BCD码
			{
		memcpy(pOut, pHexOutData, nHexDataLen);

	} else {
		// 将加密的16进制数据转换成ASCII
		ftHtoa((char *) pHexOutData, (char *) pOutData, nHexDataLen);
		memcpy(pOut, pOutData, nHexDataLen * 2);

		// Add-2013-08-14 ,增加字符串结束字符
		nHexDataLen = nHexDataLen * 2;

		memcpy(pOut + nHexDataLen, "\x00", 1);

	}

	// 释放指针
	free(pHexData);
	free(pHexOutData);
	free(pOutData);

	pHexData = NULL;
	pHexOutData = NULL;
	pOutData = NULL;

	return 0;
}

//传入参数为ASCII码的3DES加密
int ft3DesEnc(char *pKey, char *pIn, char *pOut) {
	int nKeyLen;
	int nLen;
	int nRet;

	nKeyLen = (int) strlen(pKey);
	nLen = (int) strlen(pIn);

	nRet = ft3DesEncL(pKey, nKeyLen, pIn, pOut, nLen, 1);

	return nRet;
}

/******************************************************************************************
 *******	函数名称：ftDesDec   -序号：1.20-										*******
 *******	函数功能：DES数据解密													*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pKey   :要进行DES解密的密钥,ASCII码16字节，BCD为8字节	*******
 *******			  int   pKeyLen:密钥长度,值为8:BCD码;16:ASCII码					*******
 *******			  char *pIn    :解密数据，ASCII为16字节、BCD码为8字节的倍数 	*******
 *******			  char *pOut   :(输出)解密生成的数据							*******
 *******              int   pLen   :要解密的数据长度必须是8的倍数					*******
 *******              int   pFlag  :数据格式标志，缺省为：0，BCD码；其他值为ASCII   *******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回<0	;错误代码如下：						*******
 *******								-1：输入密钥错误或输入加密数据错误			*******
 *******								-2：内部分配指针失败						*******
 ******************************************************************************************/
int ftDesDecL(char *pKey, int pKeyLen, char *pIn, char *pOut, int Len,
		int pFlag) {
	unsigned char szHexKey[8 + 1];		// 十六进制的KEY
	unsigned char *pHexData;
	unsigned char szTmpInData[8 + 1];		// 加密用输入变量
	unsigned char szTmpOutData[8 + 1];	// 加密临时输出变量
	unsigned char *pOutData;			// 临时输出的ASCII数据
	unsigned char *pHexOutData;			// 临时输出的十六进制数据

	int nHexDataLen;					// 十六进制的数据长度
	int nDataItem;					// 数据项个数
	int i;

	memset(szHexKey, 0x00, sizeof(szHexKey));

	// 根据传入的数据类型检查传入数据的合法性
	if (pFlag == 0)						// 传入数据为BCD码
			{
		if ((pKeyLen != 8) || ((Len % 8) != 0) || (Len == 0)) {
			return -1;
		}

		memcpy(szHexKey, pKey, 8);		// 加密密钥
		nHexDataLen = Len;

		// 分配指针
		pHexData = new unsigned char[Len + 1];
		pHexOutData = new unsigned char[Len + 1];
		pOutData = new unsigned char[Len + 1];
		if ((pHexData == NULL) || (pHexOutData == NULL) || (pOutData == NULL)) {
			return -2;
		}

		memset(pHexData, 0x00, Len + 1);
		memset(pHexOutData, 0x00, Len + 1);
		memset(pOutData, 0x00, Len + 1);

		memcpy(pHexData, pIn, Len);
	} else								// 传入数据为ASCII码
	{
		if ((pKeyLen != 16) || ((Len % 16) != 0) || (Len == 0)) {
			return -1;
		}

		// 将ASCII码的密钥转换成BCD码的密钥
		ftAtoh(pKey, (char *) szHexKey, 8);

		// 将ASCII码的加密数据转换成BCD码的数据
		nHexDataLen = Len / 2;

		// 分配指针
		pHexData = new unsigned char[nHexDataLen + 1];
		pHexOutData = new unsigned char[nHexDataLen + 1];
		pOutData = new unsigned char[Len + 1];

		if ((pHexData == NULL) || (pHexOutData == NULL) || (pOutData == NULL)) {
			return -2;
		}

		memset(pHexData, 0x00, nHexDataLen + 1);
		memset(pHexOutData, 0x00, nHexDataLen + 1);
		memset(pOutData, 0x00, Len + 1);

		// 将传入的ASCII码数据转换成
		ftAtoh(pIn, (char *) pHexData, nHexDataLen);
	}

	// 计算用进行计算的次数
	nDataItem = nHexDataLen / 8;

	for (i = 0; i < nDataItem; i++) {
		memset(szTmpInData, 0x00, sizeof(szTmpInData));
		memset(szTmpOutData, 0x00, sizeof(szTmpOutData));

		//memcpy(pHexData + i*8,szTmpInData,8);
		memcpy(szTmpInData, pHexData + i * 8, 8);
		// DES数据加密
		CDES::RunDes(1, (char *) szTmpInData, 8, (char *) szHexKey, 8,
				(char *) szTmpOutData);

		//保存加密的数据
		memcpy(pHexOutData + i * 8, szTmpOutData, 8);
	}

	if (pFlag == 0)						// 传入数据为BCD码
			{
		memcpy(pOut, pHexOutData, nHexDataLen);

	} else {
		// 将加密的16进制数据转换成ASCII
		ftHtoa((char *) pHexOutData, (char *) pOutData, nHexDataLen);
		memcpy(pOut, pOutData, nHexDataLen * 2);

		// Add-20130814,增加输出ASCII变量的结束字符
		nHexDataLen = nHexDataLen * 2;
		memcpy(pOut + nHexDataLen, "\x00", 1);

	}

	// 释放指针
	free(pHexData);
	free(pHexOutData);
	free(pOutData);

	pHexData = NULL;
	pHexOutData = NULL;
	pOutData = NULL;

	return NO_ERROR;
}

//传入参数为ASCII码的DES解密
int ftDesDec(char *pKey, char *pIn, char *pOut) {
	int nKeyLen;
	int nLen;
	int nRet;

	nKeyLen = (int) strlen(pKey);
	nLen = (int) strlen(pIn);

	nRet = ftDesDecL(pKey, nKeyLen, pIn, pOut, nLen, 1);

	return nRet;
}

/******************************************************************************************
 *******	函数名称：ft3DesDec   -序号：1.21-										*******
 *******	函数功能：3DES数据解密													*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pKey   :要进行3DES解密的密钥,ASCII码32字节，BCD为16字节	*******
 *******			  int   pKeyLen:密钥长度,值为16:BCD码;32:ASCII码				*******
 *******			  char *pIn    :解密数据，ASCII为16字节、BCD码为8字节的倍数 	*******
 *******			  char *pOut   :(输出)解密生成的数据							*******
 *******              int   pLen   :要解密的数据长度必须是8的倍数					*******
 *******              int   pFlag  :数据格式标志，缺省为：0，BCD码；其他值为ASCII   *******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回<0	;错误代码如下：						*******
 *******								-1：输入密钥错误或输入加密数据错误			*******
 *******								-2：内部分配指针失败						*******
 ******************************************************************************************/
int ft3DesDecL(char *pKey, int pKeyLen, char *pIn, char *pOut, int Len,
		int pFlag) {
	unsigned char szHexKey[16 + 1];		// 十六进制的KEY
	unsigned char *pHexData;
	unsigned char szTmpInData[8 + 1];		// 加密用输入变量
	unsigned char szTmpOutData[8 + 1];	// 加密临时输出变量
	unsigned char *pOutData;			// 临时输出的ASCII数据
	unsigned char *pHexOutData;			// 临时输出的十六进制数据

	int nHexDataLen;					// 十六进制的数据长度
	int nDataItem;					// 数据项个数
	int i;

	memset(szHexKey, 0x00, sizeof(szHexKey));

	// 根据传入的数据类型检查传入数据的合法性
	if (pFlag == 0)						// 传入数据为BCD码
			{
		if ((pKeyLen != 16) || ((Len % 8) != 0) || (Len == 0)) {
			return -1;
		}

		memcpy(szHexKey, pKey, 16);		// 加密密钥
		nHexDataLen = Len;

		// 分配指针
		pHexData = new unsigned char[Len + 1];
		pHexOutData = new unsigned char[Len + 1];
		pOutData = new unsigned char[Len + 1];
		if ((pHexData == NULL) || (pHexOutData == NULL) || (pOutData == NULL)) {
			return -2;
		}

		memset(pHexData, 0x00, Len + 1);
		memset(pHexOutData, 0x00, Len + 1);
		memset(pOutData, 0x00, Len + 1);

		memcpy(pHexData, pIn, Len);
	} else								// 传入数据为ASCII码
	{
		if ((pKeyLen != 32) || ((Len % 16) != 0) || (Len == 0)) {
			return -1;
		}

		// 将ASCII码的密钥转换成BCD码的密钥
		ftAtoh(pKey, (char *) szHexKey, 16);

		// 将ASCII码的加密数据转换成BCD码的数据
		nHexDataLen = Len / 2;

		// 分配指针
		pHexData = new unsigned char[nHexDataLen + 1];
		pHexOutData = new unsigned char[nHexDataLen + 1];
		pOutData = new unsigned char[Len + 1];

		if ((pHexData == NULL) || (pHexOutData == NULL) || (pOutData == NULL)) {
			return -2;
		}

		memset(pHexData, 0x00, nHexDataLen + 1);
		memset(pHexOutData, 0x00, nHexDataLen + 1);
		memset(pOutData, 0x00, Len + 1);

		// 将传入的ASCII码数据转换成
		ftAtoh(pIn, (char *) pHexData, nHexDataLen);

	}

	// 计算用进行计算的次数
	nDataItem = nHexDataLen / 8;

	for (i = 0; i < nDataItem; i++) {
		memset(szTmpInData, 0x00, sizeof(szTmpInData));
		memset(szTmpOutData, 0x00, sizeof(szTmpOutData));

		//memcpy(pHexData + i*8,szTmpInData,8);
		memcpy(szTmpInData, pHexData + i * 8, 8);
		// DES数据加密
		CDES::RunDes(1, (char *) szTmpInData, 8, (char *) szHexKey, 16,
				(char *) szTmpOutData);

		//保存加密的数据
		memcpy(pHexOutData + i * 8, szTmpOutData, 8);
	}

	if (pFlag == 0)						// 传入数据为BCD码
			{
		memcpy(pOut, pHexOutData, nHexDataLen);

	} else {
		// 将加密的16进制数据转换成ASCII
		ftHtoa((char *) pHexOutData, (char *) pOutData, nHexDataLen);
		memcpy(pOut, pOutData, nHexDataLen * 2);

		//Add-2013-08-14 增加输出的字符串参数的结束字符
		nHexDataLen = nHexDataLen * 2;
		memcpy(pOut + nHexDataLen, "\x00", 1);
	}

	// 释放指针
	free(pHexData);
	free(pHexOutData);
	free(pOutData);

	pHexData = NULL;
	pHexOutData = NULL;
	pOutData = NULL;

	return 0;
}

//传入参数为ASCII码的3DES解密
int ft3DesDec(char *pKey, char *pIn, char *pOut) {
	int nKeyLen;
	int nLen;
	int nRet;

	nKeyLen = (int) strlen(pKey);
	nLen = (int) strlen(pIn);

	nRet = ft3DesDecL(pKey, nKeyLen, pIn, pOut, nLen, 1);

	return nRet;
}

/******************************************************************************************
 *******	函数名称：ftWriteLog  -序号：1.22-										*******
 *******	函数功能：日志文件记录函数												*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *FileName   :日志文件名称，相对路径或绝对路径			*******
 *******			  char *pData      :日志写入数据							 	*******
 *******			  int   pDataLen   :写入的日志数据长度					 		*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；													*******
 ******************************************************************************************/
void ftWriteLog(char *pFileName, char *format, ...) {

	struct tm *ct;
	struct timeval hmsm;

	string strTime;
	string strDate;
	string strFileName;
	string strLogBuf;
	string strCh;
	char szLogBuf[2048 + 1];
	va_list valist;
	FILE *hLogFile;
	int nPos;

	// 获取当前系统时间
	time_t st = time(NULL);
	ct = localtime(&st);
	gettimeofday(&hmsm, NULL);

	// 获取当前时间的年-月-日 - ：时：分：秒：微秒
	printf(strTime.c_str(), "%02d:%02d:%02d:%03d", ct->tm_hour, ct->tm_min,
			ct->tm_sec, hmsm.tv_usec);
	// 获取：YYYYMMDD 格式的日期
	printf(strDate.c_str(), "%04d%02d%02d", 1900 + ct->tm_year, 1 + ct->tm_mon,
			ct->tm_mday);

	// 日志文件名保存到CString对象
	printf(strFileName.c_str(), "%s", pFileName);

	// 获取日志文件扩张名位置
	nPos = (int) strFileName.find('.');
	if (nPos > 0) {
		// 在日志文件的扩张名前增加字符:_
		strFileName.insert(nPos, "_");
	}

	nPos = (int) strFileName.find('.');
	if (nPos > 0) {
		// 在日志文件的扩展名前增加日期[YYYYMMDD]
		strFileName.insert(nPos, strDate);
	}

	memset(szLogBuf, 0x00, sizeof(szLogBuf));

	// 处理传入的格式化数据
	//va_start(valist, format);
	vsnprintf(szLogBuf, 2048, format, valist);
	//va_end(valist);

	strLogBuf = szLogBuf;
	strCh = "";
	// 处理字符串前的换行字符
	while (true) {
		nPos = (int) strLogBuf.find('\n');
		if (nPos != 0)
			break;
		strCh += '\n';
		strLogBuf.erase(0);
	}

	// 已添加方式打开日志文件
	hLogFile = fopen(strFileName.c_str(), "a+");
	if (hLogFile != NULL) {
		// 写入日志文件内容
		fprintf(hLogFile, "%s%s-->%s\n", strCh.c_str(), strTime.c_str(),
				strLogBuf.c_str());		// 加入时间显示:2008-3-17
		fclose(hLogFile);
	}

}

/******************************************************************************************
 *******	函数名称：ftWriteLog  -序号：1.22-										*******
 *******	函数功能：日志文件记录函数												*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *FileName   :日志文件名称，相对路径或绝对路径			*******
 *******			  char *pData      :日志写入数据							 	*******
 *******			  int   pDataLen   :写入的日志数据长度					 		*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；													*******
 ******************************************************************************************/
void ftWriteLog2(char *pFileName, char *format, ...) {
	struct tm *ct;
	struct timeval hmsm;

	string strTime;
	string strDate;
	string strFileName;
	string strLogBuf;
	string strCh;
	char szLogBuf[2048 + 1];
	va_list valist;
	FILE *hLogFile;
	int nPos;
	int nRet;
	int nLen;
	char szDir[128];

	//log
	memset(szDir, 0x00, sizeof(szDir));

	// 获取当前目录
//	GetCurrentDirectory(128,szDir);
	getcwd(szDir, 128);

	nLen = (int) strlen(szDir);

	memcpy(szDir + nLen, "\\log", 4);
	nLen += 4;

	// 检查目录是否存在，不存在则创建目录
	// 检查目录是否存在,存在返回零，不存在返回：-1;
	nRet = access(szDir, 00);
	if (nRet != 0) {	// 目录不存在则创建目录
						// 创建目录
		nRet = mkdir(szDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (nRet == 0) {
			// 目录创建失败，则直接返回；
			return;
		}
	}

	// 获取当前系统时间
//	GetLocalTime(&st);
	time_t st = time(NULL);
	ct = localtime(&st);
	gettimeofday(&hmsm, NULL);

	// 获取当前时间的年-月-日 - ：时：分：秒：微秒
	printf(strTime.c_str(), "%02d:%02d:%02d:%03d", ct->tm_hour, ct->tm_min,
			ct->tm_sec, hmsm.tv_usec);
	// 获取：YYYYMMDD 格式的日期
	printf(strDate.c_str(), "%04d%02d%02d", 1900 + ct->tm_year, 1 + ct->tm_mon,
			ct->tm_mday);

	// 日志文件名保存到CString对象
	printf(strFileName.c_str(), "%s", pFileName);

	// 获取日志文件扩张名位置
	nPos = (int) strFileName.find('.');
	if (nPos > 0) {
		// 在日志文件的扩张名前增加字符:_
		strFileName.insert(nPos, "_");
	}

	nPos = (int) strFileName.find('.');
	if (nPos > 0) {
		// 在日志文件的扩展名前增加日期[YYYYMMDD]
		strFileName.insert(nPos, strDate);
	}

	memset(szLogBuf, 0x00, sizeof(szLogBuf));

	// 处理传入的格式化数据
	//va_start(valist, format);
	vsnprintf(szLogBuf, 2048, format, valist);
	//va_end(valist);

	strLogBuf = szLogBuf;
	strCh = "";
	// 处理字符串前的换行字符
	while (true) {
		nPos = (int) strLogBuf.find('\n');
		if (nPos != 0)
			break;
		strCh += '\n';
		strLogBuf.erase(0);
	}

	// 已添加方式打开日志文件
	hLogFile = fopen(strFileName.c_str(), "a+");
	if (nRet == 0) {
		// 写入日志文件内容
		fprintf(hLogFile, "%s%s-->%s\n", strCh.c_str(), strTime.c_str(),
				strLogBuf.c_str());		// 加入时间显示:2008-3-17
		fclose(hLogFile);
	}

}

/******************************************************************************************
 *******	函数名称：ftWriteHexLog  -序号：1,23-									*******
 *******	函数功能：记录数据的16进制日志，每行显示16个字节的BCD码					*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *FileName   :日志文件名称，相对路径或绝对路径			*******
 *******			  char *pData      :日志写入数据							 	*******
 *******			  int   pDataLen   :写入的日志数据长度					 		*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；													*******
 ******************************************************************************************/
void ftWriteHexLog(char *pFileName, char *pBuf, int pLen) {
	struct tm *ct;
	struct timeval hmsm;

	string strTime;
	string strDate;
	string strFileName;
	string strLogBuf;
	string strCh;
	FILE *hLogFile;
	int nPos;
	int nRet;
	int i;

	// 检查写入数据长度
	if (pLen <= 0) {
		return;
	}

	// 获取当前系统时间
	time_t st = time(NULL);
	ct = localtime(&st);
	gettimeofday(&hmsm, NULL);

	// 获取当前时间的年-月-日 - ：时：分：秒：微秒
	printf(strTime.c_str(), "%02d:%02d:%02d:%03d", ct->tm_hour, ct->tm_min,
			ct->tm_sec, hmsm.tv_usec);
	// 获取：YYYYMMDD 格式的日期
	printf(strDate.c_str(), "%04d%02d%02d", 1900 + ct->tm_year, 1 + ct->tm_mon,
			ct->tm_mday);

	// 日志文件名保存到CString对象
	printf(strFileName.c_str(), "%s", pFileName);

	// 获取日志文件扩张名位置
	nPos = (int) strFileName.find('.');
	if (nPos > 0) {
		// 在日志文件的扩张名前增加字符:_
		strFileName.insert(nPos, "_");
	}

	nPos = (int) strFileName.find('.');
	if (nPos > 0) {
		// 在日志文件的扩展名前增加日期[YYYYMMDD]
		strFileName.insert(nPos, strDate);
	}

	// 已添加方式打开日志文件
	hLogFile = fopen(strFileName.c_str(), "a+");
	if (hLogFile != NULL) {

		// 循环生成日志行数据,并写入日志文件
		fprintf(hLogFile, "HEX Length =[%d]\n", pLen);

		fprintf(hLogFile,
				"---------------------------------------------------\n");

		for (i = 0; i < pLen; i++) {
			fprintf(hLogFile, "%02.2X ", (unsigned char) pBuf[i]);

			// 检查是否需写入换行符号
			if ((i > 0) && ((i + 1) % 16 == 0)) {
				fprintf(hLogFile, "\n");
			}

		}

		if (i % 16 == 0) {
			fprintf(hLogFile,
					"===================================================\n\n");
		} else {
			fprintf(hLogFile,
					"\n===================================================\n\n");
		}
		// 写入日志文件内容
		//fprintf(hLogFile,"%s%s-->%s\n",(LPCTSTR)strCh,(LPCTSTR)strTime,(LPCTSTR)strLogBuf);		// 加入时间显示:2008-3-17
		fclose(hLogFile);
	}

}

// 定义拆分的项的最大字节个数
#define SPLITMACLEN 80

/******************************************************************************************
 *******	函数名称：ftSplitStr -序号：1.24-										*******
 *******	函数功能：按指定字符拆分字符串											*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pStr		:要拆分的字符串								*******
 *******			  char pSplitChar	:指定按此字符进行查分						*******
 *******			  char pData[][81]  :(输出)拆分输出项					 		*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：返回拆分生成的数据项个数										*******
 ******************************************************************************************/
int ftSplitStr(char *pStr, char pSplitChar, char pData[][SPLITMACLEN + 1]) {
	int nStartPos;
	int nCurrPos;
	int nLen;
	int nNum;
	int pStrLen;
	char szData[SPLITMACLEN + 1];

	nNum = 0;
	nStartPos = 0;
	nCurrPos = 0;

	pStrLen = (int) strlen(pStr);

	while (1) {
		if (pStr[nCurrPos] == pSplitChar) {
			// 字符串长度
			nLen = nCurrPos - nStartPos;
			if (nLen == 0) {
				memcpy(pData[nNum], "\0", 1);
			} else {
				// 拆分数据项最大长度为80字节
				if (nLen > 80)
					nLen = SPLITMACLEN;
				memset(szData, 0x00, sizeof(szData));

				memcpy(pData[nNum], pStr + nStartPos, nLen);
			}

			nNum++;
			nStartPos = nCurrPos + 1;
		}

		nCurrPos++;

		if (nCurrPos >= pStrLen) {
			break;
		}
	}

	// 检查是否有最后一个数据项
	if (nStartPos < nCurrPos) {
		nLen = nCurrPos - nStartPos;
		if (nLen > SPLITMACLEN)
			nLen = SPLITMACLEN;

		memcpy(pData[nNum], pStr + nStartPos, nLen);

		nNum++;
	}

	return nNum;
}

/******************************************************************************************
 *******	函数名称：ftSplitStr -序号：1.24-										*******
 *******	函数功能：按指定字符拆分字符串											*******
 *******----------------------------------------------------------------------------*******
 *******
 *******	函数参数：char *pStr		:要拆分的字符串								*******
 *******			  char pSplitChar	:指定按此字符进行查分						*******
 *******			  char pData[][81]  :(输出)拆分输出项					 		*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：返回拆分生成的数据项个数										*******
 ******************************************************************************************/
//int ftSplitStr(char *pStr,char pSplitChar,CStringArray *pArrItem)
//{
//	CString strMsg;
//	CString strData;
//	CString strSplit;
//	CString strTemp;
//    
//	CStringArray pDataArr;
//    
//	char    szData[81];
//	int     nPos;
//    
//    
//	memset(szData,0x00,sizeof(szData));
//    
//	//memcpy(szData,"9F62|证件类型|cn|0|1||",22);
//    
//	strData.Format("%s",pStr);
//	strSplit.Format("%c",pSplitChar);
//    
//	while ( ( nPos = strData.Find( strSplit, 0 ) ) != -1 )
//	{
//		strTemp.Empty();
//		strTemp = strData.Left( nPos );
//		pArrItem->Add(strTemp);
//        
//		strData = strData.Right( strData.GetLength() - nPos - 1 );
//	}
//    
//	nPos = (int)pArrItem->GetCount();
//    
//	// 成功返回拆分后的数据项个数
//	return nPos;
//}

/******************************************************************************************
 *******	函数名称：ftCheckStrAlpha  -序号：1.25-									*******
 *******	函数功能：检查字符串是否都为字母字符									*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pStr   :要检查的字符串									*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；													*******
 ******************************************************************************************/
int ftCheckStrAlpha(char *pStr) {
	int nLen;
	int nRet;
	int i;

	nLen = (int) strlen(pStr);

	// 检查传入的字符串是否合法
	if (nLen <= 0) {
		return FTPROTHERERR;
	}

	// 设置字符的检查结果值
	nRet = 0;

	for (i = 0; i < nLen; i++) {
		if (!((pStr[i] >= 'a' && pStr[i] <= 'z')
				|| (pStr[i] >= 'A' && pStr[i] <= 'Z'))) {
			nRet = i + 1;
		}
	}

	// nLen 返回值非零，则表示 nLen 值指定的位置的字符非法(即不是数字字符)
	if (nRet != 0) {
		return FTPROTHERERR;
	}

	return NO_ERROR;

}

/******************************************************************************************
 *******	函数名称：ftCheckStrS		-序号：1.26-								*******
 *******	函数功能：检查字符串是否都为特殊字符									*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pStr   :要检查的字符串									*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；													*******
 ******************************************************************************************/
int ftCheckStrS(char *pStr) {
	int nLen;
	int nRet;
	int i;

	nLen = (int) strlen(pStr);

	// 检查传入的字符串是否合法
	if (nLen <= 0) {
		return FTPROTHERERR;
	}

	// 设置字符的检查结果值,0 -- 特殊字符
	nRet = 0;

	for (i = 0; i < nLen; i++) {
		// 检查是否为：0x20~0x2F 之间的字符 [!"#$%&'()*+,-./]
		if (pStr[i] >= 0x20 && pStr[i] <= 0x2F) {
			continue;
		}

		// 检查是否为：0x3A~0x40 之间的字符[:;<=>?@]
		if (pStr[i] >= 0x3A && pStr[i] <= 0x40) {
			continue;
		}

		// 检索是否为：5B ~5F 之间的字符[[\]^_]
		if (pStr[i] >= 0x5B && pStr[i] <= 0x5F) {
			continue;
		}

		// 检索是否为：7B ~7E 之间的字符[[\]^_]
		if (pStr[i] >= 0x7B && pStr[i] <= 0x7E) {
			continue;
		}

		// 如果字符不是以上特殊字符则返回字符的位置
		nRet = (i + 1);
		break;

	}

	// nLen 返回值非零，则表示 nLen 值指定的位置的字符非法(即不是数字字符)
	if (nRet != 0) {
		return FTPROTHERERR;
	}

	return NO_ERROR;

}

/******************************************************************************************
 *******	函数名称：ftCheckStrBin		-序号：1.27-								*******
 *******	函数功能：二进制数据字符串合法性检查函数								*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pStr   :要检查的字符串									*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；													*******
 ******************************************************************************************/
int ftCheckStrBin(char *pStr) {
	int nLen;
	int nRet;
	int i;

	nLen = (int) strlen(pStr);

	// 检查传入的字符串是否合法
	if (nLen <= 0) {
		return FTPROTHERERR;
	}

	// 设置字符的检查结果值,0 -- 特殊字符
	nRet = 0;

	for (i = 0; i < nLen; i++) {
		// 检查是否为：数字,0x30~0x39[0~9]   'A' ~ 'F'   'a'~'f'
		if (!((pStr[i] >= 0x30 && pStr[i] <= 0x39)
				|| (pStr[i] >= 0x41 && pStr[i] <= 0x46)
				|| (pStr[i] >= 0x61 && pStr[i] <= 0x66))) {
			nRet = (i + 1);
			break;
		}
	}

	// nLen 返回值非零，则表示 nLen 值指定的位置的字符非法(即不是数字字符)
	if (nRet != 0) {
		return FTPROTHERERR;
	}

	return NO_ERROR;

}

/******************************************************************************************
 *******	函数名称：ftCheckStrAn		-序号：1.28-								*******
 *******	函数功能：检查字符串是否为字母或数字函数								*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pStr   :要检查的字符串									*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；													*******
 ******************************************************************************************/
int ftCheckStrAn(char *pStr) {
	int nLen;
	int nRet;
	int i;

	nLen = (int) strlen(pStr);

	// 检查传入的字符串是否合法
	if (nLen <= 0) {
		return FTPROTHERERR;
	}

	// 设置字符的检查结果值,0 -- 特殊字符
	nRet = 0;

	for (i = 0; i < nLen; i++) {
		// 检查是否为：数字,0x30~0x39[0~9]   'A' ~ 'Z'   'a'~'z'
		if (!((pStr[i] >= 0x30 && pStr[i] <= 0x39)
				|| (pStr[i] >= 0x41 && pStr[i] <= 0x5A)
				|| (pStr[i] >= 0x61 && pStr[i] <= 0x7A))) {
			nRet = (i + 1);
			break;
		}
	}

	// nLen 返回值非零，则表示 nLen 值指定的位置的字符非法(即不是数字字符)
	if (nRet != 0) {
		return FTPROTHERERR;
	}

	return NO_ERROR;

}

/******************************************************************************************
 *******	函数名称：ftCheckStrAs		-序号：1.29-								*******
 *******	函数功能：检查字符串是否为字母和特殊字符								*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pStr   :要检查的字符串									*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；													*******
 ******************************************************************************************/
int ftCheckStrAs(char *pStr) {
	int nLen;
	int nRet;
	int i;

	nLen = (int) strlen(pStr);

	// 检查传入的字符串是否合法
	if (nLen <= 0) {
		return FTPROTHERERR;
	}

	// 设置字符的检查结果值,0 -- 特殊字符
	nRet = 0;

	for (i = 0; i < nLen; i++) {
		// 字符为字母字符
		if ((pStr[i] >= 0x41 && pStr[i] <= 0x5A)
				|| (pStr[i] >= 0x61 && pStr[i] <= 0x7A)) {
			continue;
		} else if (pStr[i] >= 0x20 && pStr[i] <= 0x2F) // 检查是否为：0x20~0x2F 之间的字符 [!"#$%&'()*+,-./]
				{
			continue;
		} else if (pStr[i] >= 0x3A && pStr[i] <= 0x40)// 检查是否为：0x3A~0x40 之间的字符[:;<=>?@]
				{
			continue;
		} else if (pStr[i] >= 0x5B && pStr[i] <= 0x5F)// 检索是否为：5B ~5F 之间的字符[[\]^_]
				{
			continue;
		} else if (pStr[i] >= 0x7B && pStr[i] <= 0x7E)// 检索是否为：7B ~7E 之间的字符[[\]^_]
				{
			continue;
		} else {
			// 如果字符不是以上特殊字符则返回字符的位置
			nRet = (i + 1);
			break;
		}

	}

	// nLen 返回值非零，则表示 nLen 值指定的位置的字符非法(即不是数字字符)
	if (nRet != 0) {
		return FTPROTHERERR;
	}

	return NO_ERROR;

}

/******************************************************************************************
 *******	函数名称：ftCheckStrAns		-序号：1.30-								*******
 *******	函数功能：检查字符串是否为字母、数字和特殊字符							*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pStr   :要检查的字符串									*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；													*******
 ******************************************************************************************/
int ftCheckStrAns(char *pStr) {
	int nLen;
	int nRet;
	int i;

	nLen = (int) strlen(pStr);

	// 检查传入的字符串是否合法
	if (nLen <= 0) {
		return FTPROTHERERR;
	}

	// 设置字符的检查结果值,0 -- 特殊字符
	nRet = 0;

	for (i = 0; i < nLen; i++) {
		// 字符为字母字符
		if ((pStr[i] >= 0x41 && pStr[i] <= 0x5A)
				|| (pStr[i] >= 0x61 && pStr[i] <= 0x7A)) {
			continue;
		} else if (pStr[i] >= 0x30 && pStr[i] <= 0x39)// 检查是否为0x30~0x39 之间的字符[即数字字符]
				{
			continue;
		} else if (pStr[i] >= 0x20 && pStr[i] <= 0x2F) // 检查是否为：0x20~0x2F 之间的字符 [!"#$%&'()*+,-./]
				{
			continue;
		} else if (pStr[i] >= 0x3A && pStr[i] <= 0x40)// 检查是否为：0x3A~0x40 之间的字符[:;<=>?@]
				{
			continue;
		} else if (pStr[i] >= 0x5B && pStr[i] <= 0x5F)// 检索是否为：5B ~5F 之间的字符[[\]^_]
				{
			continue;
		} else if (pStr[i] >= 0x7B && pStr[i] <= 0x7E)// 检索是否为：7B ~7E 之间的字符[[\]^_]
				{
			continue;
		} else {
			// 如果字符不是以上特殊字符则返回字符的位置
			nRet = (i + 1);
			break;
		}

	}

	// nLen 返回值非零，则表示 nLen 值指定的位置的字符非法(即不是数字字符)
	if (nRet != 0) {
		return FTPROTHERERR;
	}

	return NO_ERROR;

}

/******************************************************************************************
 *******	函数名称：pfcheckFile		序号：-1.31-								*******
 *******	函数功能：检测文件是否存在，存在则获取文件长度(字节数)					*******
 *******----------------------------------------------------------------------------*******
 ******* 函数参数：char *pFileName：要检测的文件									*******
 *******           int *pFileLen：(输出)文件存在则输出文件长度(字节数)				*******
 ******* 返 回 值：成功返回：0；失败返回：<0										*******
 ******************************************************************************************/
int ftCheckFile(char *pFileName, long *pFileLen) {
	int nRet = access(pFileName, 00);

	if (nRet != 0) {
		return -1;
	}

	FILE* hFile;
	hFile = fopen(pFileName, "r+b");
	if (hFile == NULL) {
		return -2;
	}

	fseek(hFile, 0x00, SEEK_END);
	long lPos = ftell(hFile);
	*pFileLen = (int) lPos;

	return NO_ERROR;
}

/******************************************************************************************
 *******	函数名称：ftWriteLineToFile		序号：-1.32-							*******
 *******	函数功能：将缓冲区写入到新行文件										*******
 *******----------------------------------------------------------------------------*******
 ******* 函数参数：char *pFileName：要写入的文件名									*******
 *******           char *pBuf：要写入的数据											*******
 *******		   int pLen：要写入的数据长度										*******
 *******		   int pMode：写入方式;0:添加到文件尾，1：重新写文件				*******
 ******* 返 回 值：成功返回：0；失败返回：<0										*******
 ******************************************************************************************/
int ftWriteLineToFile(char *pFileName, char *pBuf, int pLen, int pMode) {
	FILE* hFile;

	if (pMode == 1) {
		hFile = fopen(pFileName, "w+b");
		if (hFile == NULL) {
			return -2;
		}
	} else if (pMode == 0) {
		hFile = fopen(pFileName, "a+b");
		if (hFile == NULL) {
			return -2;
		}
	} else {
		return -1;
	}

	if (!hFile) {
		return -2;
	}

	if (pMode == 0) {
		fseek(hFile, 0, SEEK_END);
		long lPos = ftell(hFile);

		if (lPos != 0) {
			fwrite("\n", sizeof(char), 1, hFile);
		}
	}

	fwrite(pBuf, sizeof(char), pLen, hFile);

	fflush(hFile);
	fclose(hFile);

	return NO_ERROR;
}

/******************************************************************************************
 *******	函数名称：ftDelLeftStr     序号：-1.33-                                  *******
 *******	函数功能：删除字符串前面的空格											*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pStr：(输入/输出)传入处理前的字符，返回处理后的字符串	*******
 *******	返 回 值：成功返回：处理后的字符串长度									*******
 ******************************************************************************************/
int ftDelLeftStr(char *pStr) {
	char szStr[1024 + 1];

	int i;
	int nLen;

	memset(szStr, 0x00, sizeof(szStr));
	nLen = (int) strlen(pStr);

	// 如果字符串长度大于512字节不进行处理
	if (nLen > 1024) {
		return nLen;
	}

	for (i = 0; i < nLen; i++) {
		// 检查字符是否为空格
		if (pStr[i] != 0x20) {
			break;
		}
	}

	// 检查字符串是否需要处理空格
	if (i == 0) {
		return nLen;
	}

	// 检查字符串是否全为空格
	if (i >= nLen) {
		memcpy(pStr, "\x00", 1);
		return 0;
	}

	// 删除字符串前面的空格
	memcpy(szStr, pStr + i, nLen - i);

	// 删除空格后的长度
	nLen = nLen - i;

	memcpy(pStr, szStr, nLen);
	memcpy(pStr + nLen, "\x00", 1);

	return nLen;
}

/******************************************************************************************
 *******	函数名称：ftDelRightStr    序号：-1.34-                                 *******
 *******	函数功能：删除字符串后面的空格											*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pStr：(输入/输出)传入处理前的字符，返回处理后的字符串	*******
 *******	返 回 值：成功返回：处理后的字符串长度									*******
 ******************************************************************************************/
int ftDelRightStr(char *pStr) {

	int i;
	int nLen;

	nLen = (int) strlen(pStr);

	for (i = 0; i < nLen; i++) {
		// 检查字符是否为空格
		if (pStr[nLen - (i + 1)] != 0x20) {
			break;
		}
	}

	// 检查字符串是否需要处理空格
	if (i == 0) {
		return nLen;
	}

	// 检查字符串是否全为空格
	if (i >= nLen) {
		memcpy(pStr, "\x00", 1);
		return 0;
	}

	// 删除空格后的长度
	nLen = nLen - i;

	pStr[nLen] = 0x00;

	return nLen;

}

//int ftDelAllStr(char *pStr);
/******************************************************************************************
 *******	函数名称：ftDelAllStr	   序号：-1.35-                                 *******
 *******	函数功能：删除字符串中所有空格											*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pStr：(输入/输出)传入处理前的字符，返回处理后的字符串	*******
 *******	返 回 值：成功返回：处理后的字符串长度									*******
 ******************************************************************************************/
int ftDelAllStr(char *pStr) {
	char szStr[1024 + 1];

	int i;
	int nPos = 0;
	int nLen;

	memset(szStr, 0x00, sizeof(szStr));
	nLen = (int) strlen(pStr);

	// 如果字符串长度大于512字节不进行处理
	if (nLen > 1024) {
		return nLen;
	}

	for (i = 0; i < nLen; i++) {
		// 检查字符是否为空格
		if (pStr[i] != 0x20) {
			szStr[nPos] = pStr[i];
			nPos++;
		}
	}

	memcpy(pStr, szStr, nPos);
	memcpy(pStr + nPos, "\x00", 1);

	return nLen;
}

/******************************************************************************************
 *******	函数名称：ftCheckStrNum  -序号：1.36-									*******
 *******	函数功能：检查字符串是否都为数字字符									*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pStr   :要检查的字符串									*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；													*******
 ******************************************************************************************/
int ftCheckStrNum(char *pStr) {
	int nLen;
	int nRet;
	int i;

	nLen = (int) strlen(pStr);

	// 检查传入的字符串是否合法
	if (nLen <= 0) {
		return FTPROTHERERR;
	}

	// 设置字符的检查结果值
	nRet = 0;

	for (i = 0; i < nLen; i++) {
		if (!(pStr[i] >= '0' && pStr[i] <= '9')) {
			nRet = i + 1;
		}
	}

	// nLen 返回值非零，则表示 nLen 值指定的位置的字符非法(即不是数字字符)
	if (nRet != 0) {
		return FTPROTHERERR;
	}

	return NO_ERROR;
}

/******************************************************************************************
 *******	函数名称：ftUniToStr	  -序号：1.37-									*******
 *******	函数功能：将Unicode编码的字符串转换成ANSI编码字符串						*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pStr   :要转换的Unicode编码的字符串(如:"FE668765")。	*******
 *******              char *pOut   :转换生成的ANSI编码(如:"曾文")。					*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：NO_ERROR；失败返回错误代码							*******
 ******************************************************************************************/
//int ftUniToStr(char *pStr, char *pOut)
//{
//	char szUniCode[1024];
//	char szTmp[1024];
//	int  nRet;
//    
//	memset(szUniCode, 0x00, sizeof(szUniCode));
//	memset(szTmp,     0x00, sizeof(szTmp));
//    
//	// 获取Unicode编码字符串的长度，每个Unicode 编码用两个字节来表示(如：曾 -- 的Unicode编码为：\xFE\x66)。
//	int Len = (int)strlen(pStr);
//	if ((Len % 2) != 0)
//	{
//		ftWriteLog("pubFunc.log","ftUniToStr->传入的Unicode编码数据非法,数据=[%d][%s]",Len,pStr);
//		return  FTPROTHERERR;
//	}
//    
//	// 将ASCII 码表示的Unicode编码转换成16进制值
//	ftAtoh(pStr, szTmp, Len/2);
//    
//	// 直接内存拷贝即可
//	memcpy(szUniCode,szTmp,Len/2);
//    
//	// 调用WideCharToMultiByte函数计算Unicode编码转换成GBK编码的数据长度
//	int	dwNum = WideCharToMultiByte(CP_ACP, NULL, (LPWSTR)szUniCode, -1, NULL, 0, NULL, NULL);
//    
//	// 检查函数调用结果，失败返回：0；
//	if( dwNum == 0)
//	{
//		ftWriteLog("pubFunc.log","ftUniToStr->调用WideCharToMultiByte函数,计算生成ANSI编码数据长度失败");
//		return FTPROTHERERR;
//	}
//    
//	//WideCharToMultiByte--将一个Unicode串转换成一个ANSI串。
//	// 函数原型：
//	// int WideCharToMultiByte
//	//     UINT    CodePage,		// 字符转换成的代码页。值如下：
//    // CP_ACP -- 使用当前的ANSI代码页。代码页是256个字符集。
//    // 字符：0~127 与ANSI编码一样;		字符：128 ~ 255 与 ANSI字符不同，
//    // 它可以包含图形字符或者读音符号。每一种语言或地区都有其自己的代码页。
//	//     DWORD   dwFlags,			// 确定Window如何处理“复合”Unicode 字符，它是一种后面带读音符号的字符，此处设置为：NULL；
//	//     LPCWSTR lpWideCharStr,	// 要转换的Unicode 串
//	//     int     cchWideChar,		// lpWideCharStr在Unicode字符串中的长度。通常传递：-1，表示这个串是以0x00 结尾。
//	//     LPSTR   lpMultiByteStr,	// 接受转换的串的字符缓冲；设置为：NULL 表示计算转换生成ANSI 的长度；
//	//     int     cbMultiByte,		// 转换生成的lpMultiByteStr的长度，如果计算长度则此值设置成：0，否则为一输出变量
//	//     LPCSTR  lpDefaultChar,	// ；值设置为：NULL
//	//     LPBOOL  lpUsedDefaultChar// ；值设置为：NULL
//    
//    
//	// 分配生成的ANSI字符缓冲
//	char *szGBK;
//	szGBK =(char *)malloc(dwNum);
//    
//	memset(szGBK,0x00,dwNum);
//	
//	// 指针不能使用:sizeof函数
//	//memset(szGBK, 0x00, sizeof(szGBK));
//    
//    
//	// 进行Unicode 编码转换，生成ANSI 编码数据
//	nRet = WideCharToMultiByte(CP_ACP, NULL, (LPWSTR)szUniCode, -1, szGBK, dwNum, NULL, NULL);
//    
//	if( nRet == 0)
//	{
//		ftWriteLog("pubFunc.log","ftUniToStr->调用WideCharToMultiByte函数,转换Unicode编码失败");
//		return FTPROTHERERR;
//	}
//    
//	
//	// 计算生成的ANSI编码数据长度
//	Len = (int)strlen(szGBK);
//    
//	// 保存到输出变量
//	memcpy(pOut, szGBK, Len);
//    
//	memcpy(pOut+Len,"\x00",1);
//    
//	//delete[] szGBK;
//	free(szGBK);
//	szGBK=NULL;
//    
//	return NO_ERROR;
//}

/******************************************************************************************
 *******	函数名称：ftStrToUni	  -序号：1.38-									*******
 *******	函数功能：将Unicode编码的字符串转换成ASCII码字符串						*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pStr   :转换生成的ANSI编码(如:"曾文")。					*******
 *******              char *pOut   :转换生成的Unicode编码的字符串(如:"FE668765")。	*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：NO_ERROR；失败返回错误代码							*******
 ******************************************************************************************/
//int ftStrToUni(char *pStr,char *pOut)
//{
//	wchar_t *wszGBK;
//    
//	char     wszUniCode[2048];
//	char     szTmp[2048];
//    
//	int      dwNum;
//	int      nRet;
//    
//	memset(wszUniCode, 0x00, sizeof(wszUniCode));
//    
//	ftWriteLog("pubFunc.log","ftStrToUni->传入的ANSI编码数据=[%d][%s]",strlen(pStr),pStr);
//    
//	// 计算传入的ANSI编码生成Unicode编码的数据长度
//	dwNum = MultiByteToWideChar(CP_ACP, NULL, pStr, -1, NULL, 0);
//	if( dwNum == 0)
//	{
//		ftWriteLog("pubFunc.log","ftStrToUni->调用MultiByteToWideChar函数,计算生成的Unicode编码长度失败");
//		return FTPROTHERERR;
//	}
//    
//	// 根据计算的Unicode编码数据长度，分配指针
//	wszGBK = new wchar_t[dwNum*2-1];
//	memset(wszGBK, 0x00, sizeof(wszGBK));
//	if (!wszGBK)
//	{
//		delete[] wszGBK;
//		return FTPROTHERERR;
//	}
//    
//	// 将ANSI编码转换成Unicode编码
//	nRet = MultiByteToWideChar(CP_ACP, NULL, pStr, -1, wszGBK, dwNum);
//	if( dwNum == 0)
//	{
//		ftWriteLog("pubFunc.log","ftStrToUni->调用MultiByteToWideChar函数,生成的Unicode编码失败");
//		return FTPROTHERERR;
//	}
//    
//	// 将ANSI编码:曾文才,转换生成的Unicode编码长度值为：4
//	// Unicode编码16进制值为：\xfe \x66  \x87 \x65 \x4d \x62
//    
//	// 实际生成的Unicode编码个数为：3
//	dwNum = dwNum -1;
//    
//	// 1个Unicode编码占2个字节
//	dwNum = dwNum*2;
//    
//	memset(szTmp,0x00,sizeof(szTmp));
//    
//	// 将生成的16进制Unicode编码复制到变量,复制的字符个数为：3*2 = 6
//	memcpy(szTmp, wszGBK, dwNum);
//	//memset(str, 0x00, sizeof(str));
//    
//	/* 华旭身份证阅读器不需要做高低位转换
//     for (int i = 0; i < dwNum*2-2; i += 2)
//     {
//     wszUniCode[i]   = szTmp[i+1];
//     wszUniCode[i+1] = szTmp[i];
//     }
//     */
//    
//	//memset(str, 0x00, sizeof(str));
//	ftHtoa(szTmp, pOut, dwNum);
//	delete[] wszGBK;
//	
//	// 1个16进制编码用2个ASCII码表示
//	dwNum = dwNum*2;
//    
//	memcpy(pOut+dwNum,"\x00",1);
//    
//	return NO_ERROR;
//}

/******************************************************************************************
 *******	函数名称：ftStrGetRever	  -序号：1.39-									*******
 *******	函数功能：BCD码字符串按位取反											*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pStr   :转换生成的ANSI编码(如:"曾文")。					*******
 *******              char *pOut   :转换生成的Unicode编码的字符串(如:"FE668765")。	*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：NO_ERROR；失败返回错误代码							*******
 ******************************************************************************************/
int ftStrGetRever(char *pSrc, char *pOut) {
	char szHexSrc[512 + 1] = { 0 };
	char szHexOut[512 + 1] = { 0 };
	int nLen;
	int i;

	// 最大支持的BCD码字符长度为512
	nLen = (int) strlen(pSrc);
	if (nLen > 512) {
		return FTPROTHERERR;
	}

	//将BCD字符串转换成HEX
	nLen = nLen / 2;
	ftAtoh(pSrc, szHexSrc, nLen);

	// 按字节取反
	for (i = 0; i < nLen; i++) {
		szHexOut[i] = szHexSrc[i] ^ 0xFF;
	}

	// 将取反后的HEX转换成BCD码的字符串
	memset(szHexSrc, 0x00, sizeof(szHexSrc));
	ftHtoa(szHexOut, szHexSrc, nLen);

	nLen = nLen * 2;
	memcpy(pOut, szHexSrc, nLen);
	memcpy(pOut + nLen, "\x00", 1);

	return NO_ERROR;
}

/******************************************************************************************
 *******	函数名称：ftDiversify   -序号：17-										*******
 *******	函数功能：密钥分散函数													*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *Data：  分散因子，通常为8字节的卡号(16个字符的ASCII)	*******
 *******			  char *KEY：   要进行分散的KEY									*******
 *******			  char *Out：   密钥分散输出数据(即过程密钥SK)					*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回：-1		          						*******
 ******************************************************************************************/
int ftDiversify(char *Data, char *Key, char *Out) {
	char szHexData[16 + 1];
	char szHexKey[16 + 1];
	char szHexOut[16 + 1];
	char szOut[32 + 1];
	int i;

	memset(szHexData, 0x00, sizeof(szHexData));
	memset(szHexKey, 0x00, sizeof(szHexKey));
	memset(szHexOut, 0x00, sizeof(szHexOut));
	memset(szOut, 0x00, sizeof(szOut));

	if (strlen(Data) != 16) {
		return -1;
	}

	// 分散计算密钥为32个ASCII字符
	if (strlen(Key) != 32) {
		return -2;
	}

	// 将传入数据转换成16进制的BCD码
	ftAtoh(Data, szHexData, 8);
	ftAtoh(Key, szHexKey, 16);

	// 对传入分散因子取反并添加到数据后面生成16字节的加密数据
	for (i = 0; i < 8; i++) {
		szHexData[8 + i] = ~((unsigned char) szHexData[i]);
	}

	// 对处理后的分散因子进行3DES加密,生成过程密钥SK
	// 对处理后的分散因子进行3DES加密,生成过程密钥SK
	CDES::RunDes(0, szHexData, 16, szHexKey, 16, szHexOut);

	// 将16进制的过程密钥转换成ASCII码的过程密钥
	ftHtoa(szHexOut, szOut, 16);

	// 将过程密钥保存的函数输出变量
	memcpy(Out, szOut, 32);
	memcpy(Out + 32, "\x00", 1);

	return 0;
}

/******************************************************************************************
 *******	函数名称：ftCalSK   -序号：22-											*******
 *******	函数功能：金融交易过程密钥(SK)计算函数[圈存、圈提、取现、消费等]		*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pKey   :要进行SK计算的密钥,ASCII码32字节，BCD为16字节	*******
 *******			  int   pKeyLen:密钥长度,值为16:BCD码;32:ASCII码				*******
 *******			  char *pIn    :计算过程密钥数据，ASCII为16字节、BCD码为8字节 	*******
 *******			  int   pLen   :计算数据长度，ASCII为16字节、BCD码为8字节 		*******
 *******			  char *pOut   :(输出)计算生成的过程密钥						*******
 *******              int   pFlag  :数据格式标志，缺省为：0，BCD码；其他值为ASCII   *******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回<0	;错误代码如下：						*******
 *******								-1：输入密钥错误或输入数据错误				*******
 *******                                -2：计算过程密钥错误						*******
 ******************************************************************************************/
int ftCalSKL(char *pKey, int pKeyLen, char *pIn, int pLen, char *pOut,
		int pFlag) {
	char szKey[16 + 1];
	char szHexIn[8 + 1];
	char szHexOut[8 + 1];
	char szOut[16 + 1];

	int nRet;

	memset(szKey, 0x00, sizeof(szKey));
	memset(szHexIn, 0x00, sizeof(szHexIn));
	memset(szHexOut, 0x00, sizeof(szHexOut));

	if (pFlag == 0)				// 传入参数数据为BCD码
			{
		// 检查数据合法性
		if ((pKeyLen != 16) || (pLen != 8)) {
			return -1;
		}

		// 处理传入数据
		memcpy(szKey, pKey, pKeyLen);
		memcpy(szHexIn, pIn, pLen);

	} else						// 传入参数数据为ASCII码
	{
		// 检查数据合法性
		if (((int) strlen(pKey) != pKeyLen) || (pKeyLen != 32)
				|| ((int) strlen(pIn) != pLen) || (pLen != 16)) {
			return -1;
		}

		// 处理传入数据
		ftAtoh(pKey, szKey, 16);
		ftAtoh(pIn, szHexIn, 8);
	}

	// 使用3DES加密生成过程密钥
	nRet = ft3DesEncL(szKey, 16, szHexIn, szHexOut, 8, 0);
	if (nRet != 0) {
		return -2;
	}

	if (pFlag == 0)				// 传入参数数据为BCD码
			{
		memcpy(pOut, szHexOut, 8);
	} else						// 传入参数数据为ASCII码
	{
		ftHtoa(szHexOut, szOut, 8);
		memcpy(pOut, szOut, 16);
	}

	return 0;
}

// 函数原型二
//int ftCalSK(char *pKey,int pKeyLen,char *pIn,int pLen,char *pOut,int pFlag  )
int ftCalSK(char *pKey, char *pIn, char *pOut) {
	int nKeyLen;
	int nLen;
	int nRet;

	nKeyLen = (int) strlen(pKey);
	nLen = (int) strlen(pIn);

	nRet = ftCalSKL(pKey, nKeyLen, pIn, nLen, pOut, 1);
	if (nRet != NO_ERROR) {
		return nRet;
	}

	return NO_ERROR;
}

/******************************************************************************************
 *******	函数名称：CalMac   -序号：23-											*******
 *******	函数功能：金融交易MAC计算函数[圈存、圈提、取现、消费等]					*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *Key    :计算MAC的过程密钥，8字节,BCD码					*******
 *******			  char *Vector :计算MAC的初始化向量，8字节，BCD码,通常为：\x00	*******
 *******			  char *Data   :计算MAC的数据, BCD码,为8字节的倍数          	*******
 *******			  int   pLen   :计算MAC的数据长度，	(BCD码值长度)				*******
 *******			  char *pOut   :(输出)计算返回的MAC值，BCD码，长度为4字节		*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回<0	;错误代码如下：						*******
 *******								-1：输入密钥错误或输入数据错误				*******
 *******                                -2：计算过程密钥错误						*******
 ******************************************************************************************/
int CalMac(char *Key, char *Vector, char *Data, char *AscMAC, int Flag = 1) {
	char szCommand[512];
	char szVector[64 + 1];
	char szBcdVector[32 + 1];
	char szSK[16 + 1];
	char szBcdSK[8 + 1];
	char szMAC[8 + 1];
	char szBcdMAC[4 + 1];
	char szTmp[16 + 1];

	char szXOR[8 + 1];
	int nRet;
	int nLen;
	int nBcdDataLen;    // BCD码的计算数据长度
	int nItem;			 // 计算BCD码的数据项个数
	int i;

	memset(szVector, 0x00, sizeof(szVector));
	memset(szCommand, 0x00, sizeof(szCommand));
	memset(szMAC, 0x00, sizeof(szMAC));
	memset(szSK, 0x00, sizeof(szSK));
	memset(szTmp, 0x00, sizeof(szTmp));
	memset(szBcdMAC, 0x00, sizeof(szBcdMAC));
	memset(szXOR, 0x00, sizeof(szXOR));

	nRet = -1;

	// 检查传入参数，的长度是否合法，过程密钥：16个ASCII、初始化向量：16个ASCII；计算数据：16必须是16的倍数
	if ((strlen(Key) != 16) || (strlen(Vector) != 16)
			|| (strlen(Data) % 16 != 0)) {
		return -1;
	}

	memset(szBcdVector, 0x00, sizeof(szBcdVector));
	memset(szBcdSK, 0x00, sizeof(szBcdSK));

	// 设置初始化向量
	sprintf(szVector, Vector);

	// 设置SK
	sprintf(szSK, Key);

	nLen = (int) strlen(Data);

	nBcdDataLen = nLen / 2;				// BCD码的数据项长度

	nItem = nBcdDataLen / 8;			// 要进行拆分的数据项个数

	char *szBcdData;

	szBcdData = (char *) malloc(nBcdDataLen + 1);
	if (szBcdData == NULL) {
		return -2;
	}

	memset(szBcdData, 0x00, nBcdDataLen + 1);

	// 将ASCII的数据项都转换成BCD码的数据项
	ftAtoh(szSK, szBcdSK, 8);
	ftAtoh(szVector, szBcdVector, 8);
	ftAtoh(Data, szBcdData, nBcdDataLen);

	for (i = 0; i < nItem; i++) {
		// 1.对数据进行XOR运行
		memset(szTmp, 0x00, sizeof(szTmp));

		memcpy(szTmp, szBcdData + i * 8, 8);
		// 对数据进行异或运行
		ftCalXOR(szBcdVector, szTmp, 8, szXOR);
		memset(szTmp, 0x00, sizeof(szTmp));
		// 对异或值进行DES加密
		CDES::RunDes(0, szXOR, 8, szBcdSK, 8, szTmp);
		memcpy(szBcdVector, szTmp, 8);
	}

	free(szBcdData);
	szBcdData = NULL;

	// szBcdVector 前四字节为MAC值
	memcpy(szBcdMAC, szBcdVector, 4);
	ftHtoa(szBcdMAC, szMAC, 4);

	memcpy(AscMAC, szMAC, 8);
	memcpy(AscMAC + 8, "\x00", 1);

	return 0;

}
/******************************************************************************************
 *******	函数名称：CalMac   -序号：23-											*******
 *******	函数功能：金融交易MAC计算函数[圈存、圈提、取现、消费等]					*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *Key    :计算MAC的过程密钥，8字节,BCD码					*******
 *******			  char *Vector :计算MAC的初始化向量，8字节，BCD码,通常为：\x00	*******
 *******			  char *Data   :计算MAC的数据, BCD码,为8字节的倍数          	*******
 *******			  int   pLen   :计算MAC的数据长度，	(BCD码值长度)				*******
 *******			  char *pOut   :(输出)计算返回的MAC值，BCD码，长度为4字节		*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回<0	;错误代码如下：						*******
 *******								-1：输入密钥错误或输入数据错误				*******
 *******                                -2：计算过程密钥错误						*******
 ******************************************************************************************/
int ftCalMacL(char *Key, char *Vector, char *Data, int DataLen, char *Out,
		int Flag) {
	char szKey[8 + 1];
	char szVector[8 + 1];
	char szData[32 + 1];
	char sItem[4][9];
	char szTmp[8 + 1];
	char szXORVal[8 + 1];
	int nItem = 0;
	int nRet;
	int i;

	memset(szKey, 0x00, sizeof(szKey));
	memset(szVector, 0x00, sizeof(szVector));
	memset(szData, 0x00, sizeof(szData));
	memset(sItem, 0x00, sizeof(char) * 4 * 9);
	memset(szTmp, 0x00, sizeof(szTmp));

	// 计算的数据长度必输是8的倍数
	if ((DataLen % 8 != 0) || (DataLen <= 0)) {
		return -1;
	}
	if (Flag == 0) {
		return -1;
	}

	nItem = DataLen / 8;
	nItem = nItem / 2;
	if (nItem > 4 || nItem <= 0) {
		return -2;
	}

	if (Flag == 0) {	// 传入参数为BCD码
		memcpy(szKey, Key, 8);
		memcpy(szVector, Vector, 8);
		memcpy(szData, Data, DataLen);
	} else {	// 传入参数为ASCII码
		ftAtoh(Key, szKey, 8);
		ftAtoh(Vector, szVector, 8);
		ftAtoh(Data, szData, DataLen / 2);
	}

	for (i = 0; i < nItem; i++) {
		memcpy(sItem[i], szData + i * 8, 8);
	}

	memcpy(szTmp, szVector, 8);
	for (i = 0; i < nItem; i++) {
		memset(szXORVal, 0x00, sizeof(szXORVal));
		ftCalXOR(szTmp, sItem[i], 8, szXORVal);

		memset(szTmp, 0x00, sizeof(szTmp));

		nRet = ftDesEncL(szKey, 8, szXORVal, szTmp, 8, 0);
		if (nRet != 0) {
			return -3;
		}
	}
	// 0 - BCD码
	if (Flag == 0) {
		//取前4字节作为：MAC
		memcpy(Out, szTmp, 4);
	} else			// 非0-ASCII码
	{
		ftHtoa(szTmp, Out, 4);
		memcpy(Out + 8, "\x00", 1);
	}
	return 0;
}

// 金融交易MAC计算方法二
int ftCalMac(char *Key, char *Vector, char *Data, char *Out) {
	int nRet;
	int nLen;

	nLen = (int) strlen(Data);
	nRet = ftCalMacL(Key, Vector, Data, nLen, Out, 1);
	if (nRet != NO_ERROR) {
		return nRet;
	}

	return NO_ERROR;
}

/*
 int ftCalMac(char *Key,char *Vector,char *Data,int DataLen,char *Out,int Flag)
 {
 char szKey[8+1];
 char szVector[8+1];
 char szData[32+1];
 char sItem[4][9];
 char szTmp[8+1];
 char szXORVal[8+1];
 int  nItem = 0;
 int  nRet;
 int  i;
 
 
 memset(szKey   , 0x00, sizeof(szKey));
 memset(szVector, 0x00, sizeof(szVector));
 memset(szData  , 0x00, sizeof(szData));
 memset(sItem   , 0x00, sizeof(char)*4*9);
 memset(szTmp   , 0x00, sizeof(szTmp));
 
 // 计算的数据长度必输是8的倍数
 if( (DataLen % 8 != 0 ) || (DataLen <= 0) )
 {
 return -1;
 }
 
 nItem = DataLen / 8;
 if(nItem > 4 || nItem <= 0)
 {
 return -2;
 }
 
 if( Flag == 0)
 {	// 传入参数为BCD码
 memcpy(szKey,Key,8);
 memcpy(szVector,Vector,8);
 memcpy(szData,Data,DataLen);
 }else
 {	// 传入参数为ASCII码
 ftAtoh(Key   , szKey    , 8);
 ftAtoh(Vector, szVector , 8);
 ftAtoh(Data  , szData   ,	DataLen);
 }
 
 
 for(i=0; i < nItem; i++)
 {
 memcpy(sItem[i], szData + i*8, 8);
 }
 
 memcpy(szTmp,szVector,8);
 for(i=0; i<nItem;i++)
 {
 memset(szXORVal,0x00,sizeof(szXORVal));
 ftCalXOR(szTmp,sItem[i],8,szXORVal);
 
 memset(szTmp,0x00,sizeof(szTmp));
 
 nRet = ftDesEnc(szKey,8,szXORVal,szTmp,8);
 if( nRet != 0)
 {
 return -3;
 }
 }
 
 
 // 0 - BCD码
 if( Flag == 0)
 {
 //取前4字节作为：MAC
 memcpy(Out,szTmp,4);
 }else			// 非0-ASCII码
 {
 ftHtoa(szTmp,Out,4);
 }
 
 
 
 
 return 0;
 }
 */

/******************************************************************************************
 *******	函数名称：CalMacFor3Des   -序号：24-									*******
 *******	函数功能：无特殊情况的MAC计算函数										*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *Key     :计算MAC的密钥，32个字符的ASCII					*******
 *******			  char *InitData:计算MAC的初始化向量，16个字符的ASCII			*******
 *******			  char *Data    :计算MAC的数据,8的倍数,ASCII		          	*******
 *******			  char *Out     :(输出)计算返回的MAC值， ASCII					*******
 *******              int  Flag		:0-输出4字节,PBOC1MAC计算;1-输出8字节PBOC3.0用	*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回<0	;错误代码如下：						*******
 *******								-1：输入密钥错误或输入数据错误				*******
 *******                                -2：计算过程密钥错误						*******
 ******************************************************************************************/
int ftCalMacFor3Des(char *Key, char *InitData, char *Data, char *Out,
		int Flag) {
	char szKey[16 + 1];			// BCD码的密钥				16字节
	char szKeyL[8 + 1];			// BCD码的密钥前半部分		8字节
	char szInitData[8 + 1];		// BCD码的初始值			8字节
	char *pData;                // BCD码的计算数据
	char szItem[8 + 1];			// BCD码的当前处理数据项	8字节
	char szTmp[8 + 1];			// 进行XOR计算的数据项		8字节
	char szXORVal[8 + 1];			// XOR结果值				8字节
	int nItem = 0;
	int nDataLen;
	char szMac[8 + 1];
	int i;

	memset(szKey, 0x00, sizeof(szKey));
	memset(szInitData, 0x00, sizeof(szInitData));
	memset(szItem, 0x00, sizeof(szItem));
	memset(szTmp, 0x00, sizeof(szTmp));

	// 传入数据合法性检查
	if (strlen(Key) != 32 || strlen(InitData) != 16) {

		return -1;
	}

	// 检查传入数据的合法性，BCD码长度必须是8的倍数
	nDataLen = (int) strlen(Data);
	if (nDataLen % 2 != 0) {

		return -1;
	}

	// 取传入数据的BCD码长度
	nDataLen = nDataLen / 2;

	// 检查传入的数据是否为8的倍数
	if (nDataLen % 8 != 0) {

		return -1;
	}

	// 按8字节拆分的数据总项数
	nItem = nDataLen / 8;

	pData = (char *) malloc(nDataLen + 1);
	if (pData == NULL) {

		return -1;
	}
	memset(pData, 0x00, nDataLen + 1);

	// 将传入数据的ASCII码转换成BCD码
	// 数据由ACII转换成BCD码
	ftAtoh(Data, pData, nDataLen);

	// 计算密钥
	ftAtoh(Key, szKey, 16);

	// 密钥的左半部分(即前8字节)
	memcpy(szKeyL, szKey, 8);

	// MAC计算的初始化数据 4 字节随机数 + 4字节 0x00
	ftAtoh(InitData, szInitData, 8);

	// 计算MAC
	for (i = 0; i < nItem; i++) {
		// 处理的数据项
		memcpy(szItem, pData + i * 8, 8);
		ftCalXOR(szItem, szInitData, 8, szXORVal);
		if ((i + 1) != nItem) {
			// DES加密
			CDES::RunDes(0, szXORVal, 8, szKeyL, 8, szTmp);
		} else {
			// 3DES加密
			CDES::RunDes(0, szXORVal, 8, szKey, 16, szTmp);
		}
		memcpy(szInitData, szTmp, 8);
	}

	// 计算的MAC返回 ,
	if (Flag == 0) {
		memset(szMac, 0x00, sizeof(szMac));
		ftHtoa(szTmp, szMac, 4);
		memcpy(Out, szMac, 8);
		memcpy(Out + 8, "\x00", 1);
	} else		// PBOC3.0 的密文数据计算输出
	{
		memset(szMac, 0x00, sizeof(szMac));
		ftHtoa(szTmp, szMac, 8);
		memcpy(Out, szMac, 16);
		memcpy(Out + 16, "\x00", 1);
	}

	// 释放指针
	free(pData);
	pData = NULL;

	return 0;
}

/******************************************************************************************
 *******	函数名称：CalMacForDes   -序号：24-B									*******
 *******	函数功能：无特殊情况的MAC计算函数										*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *Key     :计算MAC的密钥，32个字符的ASCII					*******
 *******			  char *InitData:计算MAC的初始化向量，16个字符的ASCII			*******
 *******			  char *Data    :计算MAC的数据,8的倍数,ASCII		          	*******
 *******			  char *Out     :(输出)计算返回的MAC值， ASCII					*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；失败返回<0	;错误代码如下：						*******
 *******								-1：输入密钥错误或输入数据错误				*******
 *******                                -2：计算过程密钥错误						*******
 ******************************************************************************************/
int ftCalMacForDes(char *Key, char *InitData, char *Data, char *Out) {
	char szKey[16 + 1];			// BCD码的密钥				16字节
	char szInitData[8 + 1];		// BCD码的初始值			8字节
	char *pData;                // BCD码的计算数据
	char szItem[8 + 1];			// BCD码的当前处理数据项	8字节
	char szTmp[8 + 1];			// 进行XOR计算的数据项		8字节
	char szXORVal[8 + 1];			// XOR结果值				8字节
	int nItem = 0;				// 分解成8字节一组的数据项总数
	int nDataLen;				// BCD码的数据长度
	char szMac[8 + 1];
	int i;

	memset(szKey, 0x00, sizeof(szKey));
	memset(szInitData, 0x00, sizeof(szInitData));
	memset(szItem, 0x00, sizeof(szItem));
	memset(szTmp, 0x00, sizeof(szTmp));

	// 传入数据合法性检查
	if (strlen(Key) != 16 || strlen(InitData) != 16) {
		return -1;
	}

	// 检查传入数据的合法性，BCD码长度必须是8的倍数
	nDataLen = (int) strlen(Data);
	if (nDataLen % 2 != 0) {

		return -1;
	}

	// 取传入数据的BCD码长度
	nDataLen = nDataLen / 2;

	// 检查传入的数据是否为8的倍数
	if (nDataLen % 8 != 0) {

		return -1;
	}

	// 按8字节拆分的数据总项数
	nItem = nDataLen / 8;

	pData = (char *) malloc(nDataLen + 1);
	if (pData == NULL) {

		return -1;
	}
	memset(pData, 0x00, nDataLen + 1);

	// 将传入数据的ASCII码转换成BCD码
	// 数据由ACII转换成BCD码
	ftAtoh(Data, pData, nDataLen);

	// 计算密钥
	ftAtoh(Key, szKey, 8);

	// MAC计算的初始化数据 4 字节随机数 + 4字节 0x00
	ftAtoh(InitData, szInitData, 8);

	// 计算MAC
	for (i = 0; i < nItem; i++) {
		// 处理的数据项
		memcpy(szItem, pData + i * 8, 8);
		ftCalXOR(szItem, szInitData, 8, szXORVal);

		CDES::RunDes(0, szXORVal, 8, szKey, 8, szTmp);
		memcpy(szInitData, szTmp, 8);
	}

	// 计算的MAC返回
	memset(szMac, 0x00, sizeof(szMac));
	ftHtoa(szTmp, szMac, 4);

	memcpy(Out, szMac, 8);

	// 释放指针
	free(pData);
	pData = NULL;

	return 0;
}

/******************************************************************************************
 *******	函数名称：ftGetProtocol	   -序号：15-									*******
 *******	函数功能：根据上电复位信息[BCD码值]计算IC卡的通信协议,					*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：unsigned char *pATR：要计算通信协议的ATR字节流[BCD码]			*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：IC卡的通信协议；0=通信协议为：T=0;1=通信协议为T=1				*******
 ******************************************************************************************/
int ftGetProtocolForATR(unsigned char *pATR) {
	int nTDFlag = 0;
	int nTCFlag = 0;
	int nTBFlag = 0;
	int nTAFlag = 0;
	int nPos;

	int nProtocol = 0;		// IC卡通信协议，缺省为：T=0
	unsigned char nT0;

	// 获取ATR字节流中 T0 [ATR第2字节] 值
	nT0 = (unsigned char) pATR[1] & 0xF0;

	// 检测TA1是否存在
	if ((nT0 & 0x10) != 0) {
		nTAFlag = 1;
	}

	//
	// 检测TB1是否存在
	if ((nT0 & 0x20) != 0) {
		nTBFlag = 1;
	}

	// 检测TC1是否存在
	if ((nT0 & 0x40) != 0) {
		nTCFlag = 1;
	}

	// 检测TD1是否存在
	if ((nT0 & 0x80) != 0) {
		nTDFlag = 1;
	}

	// 无TD1 则通信协议为：T= 0
	if (nTDFlag == 0) {
		nProtocol = 0;
	} else {
		// 计算TD1 值的位置
		nPos = 0;
		if (nTAFlag)
			nPos += 1;
		if (nTBFlag)
			nPos += 1;
		if (nTCFlag)
			nPos += 1;

		nT0 = pATR[nPos + 2];

		nProtocol = nT0 & 0x01;
	}

	return nProtocol;

}

/******************************************************************************************
 *******	函数名称：ftGetErrCodeAndMsg        -序号：28-							*******
 *******	函数功能：获取前一个调用的错误代码和错误信息							*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：long *ErrCode：错误代码										*******
 *******			  char *ErrMsg：错误信息										*******
 *******----------------------------------------------------------------------------*******
 ******************************************************************************************/
//int ftGetErrCodeAndMsg(long *ErrCode,char *ErrMsg)
//{
//	DWORD	dwErrCode;
//	char	szErrMsg[256];
//	int		nErrLen;
//    
//	memset(szErrMsg, 0x00, sizeof(szErrMsg));
//    
//	// 获取错误代码
//	dwErrCode = GetLastError();
//	*ErrCode  = (long)dwErrCode;
//    
//	HLOCAL hlocal = NULL;
//    
//	// 格式化错误信息
//	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
//                  NULL,
//                  dwErrCode,
//                  MAKELANGID(LANG_ENGLISH,
//                             SUBLANG_ENGLISH_US),
//                  (PTSTR)&hlocal,
//                  0,
//                  NULL);
//    
//	if (hlocal != NULL)
//	{
//		sprintf_s(szErrMsg, "%s", (LPCTSTR)LocalLock(hlocal));
//		nErrLen = (int)strlen(szErrMsg);
//		memcpy(ErrMsg, szErrMsg, nErrLen);
//		LocalFree(hlocal);
//	}
//	else
//	{
//		return 0;
//	}
//    
//	return 1;
//}

/******************************************************************************************
 *******	函数名称：ftWriteFile		序号：-31-									*******
 *******	函数功能：将缓冲区写入到文件											*******
 *******----------------------------------------------------------------------------*******
 ******* 函数参数：char *pFileName：要写入的文件名									*******
 *******           char *pBuf：要写入的数据											*******
 *******		   int pLen：要写入的数据长度										*******
 *******		   int pMode：写入方式;0:添加到文件尾，1：重新写文件				*******
 ******* 返 回 值：成功返回：0；失败返回：<0										*******
 ******************************************************************************************/
int ftWriteFile(char *pFileName, char *pBuf, int pLen, int pMode) {
	FILE* hFile;
	int nRet;

	if (pMode == 0) {
		hFile = fopen(pFileName, "w+b");

		////nRet = fopen_s(&hLogFile,(LPCTSTR)strFileName,"a+");
	} else if (pMode == 1) {
		hFile = fopen(pFileName, "a+b");
	} else {
		return -2;
	}

	fwrite(pBuf, sizeof(char), pLen, hFile);

	fflush(hFile);
	fclose(hFile);

	return 0;
}

/******************************************************************************************
 *******	函数名称：ftGetSysDate		序号：-32-									*******
 *******	函数功能：获取当前系统日期												*******
 *******----------------------------------------------------------------------------*******
 ******* 函数参数：char *pSysDate(输出)指定格式的系统日期							*******
 *******           int pFlag：输出格式,格式常量如下									*******
 *******				0：'YYYYMMDD'												*******
 *******				1: 'YYYY-MM-DD'												*******
 *******				2：'YYYY/MM/DD												*******
 ******* 返 回 值：成功返回：0；失败返回：<0										*******
 ******************************************************************************************/
int ftGetSysDate(char *pSysDate, int pFlag) {
	char szYear[11];
	int nLen;

	// 获取当前系统时间
	time_t st = time(NULL);
	struct tm *ct = localtime(&st);

	memset(szYear, 0x00, sizeof(szYear));

	switch (pFlag) {
	case 0:
		sprintf(szYear, "%.4d%.2d%.2d", 1900 + ct->tm_year, 1 + ct->tm_mon,
				ct->tm_mday);
		break;
	case 1:
		sprintf(szYear, "%.4d-%.2d-%.2d", 1900 + ct->tm_year, 1 + ct->tm_mon,
				ct->tm_mday);
		break;
	case 2:
		sprintf(szYear, "%.4d/%.2d/%.2d", 1900 + ct->tm_year, 1 + ct->tm_mon,
				ct->tm_mday);
		break;
	default:
		sprintf(szYear, "%.4d%.2d%.2d", 1900 + ct->tm_year, 1 + ct->tm_mon,
				ct->tm_mday);
		break;
	}

	nLen = (int) strlen(szYear);

	memcpy(pSysDate, szYear, strlen(szYear));
	pSysDate[nLen] = 0x00;

	return 0;
}

/******************************************************************************************
 *******	函数名称：ftGetSysTime		序号：-33-									*******
 *******	函数功能：获取当前系统时间												*******
 *******----------------------------------------------------------------------------*******
 ******* 函数参数：char *pSysTime(输出)指定格式的系统日期							*******
 *******           int pFlag：输出格式,格式常量如下									*******
 *******				0：'HHMMSS'													*******
 *******				1: 'HH:MM:SS'												*******
 *******				2：'HHMMSSMMM'												*******
 *******				3：'HH:MM:SS:MMM'											*******
 ******* 返 回 值：成功返回：0；失败返回：<0										*******
 ******************************************************************************************/
int ftGetSysTime(char *pSysTime, int pFlag) {
	char szTiems[13];
	int nLen;
	struct tm *local_time;
	struct timeval local_seconds;

	memset(szTiems, 0x00, sizeof(szTiems));

	// 获取当前系统时间
	time_t st = time(NULL);
	local_time = localtime(&st);
	gettimeofday(&local_seconds, NULL);

	switch (pFlag) {
	case 0:
		sprintf(szTiems, "%.02d%.2d%.2d", local_time->tm_hour,
				local_time->tm_min, local_time->tm_sec);
		break;
	case 1:
		sprintf(szTiems, "%.2d:%.2d:%.2d", local_time->tm_hour,
				local_time->tm_min, local_time->tm_sec);
		break;
	case 2:
		sprintf(szTiems, "%.2d%.2d%.2d%.3d", local_time->tm_hour,
				local_time->tm_min, local_time->tm_sec, local_seconds.tv_usec);
		break;
	case 3:
		sprintf(szTiems, "%.2d:%.2d:%.2d:%.3d", local_time->tm_hour,
				local_time->tm_min, local_time->tm_sec, local_seconds.tv_usec);
		break;
	default:
		sprintf(szTiems, "%.2d%.2d%.2d", local_time->tm_hour,
				local_time->tm_min, local_time->tm_sec);
	}

	nLen = (int) strlen(szTiems);

	memcpy(pSysTime, szTiems, strlen(szTiems));
	pSysTime[nLen] = 0x00;

	return 0;
}

/******************************************************************************************
 *******	函数名称：ftGetWeek		序号：-34-										*******
 *******	函数功能：获取当前系统日期的星期字符串									*******
 *******----------------------------------------------------------------------------*******
 ******* 函数参数：char *pWeekStr(输出)格式的星期字符串								*******
 *******           int pFlag：输出格式,格式常量如下									*******
 *******				0：'中文格式'												*******
 *******				1: '英文格式'												*******
 *******				2：'数字表示的星期值										*******
 ******* 返 回 值：成功返回：0；失败返回：-1										*******
 ******************************************************************************************/
int ftGetWeek(char *pWeekStr, int pFlag) {

	// 获取当前系统时间
	time_t st = time(NULL);
	struct tm *ct = localtime(&st);

	char szWeek[10];
	memset(szWeek, 0x00, sizeof(szWeek));

	char strWeek[7][10];
	memset(strWeek, 0x00, 7 * 10);

	memcpy(strWeek[0], "星期日", 6);
	memcpy(strWeek[1], "星期一", 6);
	memcpy(strWeek[2], "星期二", 6);
	memcpy(strWeek[3], "星期三", 6);
	memcpy(strWeek[4], "星期四", 6);
	memcpy(strWeek[5], "星期五", 6);
	memcpy(strWeek[6], "星期六", 6);

	char strWeekEn[7][10];
	memset(strWeekEn, 0x00, 7 * 10);

	memcpy(strWeekEn[0], "Sunday", 6);
	memcpy(strWeekEn[1], "Monday", 6);
	memcpy(strWeekEn[2], "Tuesday", 7);
	memcpy(strWeekEn[3], "Wednesday", 9);
	memcpy(strWeekEn[4], "Thursday", 8);
	memcpy(strWeekEn[5], "Friday", 6);
	memcpy(strWeekEn[6], "Saturday", 8);

	switch (pFlag) {
	case 0:
		strcpy(szWeek, strWeek[ct->tm_wday]);
		break;
	case 1:
		strcpy(szWeek, strWeekEn[ct->tm_wday]);
		break;
	case 2:
		sprintf(szWeek, "%d", ct->tm_wday);
		break;
	default:
		strcpy(szWeek, strWeek[ct->tm_wday]);
	}

	memcpy(pWeekStr, szWeek, sizeof(szWeek));

	return 0;
}

/******************************************************************************************
 *******	函数名称：ftCheckLeepYear		序号：-35-								*******
 *******	函数功能：检测指定的年份是否为润年										*******
 *******----------------------------------------------------------------------------*******
 ******* 函数参数：int pYear指定要检查的年值										*******
 ******* 返 回 值：1:是润年返回；0:非润年返回;-1:输入的年非法						*******
 ******************************************************************************************/
int ftCheckLeepYear(int pYear) {
	if (pYear <= 0)	//年非法
			{
		return -1;
	}

	if ((((pYear % 4) == 0) && ((pYear % 100) != 0)) || ((pYear % 400) == 0)) {
		return 1;
	} else {
		return 0;
	}
}

/******************************************************************************************
 *******	函数名称：ftCheckDate		序号：-36-									*******
 *******	函数功能：日期合法性检测												*******
 *******----------------------------------------------------------------------------*******
 ******* 函数参数：char *pDate：要检测的日期，格式为：YYYYMMDD						*******
 ******* 返 回 值：成功返回：0；失败返回<0 ,失败返回值描述							*******
 *******						-1：日期长度非法									*******
 *******						-2：日期包含非法字符								*******
 *******						-3：年非法											*******
 *******						-4：月非法											*******
 *******						-5：日非法											*******
 ******************************************************************************************/
int ftCheckDate(char *pDate) {
	char szYear[5];
	char szMonth[3];
	char szDate[3];
	int nYear;
	int nMonth;
	int nDay;
	int i = 0;
	int nLeepYear;
	int nMaxDay;

	if (strlen(pDate) != 8)	//判断输入字符串长度是否合格
			{
		return -1;
	}

	for (; i < 8; i++)	//输入字符串中的字符必须保持为‘0’~‘9’
			{
		if (!(isdigit(pDate[i]))) {
			return -2;
		}
	}

	memset(szYear, 0x00, sizeof(szYear));
	memset(szMonth, 0x00, sizeof(szMonth));
	memset(szDate, 0x00, sizeof(szDate));

	memcpy(szYear, pDate, 4);
	memcpy(szMonth, pDate + 4, 2);
	memcpy(szDate, pDate + 6, 2);

	nYear = atoi(szYear);
	nMonth = atoi(szMonth);
	nDay = atoi(szDate);

	//检查年份是否有误
	if (nYear <= 0) {
		return -3;
	}

	//判断是否为闰年
	nLeepYear = ftCheckLeepYear(nYear);

	//检查日期中的月是否合法
	if ((nMonth <= 0) || (nMonth > 12)) {
		return -4;
	}

	// 获取当前月份的最大天数
	nMaxDay = -1;
	nMaxDay = gMaxDayMonth[nMonth];

	if ((nLeepYear == 1) && (nMonth == 2))	//对闰年二月的最大天数的处理
			{
		nMaxDay += 1;
	}

	// 检查日期中的日是否合法
	if ((nDay <= 0) || (nDay > nMaxDay))	//日期中的日的值有误
			{
		return -5;
	}

	return 0;
}

/******************************************************************************************
 *******	函数名称：ftGetYear		序号：-37-										*******
 *******	函数功能：获取日期中的年												*******
 *******----------------------------------------------------------------------------*******
 ******* 函数参数：char *pDay日期字符串格式为：YYYYMMDD								*******
 ******* 返 回 值：成功返回日期字符串的年；失败返回:<0								*******
 ******************************************************************************************/
int ftGetYear(char *pDay) {
	char szYear[5];
	int nYear = -1;
	int i = 0;

	i = ftCheckDate(pDay);
	if (i != 0) {
		return i;
	}

	memset(szYear, 0x00, sizeof(szYear));
	memcpy(szYear, pDay, 4);
	nYear = atoi(szYear);

	return nYear;
}

/******************************************************************************************
 *******	函数名称：ftGetMonth		序号：-38-									*******
 *******	函数功能：获取日期中的月												*******
 *******----------------------------------------------------------------------------*******
 ******* 函数参数：char *pDay日期字符串格式为：YYYYMMDD								*******
 ******* 返 回 值：成功返回日期字符串的年；失败返回:<0								*******
 ******************************************************************************************/
int ftGetMonth(char *pDay) {
	char szMonth[3];
	int nMonth = -1;
	int i = 0;

	i = ftCheckDate(pDay);
	if (i != 0) {
		return i;
	}

	memset(szMonth, 0x00, sizeof(szMonth));
	memcpy(szMonth, pDay + 4, 2);
	nMonth = atoi(szMonth);

	return nMonth;
}
/******************************************************************************************
 *******	函数名称：ftGetDay		序号：-39-										*******
 *******	函数功能：获取日期中的日												*******
 *******----------------------------------------------------------------------------*******
 ******* 函数参数：char *pDay日期字符串格式为：YYYYMMDD								*******
 ******* 返 回 值：成功返回日期字符串的年；失败返回:<0								*******
 ******************************************************************************************/
int ftGetDay(char *pDay) {
	char szDate[3];
	int nDate;
	int i = 0;

	i = ftCheckDate(pDay);
	if (i != 0) {
		return i;
	}

	memset(szDate, 0x00, sizeof(szDate));
	memcpy(szDate, pDay + 6, 2);
	nDate = atoi(szDate);

	return nDate;
}

/******************************************************************************************
 *******	函数名称：ftSplitDate		序号：-40-									*******
 *******	函数功能：将YYYYMMDD格式日期拆分成整型的年、月、日						*******
 *******----------------------------------------------------------------------------*******
 ******* 函数参数：char *pDate：日期字符串格式为：YYYYMMDD							*******
 *******		   int *pYear：(输出)日期的年										*******
 *******		   int *pMonth：(输出)日期的月										*******
 *******		   int *pDay：(输出)日期的日										*******
 ******* 返 回 值：成功返回:0；失败返回:<0											*******
 ******************************************************************************************/
int ftSplitDate(char *pDate, int *pYear, int *pMonth, int *pDay) {
	char szYear[5];
	char szMonth[3];
	char szDate[3];
	int nYear;
	int nMonth;
	int nDay;
	int i = 0;
	int nLeepYear = 0;
	int nMaxDay = 0;

	i = ftCheckDate(pDate);
	if (i != 0) {
		return i;
	}

	memset(szYear, 0x00, sizeof(szYear));
	memset(szMonth, 0x00, sizeof(szMonth));
	memset(szDate, 0x00, sizeof(szDate));

	memcpy(szYear, pDate, 4);
	memcpy(szMonth, pDate + 4, 2);
	memcpy(szDate, pDate + 6, 2);

	nYear = atoi(szYear);
	nMonth = atoi(szMonth);
	nDay = atoi(szDate);

	*pYear = nYear;
	*pMonth = nMonth;
	*pDay = nDay;

	return 0;
}

/******************************************************************************************
 *******	函数名称：ftDiffTime       序号：-41-                                   *******
 *******	函数功能：计算两个时间的差值											*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pStartTime：要计算的开始时间							*******
 *******			  char *pEndTime：要计算的结束时间								*******
 *******              int *pSecond:(输出)相差的秒钟值								*******
 *******              int *pMilliSecond:(输出)相差的毫秒值							*******
 *******              int pFlag：时间格式标志，参数常量如下：						*******
 *******                 0：'HHMMSS'												*******
 *******                 1: 'HH:MM:SS'												*******
 *******                 2：'HHMMSSMMM'												*******
 *******                 3：'HH:MM:SS:MMM'											*******
 *******	返 回 值：成功返回：=0；失败返回<0 ,失败返回值描述						*******
 *******                            -1：时间格式参数非法							*******
 *******                            -2：时间参数非法								*******
 *******                            -3：开始时间大于结束时间						*******
 ******************************************************************************************/
int ftDiffTime(char *pStartTime, char *pEndTime, int *pSecond,
		int *pMilliSecond, int pFlag) {
	int nStartHour;
	int nEndHour;
	int nStartMinute;
	int nEndMinute;
	int nStartSecond;
	int nEndSecond;
	int nStartMilliSecond;
	int nEndMilliSecond;
	int n = 0;
	char szTmp[4];

	int ns = 0;
	int ne = 0;

	ns = (int) strlen(pStartTime);
	ne = (int) strlen(pEndTime);

	nStartHour = 0;
	nEndHour = 0;
	nStartMinute = 0;
	nEndMinute = 0;
	nStartSecond = 0;
	nEndSecond = 0;
	nStartMilliSecond = 0;
	nEndMilliSecond = 0;

	switch (pFlag) {
	case 0:
		if ((strlen(pStartTime) != 6) || (strlen(pEndTime) != 6)) {
			return -2;
		}

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pStartTime, 2);
		nStartHour = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pStartTime + 2, 2);
		nStartMinute = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pStartTime + 4, 2);
		nStartSecond = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pEndTime, 2);
		nEndHour = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pEndTime + 2, 2);
		nEndMinute = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pEndTime + 4, 2);
		nEndSecond = atoi(szTmp);

		break;
	case 1:
		if ((strlen(pStartTime) != 8) || (strlen(pEndTime) != 8)) {
			return -2;
		}

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pStartTime, 2);
		nStartHour = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pStartTime + 3, 2);
		nStartMinute = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pStartTime + 6, 2);
		nStartSecond = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pEndTime, 2);
		nEndHour = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pEndTime + 3, 2);
		nEndMinute = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pEndTime + 6, 2);
		nEndSecond = atoi(szTmp);

		break;
	case 2:
		if ((strlen(pStartTime) != 9) || (strlen(pEndTime) != 9)) {
			return -2;
		}

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pStartTime, 2);
		nStartHour = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pStartTime + 2, 2);
		nStartMinute = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pStartTime + 4, 2);
		nStartSecond = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pStartTime + 6, 3);
		nStartMilliSecond = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pEndTime, 2);
		nEndHour = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pEndTime + 2, 2);
		nEndMinute = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pEndTime + 4, 2);
		nEndSecond = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pEndTime + 6, 3);
		nEndMilliSecond = atoi(szTmp);

		break;
	case 3:
		if ((strlen(pStartTime) != 12) || (strlen(pEndTime) != 12)) {
			return -2;
		}

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pStartTime, 2);
		nStartHour = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pStartTime + 3, 2);
		nStartMinute = atoi(szTmp);

		// 删除的代码部分
		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pStartTime + 6, 2);
		nStartSecond = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pStartTime + 9, 3);
		nStartMilliSecond = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pEndTime, 2);
		nEndHour = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pEndTime + 3, 2);
		nEndMinute = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pEndTime + 6, 2);
		nEndSecond = atoi(szTmp);

		memset(szTmp, 0x00, sizeof(szTmp));
		memcpy(szTmp, pEndTime + 9, 3);
		nEndMilliSecond = atoi(szTmp);

		break;
	default:
		return -1;
	}

	nStartSecond = nStartHour * 60 * 60 + nStartMinute * 60 + nStartSecond;
	nEndSecond = nEndHour * 60 * 60 + nEndMinute * 60 + nEndSecond;

	if (nEndSecond < nStartSecond) {
		return -3;
	}

	nStartSecond = nEndSecond - nStartSecond;

	if ((pFlag == 2) || (pFlag == 3)) {
		nStartMilliSecond = nEndMilliSecond - nStartMilliSecond;
		if (nStartMilliSecond < 0) {
			nStartMilliSecond += 1000;
			nStartSecond -= 1;
		}
	}

	*pSecond = nStartSecond;
	*pMilliSecond = nStartMilliSecond;

	return 0;
}

//-----------------------------------------------------------------------------------------
// V2.0 -- PBOC2.0 函数库
//-----------------------------------------------------------------------------------------

/******************************************************************************************
 *******	函数名称：ftCalcExtAuth2  -序号：4.1-									*******
 *******	函数功能：根据终端随机数、初始化安全通道应答数据，生成外部认证指令		*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pKey       :ASCII码的计算密钥16字节32个字符				*******
 *******              char *pTerRandom :ASCII码的终端随机数8字节16个字符			*******
 *******			  char *pData      :初始化安全通道应答数据，当前使用SCP02协议 	*******
 *******                                10字节密钥信息数据 +						*******
 *******                                2 字节密钥信息 +							*******
 *******                                2 字节序列计数器 +							*******
 *******                                6 字节卡片随机数 +							*******
 *******                                8 字节卡片密文								*******
 *******			  char *pOut      : ASCII码的终端密文(8字节)+C-MAC		 		*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；													*******
 ******************************************************************************************/
int ftCalcExtAuth2(char *pKey, char *pTermRandom, char *pData, char *pOut) {
	string strMsg;

	char szRand[16 + 1];
	char szCalData[56 + 1];
	char szKeyInfoData[10 + 1];
	char szKeyInfo[5];
	char szSeqCounter[5];
	char szCardCrypt[17];
	char szCardRand[13];
	char szBuf[256];

	char szHexData[128];
	char szHexRand[8 + 1];
	char szHexData1[9];
	char szHexData2[9];
	char szHexData3[9];
	char szHexICV[9];
	char szHexVal[9];

	char szKey[33];
	char szHexKey[17];

	// ENC会话密钥
	char szSessEncKey[16 + 1];

	// MAC会话密钥
	char szSessMacKey[16 + 1];

	memset(szSessEncKey, 0x00, sizeof(szSessEncKey));
	memset(szSessMacKey, 0x00, sizeof(szSessMacKey));
	memset(szRand, 0x00, sizeof(szRand));
	memset(szCalData, 0x00, sizeof(szCalData));
	memset(szKeyInfoData, 0x00, sizeof(szKeyInfoData));
	memset(szKeyInfo, 0x00, sizeof(szKeyInfo));
	memset(szSeqCounter, 0x00, sizeof(szSeqCounter));
	memset(szCardCrypt, 0x00, sizeof(szCardCrypt));
	memset(szCardRand, 0x00, sizeof(szCardRand));
	memset(szBuf, 0x00, sizeof(szBuf));
	memset(szKey, 0x00, sizeof(szKey));

	memset(szHexData, 0x00, sizeof(szHexData));
	memset(szHexData1, 0x00, sizeof(szHexData1));
	memset(szHexData2, 0x00, sizeof(szHexData2));
	memset(szHexData3, 0x00, sizeof(szHexData3));
	memset(szHexKey, 0x00, sizeof(szHexKey));

	//1.检查传入密钥长度合法性并转换成BCD码，ASCII码为：32个字符
	if (strlen(pKey) != 32) {
		return -1;
	}

	memcpy(szKey, pKey, 32);

	ftAtoh(szKey, szHexKey, 16);

	//2.检查传入终端随机数据的合法性并转换成BCD码，ASCII码为：16个字符
	if (strlen(pTermRandom) != 16) {
		return -2;
	}

	// 终端随机数
	memcpy(szRand, pTermRandom, 16);

	ftAtoh(szRand, szHexRand, 8);

	//3.检查传入初始化安全通道指令应答数据合法性并转换成BCD码，ASCII码长度为：56
	if (strlen(pData) != 56) {
		return -3;
	}

	// szCalData,
	ftAtoh(pData, szCalData, 28);

	// 拆分应答数据
	// 1.10字节的密钥信息数据；
	memcpy(szKeyInfoData, szCalData, 10);

	// 2.2字节的密钥信息
	memcpy(szKeyInfo, szCalData + 10, 2);

	// 3.2字节的序列计数器
	memcpy(szSeqCounter, szCalData + 24, 4);

	// 4.6字节卡片随机数
	memcpy(szCardRand, szCalData + 28, 12);

	// 5.8字节的卡片密文
	memcpy(szCardCrypt, szCalData + 40, 16);

	//
	ftAtoh(szKey, szHexKey, 16);

	memset(szBuf, 0x00, sizeof(szBuf));
	//1.生成S-ENC-KEY
	// 组织生成S-ENC-KEY数据, 2字节常量值为：0182 +
	memcpy(szBuf, "0182", 4);

	// + 2字节 2byte sequence conter(序列计数器:8050指令返回)
	memcpy(szBuf + 4, pData + 24, 4);

	// + 12字节 00
	memcpy(szBuf + 8, "000000000000000000000000", 24);

	// 转换成BCD码
	ftAtoh(szBuf, szHexData, 16);

	// 按8字节拆分成Data1,和Data2
	memcpy(szHexData1, szHexData, 8);
	memcpy(szHexData2, szHexData + 8, 8);

	memset(szHexICV, 0x00, sizeof(szHexICV));
	memset(szHexVal, 0x00, sizeof(szHexVal));

	// 用初始化的ICV同第个数据进行异或
	ftCalXOR(szHexICV, szHexData1, 8, szHexVal);

	memset(szHexICV, 0x00, sizeof(szHexICV));
	// 使用传入的密钥对结果进行3DES加密
	ft3DesEncL(szHexKey, 16, szHexVal, szHexICV, 8, 0);

	// 加密会话密钥的高8字节；
	memcpy(szSessEncKey, szHexICV, 8);

	//用结果作为ICV 对数据第2块进行异或计算
	memset(szHexVal, 0x00, sizeof(szHexVal));
	ftCalXOR(szHexICV, szHexData2, 8, szHexVal);

	ft3DesEncL(szHexKey, 16, szHexVal, szHexICV, 8, 0);

	// 添加会话密钥的加密密钥
	memcpy(szSessEncKey + 8, szHexICV, 8);

	memset(szBuf, 0x00, sizeof(szBuf));
	//2.生成C-MAC-KEY
	// 组织生成S-ENC-KEY数据, 2字节常量值为：0101 +
	memcpy(szBuf, "0101", 4);

	memcpy(szBuf + 4, pData + 24, 4);

	// + 12字节 00
	memcpy(szBuf + 8, "000000000000000000000000", 24);

	// 转换成BCD码
	ftAtoh(szBuf, szHexData, 16);

	// 按8字节拆分成Data1,和Data2
	memcpy(szHexData1, szHexData, 8);
	memcpy(szHexData2, szHexData + 8, 8);

	memset(szHexICV, 0x00, sizeof(szHexICV));
	memset(szHexVal, 0x00, sizeof(szHexVal));

	// 用初始化的ICV同第个数据进行异或
	ftCalXOR(szHexICV, szHexData1, 8, szHexVal);

	memset(szHexICV, 0x00, sizeof(szHexICV));
	// 使用传入的密钥对结果进行3DES加密
	ft3DesEncL(szHexKey, 16, szHexVal, szHexICV, 8, 0);

	// MAC会话密钥的高8字节；
	memcpy(szSessMacKey, szHexICV, 8);

	//用结果作为ICV 对数据第2块进行异或计算
	memset(szHexVal, 0x00, sizeof(szHexVal));
	ftCalXOR(szHexICV, szHexData2, 8, szHexVal);

	ft3DesEncL(szHexKey, 16, szHexVal, szHexICV, 8, 0);

	// 添加会话密钥的加密密钥
	memcpy(szSessMacKey + 8, szHexICV, 8);

	//3.计算外部认证命令的终端密文
	memset(szBuf, 0x00, sizeof(szBuf));

	// 计算数据,Sequence counter(序列计数器)2字节 + Card challenge(卡片随机数)6字节 + host challenge(终端随机数)8字节 +补 80000000 00000000
	// Sequence counter(序列计数器)2字节
	memcpy(szBuf, pData + 24, 4);

	//  Card challenge(卡片随机数)6字节
	memcpy(szBuf + 4, pData + 28, 12);

	//  host challenge(终端随机数)8字节
	memcpy(szBuf + 16, pTermRandom, 16);

	// 补 80000000 00000000
	memcpy(szBuf + 32, "8000000000000000", 16);

	// 测试用用于显示
	memset(szHexData, 0x00, sizeof(szHexData));
	ftAtoh(szBuf, szHexData, 24);

	// 拆分成三个8字节值
	memcpy(szHexData1, szHexData, 8);
	memcpy(szHexData2, szHexData + 8, 8);
	memcpy(szHexData3, szHexData + 16, 8);

	memset(szHexICV, 0x00, sizeof(szHexICV));
	memset(szHexVal, 0x00, sizeof(szHexVal));

	// 用初始化ICV[8字节0x00] 对第1个数据进行异或
	ftCalXOR(szHexICV, szHexData1, 8, szHexVal);

	memset(szHexICV, 0x00, sizeof(szHexICV));

	// 对第1个异或结果进行3DES加密
	ft3DesEncL(szSessEncKey, 16, szHexVal, szHexICV, 8, 0);

	memset(szHexVal, 0x00, sizeof(szHexVal));
	// 用3DES计算结果对第2个数据块进行异或计算
	ftCalXOR(szHexICV, szHexData2, 8, szHexVal);

	memset(szHexICV, 0x00, sizeof(szHexICV));
	// 对第2个异或结果进行3DES加密
	ft3DesEncL(szSessEncKey, 16, szHexVal, szHexICV, 8, 0);

	memset(szHexVal, 0x00, sizeof(szHexVal));
	// 用3DES计算结果对第2个数据块进行异或计算
	ftCalXOR(szHexICV, szHexData3, 8, szHexVal);

	memset(szHexICV, 0x00, sizeof(szHexICV));
	// 对第3个异或结果进行3DES加密
	ft3DesEncL(szSessEncKey, 16, szHexVal, szHexICV, 8, 0);

	//第个计算结果即为指令终端密文

	memset(szRand, 0x00, sizeof(szRand));

	ftHtoa(szHexICV, szRand, 8);

	// 将生成的指令密文保存输出变量
	memcpy(pOut, szRand, 16);

	// 计算外部认证命令的C-MAC值
	//8482030010	 + 8字节第5节中计算生成的终端密文；后补800000 到16字节；

	//4.计算外部认证命令的C-MAC

	//计算数据：8482030010	 + 8字节第5节中计算生成的终端密文；后补800000 到16字节；
	memset(szBuf, 0x00, sizeof(szBuf));

	// 指令头部分,5字节
	memcpy(szBuf, "8482000010", 10);

	// 第3部分计算输出数据,8主机
	memcpy(szBuf + 10, szRand, 16);

	// 补补3字节的800000 到长度为8的倍数
	memcpy(szBuf + 26, "800000", 6);

	memset(szHexData, 0x00, sizeof(szHexData));
	ftAtoh(szBuf, szHexData, 16);

	memset(szHexData1, 0x00, sizeof(szHexData1));
	memset(szHexData2, 0x00, sizeof(szHexData2));

	// 按8字节拆分
	memcpy(szHexData1, szHexData, 8);
	memcpy(szHexData2, szHexData + 8, 8);

	memset(szHexICV, 0x00, sizeof(szHexICV));
	memset(szHexVal, 0x00, sizeof(szHexVal));

	//
	ftCalXOR(szHexICV, szHexData1, 8, szHexVal);

	// 使用MAC会话密钥的前半部分进行DES加密,缺省使用：BCD码
	ftDesEncL(szSessMacKey, 8, szHexVal, szHexICV, 8, 0);

	// 使用加密结果对第2个块进行异或
	ftCalXOR(szHexICV, szHexData2, 8, szHexVal);

	// 使用MAC会话密钥进行3DES加密
	ft3DesEncL(szSessMacKey, 16, szHexVal, szHexICV, 8, 0);

	memset(szBuf, 0x00, sizeof(szBuf));
	ftHtoa(szHexICV, szBuf, 8);

	memcpy(pOut + 16, szBuf, 16);

	memcpy(pOut + 32, "\x00", 1);

	return 0;
}

/******************************************************************************************
 *******	函数名称：ftCalcPutKey		-序号：4.2-									*******
 *******	函数功能：计算GP的PUT KEY指令的密钥密文和CHECK值						*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pKey       :ASCII码的密钥(KEY)16字节32个字符			*******
 *******              char *pInitData  :ASCII码的初始化计算数据						*******
 *******                                通常为PUT KEY 指令的P1,P2值(0181)			*******
 *******			  char *pOut      : ASCII码的密钥密文(16字节)+Check		 		*******
 *******                                check值为：4字节，第1字节为：0x03后三字节	*******
 ******                                 为用密钥明文对8字节的0x00加密取前3字节值    *******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；													*******
 ******************************************************************************************/
int ftCalcPutKey(char *pKey, char *pInitData, char *pOut) {
	char szHexKey[17];
	char szHexData1[8 + 1];
	char szHexData2[8 + 1];
	char szOut[40 + 1];
	char szSessKey[16 + 1];
	char szOutData[40 + 1];

	int nLen;
	int nRet;

	// 检查传入的密钥
	nLen = (int) strlen(pKey);
	if (nLen != 32) {	// 传入密钥错误
		return -1;
	}

	// 将ASCII码的密钥转换成BCD码
	ftAtoh(pKey, szHexKey, 16);

	// 处理过程密钥计算数据
	memset(szHexData1, 0x00, sizeof(szHexData1));
	memset(szHexData2, 0x00, sizeof(szHexData2));
	memset(szOutData, 0x00, sizeof(szOutData));

	nLen = (int) strlen(pInitData);
	if (nLen != 4) {
		return -2;
	}

	// 将2字节的ASCII码初始化数据转换成BCD码
	ftAtoh(pInitData, szHexData1, 2);

	// 应初始化数据8字节的0x00,对第1个数据块(8字节) 进行异或 对异或结果用传入的KEY进行3DES加密,生成过程密钥的高8字节

	// 使用传入密钥计算会话密钥的前8字节
	nRet = ft3DesEncL(szHexKey, 16, szHexData1, szOut, 8, 0);
	if (nRet != NO_ERROR) {
		return -3;
	}

	// 保存过程密钥的高8字节
	memcpy(szSessKey, szOut, 8);

	// 用上一步中加密输出数据对8字节的0x00 进行异或运算 ，对异或结果用传入的密钥进行加密生成过程密钥的后8字节
	memcpy(szHexData2, szOut, 8);
	nRet = ft3DesEncL(szHexKey, 16, szHexData2, szOut, 8, 0);
	if (nRet != NO_ERROR) {
		return -4;
	}

	// 保存过程密钥的低8字节
	memcpy(szSessKey + 8, szOut, 8);

	// 使用过程密钥对传入的密钥进行3DES加密，生成密钥的密文
	nRet = ft3DesEncL(szSessKey, 16, szHexKey, szOut, 16, 0);
	if (nRet != NO_ERROR) {
		return -5;
	}

	// 将密钥密文BCD码转换成ASCII码
	ftHtoa(szOut, szOutData, 16);

	memset(szHexData1, 0x00, sizeof(szHexData1));

	// 计算密钥的Check值
	nRet = ft3DesEncL(szHexKey, 16, szHexData1, szOut, 8, 0);
	if (nRet != NO_ERROR) {
		return -6;
	}

	memset(szHexData1, 0x00, sizeof(szHexData1));
	// 将16进制的check值转换成ASCII码值
	ftHtoa(szOut, szHexData1, 3);

	// 输出变量的Check值长度为：03 固定值
	memcpy(szOutData + 32, "03", 2);

	// 取密钥Check值的前3字节
	memcpy(szOutData + 34, szHexData1, 6);

	memcpy(szOutData + 40, "\x00", 1);

	memcpy(pOut, szOutData, 40);
	memcpy(pOut + 40, "\x00", 1);

	return NO_ERROR;
}

static int gnRandSeq = 0;
/******************************************************************************************
 *******	函数名称：ftGetRand	  -序号：46-										*******
 *******	函数功能：获取4字节或8字节的终端随机数									*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：int  pLen :指定生成的随机数长度，值设置如下：					*******
 *******                         4：生成4字节的终端随机数							*******
 *******                         8：或其它值，生成8字节的终端随机数					*******
 *******			  char *pRand:(输出)生成的ASCII码的终端随机数					*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；													*******
 ******************************************************************************************/
int ftGetRand(int pLen, char *pRand) {
	string strMsg;
	char charTemp[4];

	unsigned int nVal;

	int nRand1;
	int nRand2;
	int nRand3;
	int nRand4;
	int nRand5;
	int nRand6;
	int nRand7;
	int nRand8;

	//	SYSTEMTIME st;
	struct tm *local_time;
	struct timeval local_seconds;
	time_t st = time(NULL);

	// 获取当前系统时间
	local_time = localtime(&st);
	gettimeofday(&local_seconds, NULL);

	// 取当前的分钟值
	nVal = local_seconds.tv_usec + local_time->tm_sec * 1000
			+ local_time->tm_min * 60 * 1000;

	// 获取当前时间
	nVal = (unsigned) time(0);

	nVal += gnRandSeq;

	srand((unsigned) nVal + 100);

	// 获取第1个随机数；
	nRand1 = rand();

	srand((unsigned) nVal + 200);
	// 获取低2个随机数
	nRand2 = rand();

	srand((unsigned) nVal + 300);
	// 获取低3个随机数
	nRand3 = rand();

	srand((unsigned) nVal + 400);
	// 获取低4个随机数
	nRand4 = rand();

	srand((unsigned) nVal + 500);
	// 获取低5个随机数
	nRand5 = rand();

	srand((unsigned) nVal + 600);
	// 获取低6个随机数
	nRand6 = rand();

	srand((unsigned) nVal + 700);
	// 获取低7个随机数
	nRand7 = rand();

	srand((unsigned) nVal + 800);
	// 获取低8个随机数
	nRand8 = rand();

	gnRandSeq += 10;

	printf("Rand:%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X",
			nRand1 % 256, nRand2 % 256, nRand3 % 256, nRand4 % 256,
			nRand5 % 256, nRand6 % 256, nRand7 % 256, nRand8 % 256);
	sprintf(charTemp, "%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X",
			nRand1 % 256, nRand2 % 256, nRand3 % 256, nRand4 % 256,
			nRand5 % 256, nRand6 % 256, nRand7 % 256, nRand8 % 256);
	strMsg = charTemp;

	// 指定长度为8字节；
	if (pLen == 8) {
		memcpy(pRand, &strMsg, 16);
		memcpy(pRand + 16, "\x00", 1);
	} else // 其它值为4字节；
	{
		memcpy(pRand, &strMsg, 8);
		memcpy(pRand + 8, "\x00", 1);
	}

	return 0;

}

/******************************************************************************************
 *******	函数名称：ftTLVGetVal  -序号：47-										*******
 *******	函数功能：根据TAG码获取数据中此TAG的值的长度及值						*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pTLV：要获取TAG值的TLV缓冲区(ASCII码字符串)				*******
 *******              char *pTAG：要获取长度及值的TAG码								*******
 *******              int  *pLen：(输出)TAG值长度									*******
 *******			  char *pVal：(输出)TAG值										*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；	失败返回错误代码，如下：						*******
 *******                            -1：传入的TLV字符串非法							*******
 *******                            -2：TLV中无此TAG								*******
 *******                            -3：值超出TLV缓冲区长度							*******
 ******************************************************************************************/
int ftTLVGetVal(char *pTLV, char *pTAG, int *pLen, char *pVal) {
	string strMsg;
	string strTLV;
	char *strTLVtemp;

	char szTag[5];
	char szLen[5];
	char szVal[1024 + 1];
	char szTLV[1024 + 1];

	int nPosition;
	int nTagNameLen;
	int nTLVLen;
	int nLen;

	//strTLV.Format("702E57136228000100001117D3012201012345123999919F1F1630313032303330343035303630373038303930413042");
	// 保存传入的TLV(ASCII码字符串)
	sprintf(strTLVtemp, "%s", pTLV);
	strTLV = strTLVtemp;

	// 保存传入的TAG
	nTagNameLen = (int) strlen(pTAG);
	memset(szTag, 0x00, sizeof(szTag));
	memcpy(szTag, pTAG, nTagNameLen);

	memset(szTLV, 0x00, sizeof(szTLV));
	nTLVLen = (int) strTLV.length();
	if (nTLVLen > FTTLVMAXLEN) {
		return -1;
	}

	memcpy(szTLV, strTLV.c_str(), nTLVLen);

	// 查询TLV缓冲区中的TAG的位置
	nPosition = (int) strTLV.find(szTag);
	if (nPosition == -1) {
		// TLV中午此TAG
		return -2;
	}

	// 设置L的位置
	nPosition += nTagNameLen;

	memset(szLen, 0x00, sizeof(szLen));

	memcpy(szLen, szTLV + nPosition, 2);

	nPosition += 2;

	// 检查长度是否为多字节，如为多字节则第1字节为:81,(只处理2字节长度)
	if (memcmp(szLen, "81", 2) == 0) {
		memcpy(szLen, szTLV + nPosition, 2);
		nPosition += 2;
	}

	// 将BCD码字符转换成整型长度
	nLen = (int) ftHexToLong(szLen);

	// 将BCD码长度转换成ASCII码长度
	nLen = nLen * 2;

	// 检查值是否超长度
	if (nPosition + nLen > nTLVLen) {
		return -3;
	}

	// 取TLV的V
	memset(szVal, 0x00, sizeof(szVal));
	memcpy(szVal, szTLV + nPosition, nLen);

	// 设置输出变量值
	*pLen = nLen;

	// 保存TLV中的V到输出变量
	memcpy(pVal, szVal, nLen);

	// 增加字符串的结束字符
	memcpy(pVal + nLen, "\x00", 1);

	return 0;
}

//static CMapStringToOb oMapDict;
static int gDictInitFlag = 0;			// 数据字典初始化标志

/******************************************************************************************
 *******	函数名称：ftTLVInitDict  -序号：48-										*******
 *******	函数功能：加载TAG码定义配置文件dict.cfg									*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：无															*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：0；	失败返回错误代码，如下：						*******
 *******                            -1：传入的TLV字符串非法							*******
 *******                            -2：TLV中无此TAG								*******
 *******                            -3：值超出TLV缓冲区长度							*******
 ******************************************************************************************/
//int ftTLVInitDict()
//{
//    
//	char    szPath[512];
//	char    szFile[512];
//	char    szData[256];
//	char    szItem[6][81];
//	char    szTagCode[4+1];
//    
//	int     nLen;
//	int     nRet;
//	int     nRowNum;
//    
//	FILE    *hFile;
//    
//	memset(szPath,0x00,sizeof(szPath));
//	memset(szFile,0x00,sizeof(szFile));
//    
//	// 获取运行程序当前目录
//	getcwd(szPath, 512);
//    
//	// 路径长度
//	nLen = (int)strlen(szPath);
//    
//	memcpy(szFile,szPath,nLen);
//    
//	memcpy(szFile+nLen,"\\",1);
//	nLen +=1;
//	memcpy(szFile+nLen,"dict.cfg",8);
//	nLen +=8;
//    
//	// 检查文件是否存在,目录存在返回：0；不存在返回：-1
//	nRet = access(szFile,00);
//	if( nRet != 0)
//	{
//		return  FTPRNOTDICT;
//	}
//    
//	// 以只读方式,打开TAG码定义数据字典
//	hFile = fopen(szFile,"r");
//	if( hFile == NULL)
//	{
//		return FTPROPENDICTERR;
//	}
//    
//	nRowNum = 0;
//    
//	// 删除TAG字典对象的所有对象
//	oMapDict.RemoveAll();
//    
//    
//	// 循环读取文件
//	while(1)
//	{
//		// 检查文件是否已读取结束,返回非零：表示指针在文件的结束位置；返回：0，表示不在结束位置
//		nRet = feof(hFile);
//        
//		// 读文件结束则退出循环
//		if(nRet != 0)
//		{
//			break;
//		}
//        
//		memset(szData,0x00,sizeof(szData));
//        
//		// 读取数据行
//		fgets(szData,255,hFile);
//        
//		// 删除对注释行及空行的处理
//		if( szData[0] =='#')
//		{
//			continue;
//		}
//        
//		if(memcmp(szData,"  ",2) == 0)
//		{
//			continue;
//		}
//        
//		STDICT *pDict = new STDICT;
//        
//		memset(pDict,0x00,sizeof(STDICT));
//        
//		//szItem[6][81];
//		memset(&szItem,0x00,6*81);
//        
//		// 将TAG定义行拆分到数组中
//		ftSplitStr(szData,'|',szItem);
//        
//        
//        
//		// 将拆分的TAG码定义信息保存的TAG定义结构
//		// 处理TAG码定义
//		nLen =(int)strlen(szItem[0]);
//		if( !(nLen == 2 || nLen == 4))
//		{
//			fclose(hFile);
//			return FTPRTAGDEFERR;
//		}
//        
//		// 将TAG码保存到TAG码定义结构指针
//		memcpy(pDict->szTagCode,szItem[0],nLen);
//        
//		memset(szTagCode,0x00,sizeof(szTagCode));
//		memcpy(szTagCode,szItem[0],nLen);
//        
//		// 处理TAG码描述信息
//		nLen =(int)strlen(szItem[1]);
//        
//		// 检查描述信息是否超过最大长度
//		if( nLen >50)
//			nLen = 50;
//        
//		memcpy(pDict->szTagName,szItem[1],nLen);
//        
//        
//		// 处理TAG类型,可用类型为：b-二进制; n-数字型;	cn-压缩数字型;	an-字母数字;	ans - 特殊字母数字
//		// 检查TAG类型数据长度是否合法
//		nLen = (int)strlen(szItem[2]);
//		if( nLen > 3)
//		{
//			fclose(hFile);;
//			return FTPRTAGTYPEERR;
//		}
//        
//		if( !( memcmp(szItem[2],"b",1) ==0 || memcmp(szItem[2],"n",1) ==0
//              || memcmp(szItem[2],"cn",2) ==0 || memcmp(szItem[2],"an",2) ==0 || memcmp(szItem[2],"ans",3) ==0))
//		{
//			fclose(hFile);;
//			return FTPRTAGTYPEERR;
//		}
//        
//		memcpy(pDict->szDataType,szItem[2],nLen);
//        
//		// 处理值长度类型
//		nLen = (int)strlen(szItem[3]);
//		if( nLen != 1)
//		{
//			fclose(hFile);;
//			return FTPRTAGVALLENTYPEERR;
//		}
//        
//		// 长度类型值只有：0-定义；1-变长两个值
//		if( !(szItem[3][0] =='0' || szItem[3][0] =='1'))
//		{
//			fclose(hFile);;
//			return FTPRTAGVALLENTYPEERR;
//		}
//        
//		memcpy(pDict->szLenType,szItem[3],1);
//        
//		// 处理长度 ，长度合法数据为：1~256 或var 值
//		nLen = (int)strlen(szItem[4]);
//		if( nLen<0 || nLen >3)
//		{
//			fclose(hFile);;
//			return FTPRTAGVALLENERR;
//		}
//        
//		// 检查值合法性
//		if( memcmp(szItem[4],"var",3)!= 0)
//		{
//			nRet = atoi(szItem[4]);
//			if( nRet <0 || nRet > 256)
//			{
//				fclose(hFile);;
//				return FTPRTAGVALLENERR;
//			}
//		}
//        
//		// 保存值长度定义信息
//		memcpy(pDict->szValLen,szItem[4],nLen);
//        
//		// 处理模板标志
//		nLen = (int)strlen(szItem[5]);
//        
//		// 检查长度合法性
//		if(nLen != 1)
//		{
//			fclose(hFile);;
//			return FTPRTAGTEMPFLAGERR;
//		}
//        
//		// 检查值合法性
//		if( !( szItem[5][0] =='0' || szItem[5][0]=='1'))
//		{
//            
//			break;
//		}
//        
//		// 保存模板标志
//		memcpy(pDict->szTempFlag,szItem[5],1);
//		// 按TAG码保存TAG码定义信息
//		oMapDict.SetAt(szTagCode,(CObject *)pDict);
//		nRowNum ++;
//	}
//    
//	// 关闭打开的文件
//	fclose(hFile);
//    
//	// 检查TAG码定义配置文件中定义的TAG码个数
//	if( nRowNum <= 0)
//	{
//		return FTPRNOTTAGDEF;
//	}
//    
//	// 设置TAG码数据字典初始化标志值为已初始化
//	gDictInitFlag = 1;
//    
//	return NO_ERROR;
//}

/******************************************************************************************
 *******	函数名称：ftTLVFreeDict  -序号：49-										*******
 *******	函数功能：释放加载的TAG码数据字典										*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：无															*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：NO_ERROR											*******
 ******************************************************************************************/
//int ftTLVFreeDict()
//{
//	
//	oMapDict.RemoveAll();
//    
//	gDictInitFlag = 0;
//    
//	return NO_ERROR;
//}

/******************************************************************************************
 *******	函数名称：ftTLVGetTAGNum  -序号：50-									*******
 *******	函数功能：取TAG码字典中的TAG码个数										*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：int *pNum:(输出)TAG码定义数据字典中TAG个数					*******
 *******----------------------------------------------------------------------------*******
 ******************************************************************************************/
//int ftTLVGetTAGNum(int *pNum)
//{
//	int nElNum;
//    
//	// 检查是否初始化TAG码定义数据字典
//	if( gDictInitFlag == 0)
//	{
//		return FTPRNOTINITDICT;
//	}
//    
//	// 检查字典中是否有TAG码定义数据对象
//	nElNum = (int)oMapDict.GetCount();
//	if( nElNum <= 0)
//	{
//		return FTPRNOTDICTEL;
//	}
//    
//	*pNum = nElNum;
//    
//	return NO_ERROR;
//}

/******************************************************************************************
 *******	函数名称：ftTLVGetTAGNames  -序号：51-									*******
 *******	函数功能：取TAG码字典中的全部TAG码										*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：int *pNum:		(输出)字典中TAG码个数						*******
 *******              char pNames[][5]:	(输出)字典中TAG码列表						*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：NO_ERROR，失败返回错误代码							*******
 ******************************************************************************************/
//int ftTLVGetTAGNames(int *pNum,char pNames[][5])
//{
//	int nNum;
//	
//	STDICT    *pTagEl;
//	string    strKey;
//	POSITION   pPos;
//	int        nLen;
//    
//    
//	// 检查是否初始化TAG码定义数据字典
//	if( gDictInitFlag == 0)
//	{
//		return FTPRNOTINITDICT;
//	}
//    
//	// 获取字典的开始位置
//	pPos = oMapDict.GetStartPosition();
//	if( pPos == NULL )
//	{
//		return FTPRGETSTARTPOSERR;
//	}
//    
//	// 设置读取的TAG码定义对象读取个数
//	nNum = 0;
//    
//	// 循环读取字典中的对象
//	while( pPos != NULL)
//	{
//        
//		//m_mapStr.GetNextAssoc( pos, strKey,(CObject*&)myData );
//		oMapDict.GetNextAssoc(pPos,strKey,(CObject *&)pTagEl);
//        
//		nLen = strKey.GetLength();
//		if( nLen>4)
//			nLen = 4;
//        
//		memcpy(pNames[nNum],strKey.GetBuffer(0),nLen);
//        
//		nNum ++;
//	}
//    
//    
//	// 设置函数的输出变量
//	*pNum = nNum;
//    
//	return NO_ERROR;
//	
//    
//}
/******************************************************************************************
 *******	函数名称：ftTLVGetTAGDef  -序号：52-									*******
 *******	函数功能：取TAG码字典中指定TAG码的定义信息								*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：无															*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：字典中TAG码个数，失败返回错误代码					*******
 ******************************************************************************************/
//int ftTLVGetTAGDef(char *pTagName,STDICT *pElDef)
//{
//	STDICT    *pTagEl;
//	string    strKey;
//	int        nRet;
//    
//    
//	// 检查是否初始化TAG码定义数据字典
//	if( gDictInitFlag == 0)
//	{
//		return FTPRNOTINITDICT;
//	}
//    
//	sprintf(strKey.c_str(), "%s",pTagName);
//    
//	// 失败返回：0；成功返回：非零；
//	nRet=oMapDict.Lookup(strKey,(CObject *&)pTagEl);
//	if( nRet == 0)
//	{
//		return FTPRGETELERR;
//	}
//    
//	memcpy(pElDef,pTagEl,sizeof(STDICT));
//	
//	return NO_ERROR;
//}

/******************************************************************************************
 *******	函数名称：ftTLVSplitTAG		 -序号：53-									*******
 *******	函数功能：拆分TAG值字符串中的TAG码和值(TAG和值以=号分隔)				*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pBuf	:	要进行拆分的字符缓冲区						*******
 *******              char *pTagCode:	(输出)TAG码									*******
 *******              char *pTagVal	:	(输出)TAG值									*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：NO_ERROR，		失败返回错误代码					*******
 ******************************************************************************************/
int ftTLVSplitTAG(char *pBuf, char *pTagCode, char *pTagVal) {
	string strBuf;
	int nLen;
	int nPos;

	// 将传入的数据
	strBuf = pBuf;

	// 获取处理字符串的长度
	nLen = (int) strBuf.length();

	// TAG码及值字符最小长度为4个字符
	if (nLen < 4) {
		return FTPROTHERERR;
	}

	// 获取拆分字符的位置并检查TAG码部分数据长度的合法性(4个字符或2个字符)
	nPos = (int) strBuf.find("=");
	if (nPos == -1) {
		return FTPROTHERERR;
	}

	if (!(nPos == 4 || nPos == 2)) {
		return FTPROTHERERR;
	}

	// 取TAG码及值字符串中的TAG码和值
	memcpy(pTagCode, pBuf, nPos);
	memcpy(pTagCode + nPos, "\x00", 1);

	// 计算值长度
	nLen = nLen - (nPos + 1);

	// 取TAG码值
	memcpy(pTagVal, pBuf + (nPos + 1), nLen);
	memcpy(pTagVal + nLen, "\x00", 1);

	return NO_ERROR;

}

// TAG-值 处理相关函数
// 定义一个TAG值列表对象
//static CStringList oTagValList;

// 值列表对象初始化标志
static int gnListInitFlag = 0;

/******************************************************************************************
 *******	函数名称：ftTLVInitTagList	 -序号：54-									*******
 *******	函数功能：初始化TAG值列表对象											*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：无															*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：NO_ERROR，		失败返回错误代码					*******
 ******************************************************************************************/
//int ftTLVInitTagList()
//{
//    
//	oTagValList.RemoveAll();
//    
//	gnListInitFlag = 1;
//	
//	return NO_ERROR;
//}

/******************************************************************************************
 *******	函数名称：ftTLVFreeTagList	 -序号：55-									*******
 *******	函数功能：删除TAG值列表对象中的所有TAG值								*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：无															*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：NO_ERROR，		失败返回错误代码					*******
 ******************************************************************************************/
//int ftTLVFreeTagList()
//{
//    
//	oTagValList.RemoveAll();
//    
//	gnListInitFlag = 0;
//	
//	return NO_ERROR;
//}

/******************************************************************************************
 *******	函数名称：ftTLVPutVal		 -序号：56-									*******
 *******	函数功能：项TAG值列表赋值												*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pTag	:指定TAG码										*******
 *******              char *pTagVal	:指定TAG码的值									*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：NO_ERROR，		失败返回错误代码					*******
 ******************************************************************************************/
//int ftTLVPutVal(char *pTag,char *pTagVal)
//{
//	STDICT  stElDef;
//	int     nRet;
//    
//	string strBuf;
//    
//	//1.检查TAG码定义数据是否已经装载
//	// 检查是否初始化TAG码定义数据字典
//	if( gDictInitFlag == 0)
//	{
//		return FTPRNOTINITDICT;
//	}
//    
//	//2.检查TAG值列表是否初始化化
//	if( gnListInitFlag == 0 )
//	{
//		return FTPRNOTINITTAGLIST;
//	}
//    
//	//3.检查TAG码参数的合法性(ASCII码的长度必须为：4或2)
//	if( !( strlen(pTag) == 4 || strlen(pTag) == 2))
//	{
//		return FTPROTHERERR;
//	}
//    
//	//4.检查TAG码值参数的合法性(ASCII码值长度需小于：504)
//	if( strlen(pTagVal) > 504 )
//	{
//		return FTPROTHERERR;
//	}
//    
//	//5.根据TAG码从TAG码定义字典获取TAG定义信息结构
//	nRet = ftTLVGetTAGDef(pTag,&stElDef);
//	if( nRet != NO_ERROR)
//	{
//		return nRet;
//	}
//    
//	// 数据格式为：TAG=VAL
//	sprintf(strBuf, "%s=%s",pTag,pTagVal);
//    
//	//7.将TAG码及值数据保存到列表对象
////	oTagValList.AddTail(strBuf);
//    
//	return NO_ERROR;
//}

/******************************************************************************************
 *******	函数名称：ftTLVGetValNum	 -序号：57-									*******
 *******	函数功能：获取TAG码值列表对象中存储的值个数								*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：int *pNum:(输出)成功返回TAG码值列表对象中存储的值个数			*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：NO_ERROR，		失败返回错误代码					*******
 ******************************************************************************************/
//int ftTLVGetValNum(int *pNum)
//{
//    
//	int nNum;
//    
//	//1.检查TAG值列表是否初始化化
//	if( gnListInitFlag == 0 )
//	{
//		return FTPRNOTINITTAGLIST;
//	}
//    
//	//2.取TAG值列表保存的值个数
//	nNum = (int)oTagValList.GetCount();
//    
//	if( nNum < 0)
//	{
//		return FTPROTHERERR;
//	}
//    
//	*pNum = nNum;
//    
//	return NO_ERROR;
//}

int getTagLen(const char* tag) {
	//判断是否有后续tag字节

	char buf1[2];
	char buf2[2];

	int buf;

	memset(buf1, 0x00, sizeof(buf1));
	memset(buf2, 0x00, sizeof(buf2));

	memcpy(buf1, tag, 1);
	memcpy(buf2, tag + 1, 1);

	buf = (int) ftHexToLong(buf1);
	//第一位不能被2整除，第二位为F，则符合TLV规定的“11111”实际标签值表示在后续字节中
	if (buf % 2 != 0 && memcmp(buf2, "F", 1) == 0) { //此tag为两字节tag
		return 2;
	} else { //此tag为一字节tag
		return 1;
	}
	return 0;
}

/******************************************************************************************
 *******	函数名称：ftTLVGetStrVal	 -序号：58-									*******
 *******	函数功能：解析TLV字符串获取指定TAG的值									*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pStr:要解析的TLV字符串									*******
 *******			  char *pTag:要获取值的TAG,多个TAG以符号,分格					*******
 *******              char *pVal:(输出)TAG值										*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：NO_ERROR，		失败返回错误代码					*******
 ******************************************************************************************/
int ftTLVGetStrVal(char *pStr, char *pTag, char *pVal) {
	string strBuf;
	char szTags[10][5] = { 0 };
	char szTag[5];
	char szBuf[4096] = { 0 };
	char szVal[1024] = { 0 };
	char szLen1[3] = { 0 };
	char szTagBuf[5];
	int nLayer = 0;
	int nLen;
	int nPos;
	int nValLen = 0;
	int i;
	int nTagLen;
	bool bFlag;

	// 处理传入的TAG列表
	nLen = (int) strlen(pTag);
	strBuf = pTag;

	i = 0;
	nLayer = 0;
	// 拆分TAG列表
	while (true) {
		nPos = (int) strBuf.find(",", i);
		if (nPos == -1) {
			if (nLen > i) {
				memcpy(szTags[nLayer], pTag + i, (nLen - i));
				nLayer += 1;
			}

			break;
		} else {
			memcpy(szTags[nLayer], pTag + i, (nPos - i));
			i = nPos + 1;
			nLayer += 1;
		}
		if (nLayer >= 9) {	// 最大只处理9级
			break;
		}
	}
	if (nLayer <= 0) {
		return -1;
	}

	// 将TLV数据保存 CString对象
	nLen = (int) strlen(pStr);

	memcpy(szBuf, pStr, nLen);
	memcpy(szBuf + nLen, "\x00", 1);

	// 根据TAG级数处理TLV数据
	for (i = 0; i < nLayer; i++) {
		nPos = 0;
		bFlag = true;
		// 设置要获取数据的TAG
		nLen = (int) strlen(szTags[i]);
		memcpy(szTag, szTags[i], nLen);
		memcpy(szTag + nLen, "\x00", 1);

		while (bFlag) {
			memset(szTagBuf, 0x00, sizeof(szTagBuf));
			memcpy(szTagBuf, szBuf + nPos, 2);

			nTagLen = getTagLen(szTagBuf);
			if (nTagLen == 2) {
				memcpy(szTagBuf, szBuf + nPos, 4);
				nPos = nPos + 4;
			} else {
				nPos = nPos + 2;
			}
			if (memcmp(szTagBuf, szTag, nTagLen * 2) == 0) {
				// 取TAG数据长度高字节
				memset(szLen1, 0x00, sizeof(szLen1));
				memcpy(szLen1, szBuf + nPos, 2);
				if (memcmp(szLen1, "81", 2) == 0) {
					nPos += 2;
					memcpy(szLen1, szBuf + nPos, 2);

				}
				// 值长度
				nValLen = (int) ftHexToLong(szLen1);
				// ASCII码长度值
				nValLen = nValLen * 2;
				// 值开始位置
				nPos += 2;
				// 设置值数据
				memset(szVal, 0x00, sizeof(szVal));
				memcpy(szVal, szBuf + nPos, nValLen);

				// 将值保存到szBuf中用于下次查询
				memset(szBuf, 0x00, sizeof(szBuf));
				memcpy(szBuf, szVal, nValLen);
				bFlag = false;

			} else {
				// 取TAG数据长度高字节
				memset(szLen1, 0x00, sizeof(szLen1));
				memcpy(szLen1, szBuf + nPos, 2);
				if (memcmp(szLen1, "81", 2) == 0) {
					nPos += 2;
					memcpy(szLen1, szBuf + nPos, 2);
				}
				// 值长度
				nValLen = (int) ftHexToLong(szLen1);
				// ASCII码长度值
				nValLen = nValLen * 2;
				// 值开始位置
				nPos = nPos + 2 + nValLen;
				//tal寻找完成
				if (nPos > strlen(szBuf)) {
					bFlag = false;
				}

			}
		}
	}

	// 成功设置函数输出变量
	memcpy(pVal, szVal, nValLen);
	memcpy(pVal + nValLen, "\x00", 1);

	return NO_ERROR;
}

//-----------------------------------------------------------------------------------------
// V3.0 -- 相关规范解析函数
//-----------------------------------------------------------------------------------------

// GB/T 2261.1-2003 个人基本信息分类与代码 第1部分：人的性别代码
/******************************************************************************************
 *******	函数名称：ftGetSexCode	  -序号：5.1-									*******
 *******	函数功能：根据性别代码获取性别名称	 (GB 2261-1980 性别及代码)			*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：int pCode    :要获取的性别代码,规范代码如下：					*******
 *******                             0 -- 未知的性别								*******
 *******                             1 -- 男										*******
 *******							 2 -- 女										*******
 *******							 9 -- 未说明的性别                              *******
 *******              char *pName  :(输出)性别代码名称								*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：NO_ERROR；失败返回错误代码							*******
 ******************************************************************************************/
int ftGetSexCode(int pCode, char *pName) {
	memcpy(pName, "\x00", 1);

	switch (pCode) {
	case 0:
		memcpy(pName, "未知的性别", 11);
		break;
	case 1:
		memcpy(pName, "男", 3);
		break;
	case 2:
		memcpy(pName, "女", 3);
		break;
	case 9:
		memcpy(pName, "未说明的性别", 13);
		break;
	default:
		return -1;
	}
	return NO_ERROR;
}

// 函数二
int ftGetSexCode(char *pCode, char *pName) {
	int nCode;
	int nRet;

	// 将字符串的性别代码转换成功整型
	nCode = (int) atoi(pCode);
	nRet = ftGetSexCode(nCode, pName);
	return nRet;
}

// GB/T 2261.2-2003 个人基本信息分类与代码 第2部分：婚姻状况代码
/******************************************************************************************
 *******	函数名称：ftGetMarrCode	  -序号：5.2-									*******
 *******	函数功能：根据婚姻状态代码获取状态名称									*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：int pCode    :要获取的婚姻状态代码,规范代码如下：				*******
 *******                             1 -- 未婚										*******
 *******                             2 -- 已婚										*******
 *******							 3 -- 丧偶										*******
 *******							 4 -- 离异										*******
 *******							 9 -- 其他			                             ******* 
 *******              char *pName  :(输出)婚姻状态代码名称							*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：NO_ERROR；失败返回错误代码							*******
 ******************************************************************************************/
int ftGetMarrCode(int pCode, char *pName) {
	memcpy(pName, "\x00", 1);

	switch (pCode) {
	case 1:
		memcpy(pName, "未婚", 5);
		break;
	case 2:
		memcpy(pName, "已婚", 5);
		break;
	case 3:
		memcpy(pName, "丧偶", 5);
		break;
	case 4:
		memcpy(pName, "离异", 5);
		break;
	case 9:
		memcpy(pName, "其他", 5);
		break;
	default:
		return -1;
	}

	return NO_ERROR;
}

// 函数二
int ftGetMarrCode(char *pCode, char *pName) {
	int nCode;
	int nRet;

	// 将ASCII码的婚姻状态代码转换成整型
	nCode = (int) atoi(pCode);

	nRet = ftGetMarrCode(nCode, pName);
	return nRet;
}

//GB/T 3304-1991 国各民族名称的罗马字母拼写法和代码
/******************************************************************************************
 *******	函数名称：ftGetNationCode	  -序号：5.3-								*******
 *******	函数功能：根据民族代码获取民族名称										*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：int pCode    :要获取的民族代码,               				*******
 *******              char *pName  :(输出)民族代码名称								*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：NO_ERROR；失败返回错误代码							*******
 ******************************************************************************************/
int ftGetNationCode(int pCode, char *pName) {

	memcpy(pName, "\x00", 1);

	switch (pCode) {
	case 1:
		memcpy(pName, "汉族", 5);
		break;
	case 2:
		memcpy(pName, "蒙古族", 7);
		break;
	case 3:
		memcpy(pName, "回族", 5);
		break;
	case 4:
		memcpy(pName, "藏族", 5);
		break;
	case 5:
		memcpy(pName, "维吾尔族", 9);
		break;
	case 6:
		memcpy(pName, "苗族", 5);
		break;
	case 7:
		memcpy(pName, "彝族", 5);
		break;
	case 8:
		memcpy(pName, "壮族", 5);
		break;
	case 9:
		memcpy(pName, "布依族", 7);
		break;
	case 10:
		memcpy(pName, "朝鲜族", 7);
		break;
	case 11:
		memcpy(pName, "满族", 5);
		break;
	case 12:
		memcpy(pName, "侗族", 5);
		break;
	case 13:
		memcpy(pName, "瑶族", 5);
		break;
	case 14:
		memcpy(pName, "白族", 5);
		break;
	case 15:
		memcpy(pName, "土家族", 7);
		break;
	case 16:
		memcpy(pName, "哈尼族", 7);
		break;
	case 17:
		memcpy(pName, "哈萨克族", 9);
		break;
	case 18:
		memcpy(pName, "傣族", 5);
		break;
	case 19:
		memcpy(pName, "黎族", 5);
		break;
	case 20:
		memcpy(pName, "傈僳族", 7);
		break;
	case 21:
		memcpy(pName, "佤族", 5);
		break;
	case 22:
		memcpy(pName, "畲族", 5);
		break;
	case 23:
		memcpy(pName, "高山族", 7);
		break;
	case 24:
		memcpy(pName, "拉祜族", 7);
		break;
	case 25:
		memcpy(pName, "水族", 5);
		break;
	case 26:
		memcpy(pName, "东乡族", 7);
		break;
	case 27:
		memcpy(pName, "纳西族", 7);
		break;
	case 28:
		memcpy(pName, "景颇族", 7);
		break;
	case 29:
		memcpy(pName, "柯尔克孜族", 11);
		break;
	case 30:
		memcpy(pName, "土族", 5);
		break;
	case 31:
		memcpy(pName, "达斡尔族", 9);
		break;
	case 32:
		memcpy(pName, "仫佬族", 7);
		break;
	case 33:
		memcpy(pName, "羌族", 5);
		break;
	case 34:
		memcpy(pName, "布朗族", 7);
		break;
	case 35:
		memcpy(pName, "撒拉族", 7);
		break;
	case 36:
		memcpy(pName, "毛南族", 7);
		break;
	case 37:
		memcpy(pName, "仡佬族", 7);
		break;
	case 38:
		memcpy(pName, "锡伯族", 7);
		break;
	case 39:
		memcpy(pName, "阿昌族", 7);
		break;
	case 40:
		memcpy(pName, "普米族", 7);
		break;
	case 41:
		memcpy(pName, "塔吉克族", 9);
		break;
	case 42:
		memcpy(pName, "怒族", 5);
		break;
	case 43:
		memcpy(pName, "乌孜别克族", 11);
		break;
	case 44:
		memcpy(pName, "俄罗斯族", 9);
		break;
	case 45:
		memcpy(pName, "鄂温克族", 9);
		break;
	case 46:
		memcpy(pName, "德昂族", 7);
		break;
	case 47:
		memcpy(pName, "保安族", 7);
		break;
	case 48:
		memcpy(pName, "裕固族", 7);
		break;
	case 49:
		memcpy(pName, "京族", 5);
		break;
	case 50:
		memcpy(pName, "塔塔尔族", 9);
		break;
	case 51:
		memcpy(pName, "独龙族", 7);
		break;
	case 52:
		memcpy(pName, "鄂伦春族", 9);
		break;
	case 53:
		memcpy(pName, "赫哲族", 7);
		break;
	case 54:
		memcpy(pName, "门巴族", 7);
		break;
	case 55:
		memcpy(pName, "珞巴族", 7);
		break;
	case 56:
		memcpy(pName, "基诺族", 7);
		break;
	case 57:
		memcpy(pName, "其他", 5);
		break;
	case 58:
		memcpy(pName, "外国血统", 9);
		break;
	default:
		return -1;
	}

	return NO_ERROR;
}

// 函数2
int ftGetNationCode(char *pCode, char *pName) {
	int nCode;
	int nRet;

	// 将ASCII码的民族代码转换成整型代码值
	nCode = (int) atoi(pCode);
	nRet = ftGetNationCode(nCode, pName);
	return nRet;
}

char *FuncP(int pVal) {
	//static char *p ;
	char *p;

	p = (char *) malloc(81);

	memset(p, 0x00, 81);

	switch (pVal) {
	case 0:
		memcpy(p, "abc", 3);
		break;
	case 1:
		memcpy(p, "123", 3);
		break;
	case 2:
		memcpy(p, "33dd33", 6);
		break;
	default:
		memcpy(p, "dd331133", 8);
		break;

	}

	return p;

}
/******************************************************************************************
 *******	函数名称：ftTLVGetStrVal2	 -序号：58-									*******
 *******	函数功能：解析TLV字符串获取指定TAG的值(不规则TL)									*******
 *******----------------------------------------------------------------------------*******
 *******	函数参数：char *pStr:要解析的TLV字符串									*******
 *******			  char *pTag:要获取值的TAG,多个TAG以符号,分格					*******
 *******              char *pVal:(输出)TAG值										*******
 *******----------------------------------------------------------------------------*******
 *******	返 回 值：成功返回：NO_ERROR，		失败返回错误代码					*******
 ******************************************************************************************/
int ftTLVGetStrVal2(char *pStr, char *pTag, char *pVal) {
	string strBuf;
	char szTags[10][5] = { {0} };
	char szTag[5];
	char szBuf[4096] = { 0 };
	char szVal[1024] = { 0 };
	char szLen1[3] = { 0 };
	char szTagBuf[5];
	int nLayer = 0;
	int nLen;
	int nPos;
	int nValLen = 0;
	int i;
	int nTagLen;
	bool bFlag;

	// 处理传入的TAG列表
	nLen = (int) strlen(pTag);
	strBuf = pTag;

	i = 0;
	nLayer = 0;
	// 拆分TAG列表
	while (true) {
		nPos = (int) strBuf.find(",", i);
		if (nPos == -1) {
			if (nLen > i) {
				memcpy(szTags[nLayer], pTag + i, (nLen - i));
				nLayer += 1;
			}

			break;
		} else {
			memcpy(szTags[nLayer], pTag + i, (nPos - i));
			i = nPos + 1;
			nLayer += 1;
		}
		if (nLayer >= 9) {	// 最大只处理9级
			break;
		}
	}
	if (nLayer <= 0) {
		return -1;
	}

	// 将TLV数据保存 CString对象
	nLen = (int) strlen(pStr);

	memcpy(szBuf, pStr, nLen);
	memcpy(szBuf + nLen, "\x00", 1);

	// 根据TAG级数处理TLV数据
	for (i = 0; i < nLayer; i++) {
		nPos = 0;
		bFlag = true;
		// 设置要获取数据的TAG
		nLen = (int) strlen(szTags[i]);
		memcpy(szTag, szTags[i], nLen);
		memcpy(szTag + nLen, "\x00", 1);

		while (bFlag) {
			memset(szTagBuf, 0x00, sizeof(szTagBuf));
			memcpy(szTagBuf, szBuf + nPos, 2);

			nTagLen = getTagLen(szTagBuf);
			if (nTagLen == 2) {
				memcpy(szTagBuf, szBuf + nPos, 4);
				nPos = nPos + 4;
			} else {
				nPos = nPos + 2;
			}
			if (memcmp(szTagBuf, szTag, nTagLen * 2) == 0) {
				// 取TAG数据长度高字节
				memset(szLen1, 0x00, sizeof(szLen1));
				memcpy(szLen1, szBuf + nPos, 2);
				if (memcmp(szLen1, "81", 2) == 0) {
					nPos += 2;
					memcpy(szLen1, szBuf + nPos, 2);

				}
				// 值长度
				nValLen = (int) ftHexToLong(szLen1);
				// ASCII码长度值
				nValLen = nValLen * 2;
				// 值开始位置
				nPos += 2;
				// 设置值数据
				memset(szVal, 0x00, sizeof(szVal));

				int ValueLenCmp = (int) strlen(szBuf)
						- ((int) strlen(szTagBuf) + (int) strlen(szLen1));
				//最后一个Tag
				if (i == nLayer - 1 || nValLen == ValueLenCmp) {

					memcpy(szVal, szBuf + nPos, nValLen);
				} else {
					memcpy(szVal, szBuf + nPos + nValLen, ValueLenCmp);
				}

				// 将值保存到szBuf中用于下次查询
				memset(szBuf, 0x00, sizeof(szBuf));
				memcpy(szBuf, szVal, ValueLenCmp);
				bFlag = false;

			} else {
				// 取TAG数据长度高字节
				memset(szLen1, 0x00, sizeof(szLen1));
				memcpy(szLen1, szBuf + nPos, 2);
				if (memcmp(szLen1, "81", 2) == 0) {
					nPos += 2;
					memcpy(szLen1, szBuf + nPos, 2);
				}
				// 值长度
				nValLen = (int) ftHexToLong(szLen1);
				// ASCII码长度值
				nValLen = nValLen * 2;
				// 值开始位置
				nPos = nPos + 2 + nValLen;
				//tal寻找完成
				if (nPos > strlen(szBuf)) {
					bFlag = false;
				}

			}
		}
	}

	// 成功设置函数输出变量
	memcpy(pVal, szVal, nValLen);
	memcpy(pVal + nValLen, "\x00", 1);

	return NO_ERROR;;
}

/************************************************************************/
/* 函数名称 nBinTonHex                序号-2-		                        */
/* 函数功能 二进制数字型字符串转成十六进制数字数字型字符串                       */
/* 函数参数 char *hexStr   （OUT）十六进制表示的字符数组                      */
/*         char *binStr   （IN）二进制表示的字符串                          */
/*         int  binLen     二进制表示的字符串的长度                         */
/*                                                                      */
/* 返回值	 转换后的字符串												    */
/************************************************************************/
int nBinTonHex(char *hexStr, char *binStr, int binLen) {
	int index = binLen / 4;

	for (int i = 0; i < index; i++) {
		int nReturn = 0;
		char pBuf[5] = { 0 };
		memcpy(pBuf, binStr + (i * 4), 4);
		int Len = (int) strlen(pBuf);
		for (int j = 0; j < Len; j++) {
			if (pBuf[j] == '1') {
				int nVal = (int) pow((double) 2, (int) Len - 1 - j);
				if (nVal > 0) {
					nReturn += nVal;
				}
			}
		}
		if (nReturn < 9) {
			sprintf(hexStr + i, "%d", nReturn);
		} else {
			switch (nReturn) {
			case 10:
				memcpy(hexStr + i, "A", 1);
				break;
			case 11:
				memcpy(hexStr + i, "B", 1);
				break;
			case 12:
				memcpy(hexStr + i, "C", 1);
				break;
			case 13:
				memcpy(hexStr + i, "D", 1);
				break;
			case 14:
				memcpy(hexStr + i, "E", 1);
				break;
			case 15:
				memcpy(hexStr + i, "F", 1);
				break;
			default:
				break;
			}
		}

	}
	return 0;
}
