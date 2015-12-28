#ifndef ____publicFunc__
#define ____publicFunc__
#ifdef __cplusplus
extern "C" {
#endif

//第一部分: 基础公共函数
//-----------------------------------------------------------------------------------
//1.1.ASC码转换成BCD码
int ftAtoh(char *ascstr, char *bcdstr, int bcdlen);

//1.2.BCD码转换成ASC码
int ftHtoa(char *hexstr, char *ascstr, int length);

//1.3.16进制金额字符串转换成：10进制金额
long ftHexToLong(char *HexStr);

//1.4.将long型的金额值转换成16进制的金额字符串 默认值：Flag = 0
int ftLongToHex(long Val, char *HexStr, int Flag);

//1.5.填充16进制的金额字符串到 4字节 BCD码的ASCII字符；
int ftFillAmount(char *Str);

//1.6.MAC/TAC计算的字符串填充函数,填充80 或 80 00..到长度为8的倍数
int ftFillString(char *Str);

//1.7.填充字符串到指定长度 默认值：Val = 'F'
int ftFillStringF(char *Str, int Len, char Val);

//1.8.将字符串转换成大写
void ftStringToUpper(char *Str);

//1.9.将字符串转换成小写
void ftStringToLower(char *Str);

//1.10.将字符值转换成二进制编码表示的字符串，字符串长度为8
int ftCharToBitString(unsigned char Val, char *OutBuf);

//1.11.将缓冲区数据转换成二进制编码表示的字符串(字符串中每个字节转换成二进制编码为8个字节)
int ftDataToBitString(unsigned char *InData, int Len, char *OutBuf);

//1.11.1将缓冲区数据转换成二进制编码表示的字符串(字符串中每个字节转换成二进制编码为4个字节去掉了0011高4位)
int ftDataToBitStringNoASCII(char *OutBuf, unsigned char *InData, int Len);

//1.12.将BIT字符串转换成整型值
int ftBitStringToLong(char *BitString, int Len, long *Val);

//1.13.将BIT字符串转换成16进制值缓冲区
int ftBitStringToData(char *BitString, int Len, unsigned char *OutData);

//1,14.异或计算[数据间异或]
int ftCalXOR(char *Param1, char *Param2, int Len, char *Out);
int ftStrXOR(char *Param1, char *Param2, char *Out);

int ftCalLRC1(char *DataBuf, int Len);

//1.15.数据异或验证和计算函数
int ftCalLRC(char *DataBuf, int Len, char *RetVal);

//1.16.密码加密函数(密码及相关数据加密)
int ftDataEncL(char *pIn, int pLen, char *pOut);
int ftDataEnc(char *pIn, char *pOut);

//1.17.密码解密函数
int ftDataDecL(char *pIn, int pLen, char *pOut);
int ftDataDec(char *pIn, char *pOut);

//1.18.DES加密,默认值：pFlag = 0 ，计算数据为BCD码
int ftDesEncL(char *pKey, int pKeyLen, char *pIn, char *pOut, int Len,
		int pFlag);

//传入参数为ASCII码的DES加密函数
int ftDesEnc(char *pKey, char *pIn, char *pOut);

//1.19.3DES加密 默认值：pFlag=0
int ft3DesEncL(char *pKey, int pKeyLen, char *pIn, char *pOut, int Len,
		int pFlag);

//传入参数为ASCII码的3DES加密
int ft3DesEnc(char *pKey, char *pIn, char *pOut);

//1.20.DES解密 默认值：pFlag=0
int ftDesDecL(char *pKey, int pKeyLen, char *pIn, char *pOut, int Len,
		int pFlag);

//传入参数为ASCII码的DES解密
int ftDesDec(char *pKey, char *pIn, char *pOut);

//1.21.3DES解密
int ft3DesDecL(char *pKey, int pKeyLen, char *pIn, char *pOut, int Len,
		int pFlag);

//传入参数为ASCII码的3DES解密
int ft3DesDec(char *pKey, char *pIn, char *pOut);

//1.22.日志记录函数
void ftWriteLog(char *pFileName, char *format, ...);
void ftWriteLog2(char *pFileName, char *format, ...);

//1.23.写数据的二进制日志
void ftWriteHexLog(char *pFileName, char *pBuf, int pLen);

//1.24. 字符串拆分函数
int ftSplitStr(char *pStr, char pSplitChar, char pData[][81]);
//int ftSplitStr(char *pStr,char pSplitChar,CStringArray *pArrItem);

//1.25.检查字符串是否为字母字符
int ftCheckStrAlpha(char *pStr);

//1.26.检查字符串是否为特殊字符
int ftCheckStrS(char *pStr);

//1.27.二进制数据字符串检查函数
int ftCheckStrBin(char *pStr);

//1.28.检查字符串是否为字母和数字
int ftCheckStrAn(char *pStr);

//1.29.检查字符串是否为字母和特殊字符
int ftCheckStrAs(char *pStr);

//1.30.检查字符串是否为字母，数字和特殊字符
int ftCheckStrAns(char *pStr);

//1.31. 文件处理类API函数
int ftCheckFile(char *pFileName, long *pFileLen);

//1.32. 将缓冲区写入到新行文件
int ftWriteLineToFile(char *pFileName, char *pBuf, int pLen, int pMode);

//1.33 删除字符串前面的空格
int ftDelLeftStr(char *pStr);

//1.34.删除字符串后面的空格
int ftDelRightStr(char *pStr);

//1.35.删除字符串中的所有空格
int ftDelAllStr(char *pStr);

//1.36.检查字符串是否为数字
int ftCheckStrNum(char *pStr);

//1.37.将Unicode编码的字符串转换成GBK编码的字符串
//int ftUniToStr(char *pStr, char *pOut);

//1.38.将ANSI字符串转换成Unicode编码
//int ftStrToUni(char *pStr,char *pOut);

//2.39.BCD码字符串按位取反
int ftStrGetRever(char *pSrc, char *pOut);

//第二部分: 日期时间类处理函数
//-----------------------------------------------------------------------------------
//2.1. 获取当前系统日期
int ftGetSysDate(char *pSysDate, int pFlag);

//2.2. 获取当前系统时间
int ftGetSysTime(char *pSysTime, int pFlag);

//2.3. 获取当前系统日期的星期字符串
int ftGetWeek(char *pWeekStr, int pFlag);

//2.4. 检测指定的年份是否为闰年
int ftCheckLeepYear(int pYear);

//2.5. 检查日期合法性
int ftCheckDate(char *pDate);

//2.6. 获取日期中的年
int ftGetYear(char *pDay);

//2.7. 获取日期中的月
int ftGetMonth(char *pDay);

//2.8. 获取日期中的日
int ftGetDay(char *pDay);

//2.9. 将YYYYMMDD格式日期拆分成整型的年、月、日
int ftSplitDate(char *pDate, int *pYear, int *pMonth, int *pDay);

//2.10. 计算两个时间的差值
int ftDiffTime(char *pStartTime, char *pEndTime, int *pSecond,
		int *pMilliSecond, int pFlag);

//第三部分: PBOC1.0 电子钱包/电子存折 相关API函数
//-----------------------------------------------------------------------------------
//3.1. 圈存、圈提、取现、消费 交易过程密钥计算函数
int ftCalSKL(char *pKey, int pKeyLen, char *pIn, int pLen, char *pOut,
		int pFlag);
int ftCalSK(char *pKey, char *pIn, char *pOut);

//3.2. 圈存、圈提、取现、消费 交易MAC计算函数
int ftCalMacL(char *Key, char *Vector, char *Data, int DataLen, char *Out,
		int Flag);
int ftCalMac(char *Key, char *Vector, char *Data, char *Out);

//3.3. 无特殊情况的3DES计算MAC函数,增加 Flag = 0 参数用于实现PBOC3.0 的密文数据计算,8字节
int ftCalMacFor3Des(char *Key, char *InitData, char *Data, char *Out, int Flag);

//3.4 无特殊情况的DES计算MAC函数
int ftCalMacForDes(char *Key, char *InitData, char *Data, char *Out);

//3.5. 密钥分散函数
int ftDiversify(char *Data, char *Key, char *Out);

//3.6. 根据BCD码的ATR信息计算IC卡的通信协议,返回值为通信协议类型；0:通信协议为：T=0 ; 1:通信协议为：T=1
int ftGetProtocolForATR(unsigned char *pATR);

//第四部分: PBOC2.0 相关API函数
//-----------------------------------------------------------------------------------

//4.1.计算PBOC2.0外部认证指令的终端密文和C-MAC值
// 如函数成功，生成的终端密文 + C-MAC值为：75560EE1C7A1808F 302C484C9BF9E990
// 则对应的外部认证指令：8482000010 75560EE1C7A1808F 302C484C9BF9E990
// pKey：为计算密钥；
// pTermReand：为初始化安全通道指令(8050)传入的终端随机数；
int ftCalcExtAuth2(char *pKey, char *pTermRandom, char *pData, char *pOut);

//4.2.计算GP 指令中 PUT KEY 中的密钥的密文
int ftCalcPutKey(char *pKey, char *pInitData, char *pOut);

//46.获取4字节，或8字节的终端随机数 -- Add-2013-03-26
int ftGetRand(int pLen, char *pRand);

// TLV相关函数
//47.根据TAG码获取TLV中TAG码的值及长度
int ftTLVGetVal(char *pTLV, char *pTAG, int *pLen, char *pVal);

//定义一个保存TAG定义的数据结构
typedef struct {
	char szTagCode[5];			// TAG编码
	char szTagName[51];			// TAG描述
	char szDataType[5];	// 数据类型，值为：b-二进制; n-数字型;	cn-压缩数字型;	an-字母数字;	ans - 特殊字母数字
	char szLenType[2];			// 值长度类型，0-定长度; 1-变长
	char szValLen[7];           // 值长度，值长度类型为：定长，此值为固定值；值长度类型为变长此值为：var或最多长度；
	char szTempFlag[2];			// 模板标志；0-非模板TAG;1-模板TAG
} STDICT;

//48.初始化读取TLV数据字典的数据元素定义
//int ftTLVInitDict();

//49.是否TLV数据字典
int ftTLVFreeDict();

//50.获取TAG定义数据字典定义的TAG个数
int ftTLVGetTAGNum(int *pNum);

//51.获取TAG定义数据字典中的所有TAG码列表
//int ftTLVGetTAGNames(int *pNum,char pNames[][5]);

//52.获取TAG定义数据字典中指定TAG码的定义结构信息
int ftTLVGetTAGDef(char *pTagName, STDICT *pElDef);

//53.拆分TAG码值列表数据中的TAG码和值
int ftTLVSplitTAG(char *pBuf, char *pTagCode, char *pTagVal);

//54.初始化TAG值列表对象，在TAG赋值前需先调用此函数
int ftTLVInitTagList();

//55.删除TAG值列表对象中的所有TAG值，并设置处理标志为初始化前状态
int ftTLVFreeTagList();

//56.TAG码赋值
int ftTLVPutVal(char *pTag, char *pTagVal);

//57.取TAG值列表存储的TAG值个数
int ftTLVGetValNum(int *pNum);

//58.取字符串中指定TAG的值,
int ftTLVGetStrVal(char *pStr, char *pTag, char *pVal);

//51.将字典中各个赋值的元素值转换成BUF

//52.将BUF解包到字典的各数据元素对象中

//获取系统函数调用的错误代码及错误信息(指获取VC函数返回错误代码的错误信息)
//int ftGetErrCodeAndMsg(long *ErrCode,char *ErrMsg);

//PBOC TLV相关函数

// 第五部分:规范实现函数

//5.1.根据性别代码获取性别名称			GB/T 2261.1―2003 个人基本信息分类与代码 第1部分: 人的性别代码
int ftGetSexCodeI(int pCode, char *pName);
int ftGetSexCode(char *pCode, char *pName);

//5.2.根据婚姻状态代码获取状态名称		GB/T 2261.2-2003 个人基本信息分类与代码 第2部分: 婚姻状况代码
int ftGetMarrCodeI(int pCode, char *pName);
int ftGetMarrCode(char *pCode, char *pName);

//5.3.根据民族代码获取民族名称			GB/T 3304-1991 中国各民族名称的罗马字母拼写法和代码
int ftGetNationCodeI(int pCode, char *pName);
int ftGetNationCode(char *pcode, char *pName);

// 返回指令函数的测试
char *FuncP(int pVal);

//柯清元定义
int nBinTonHex(char *hexStr, char *binStr, int binLen);
int ftTLVGetStrVal2(char *pStr, char *pTag, char *pVal);

// 定义公共函数返回值
#define FTPRNOTDICT				-1				// TAG码定义数据字典配置文件dict.cfg不存在;
#define FTPROPENDICTERR			-2				// 打开TAG码定义数据字典配置文件dict.cfg失败;
#define FTPRNOTTAGDEF			-3				// 配置文件dict.cfg中无TAG码定义信息;
#define FTPRTAGDEFERR			-4				// 配置文件dict.cfg中TAG码定义错误
#define FTPRTAGTYPEERR			-5				// 配置文件dict.cfg中TAG类型定义错误
#define FTPRTAGVALLENTYPEERR	-6				// 配置文件dict.cfg中TAG值类型定义错误
#define FTPRTAGVALLENERR		-7				// 配置文件dict.cfg中TAG值长度定义错误
#define FTPRTAGTEMPFLAGERR		-8				// 配置文件dict.cfg中模板标志定义错误
#define FTPRNOTINITDICT			-9				// TAG码定义数据字典未初始化
#define FTPRNOTDICTEL			-10				// TAG码定义数据字典无TAG码定义对象
#define FTPRGETSTARTPOSERR		-11				// 调用字典对象的GetStartPosition函数失败
#define FTPRGETELERR            -12             // 取TAG码定义数据字典中的指定TAG码定义对象失败,或字典中无此TAG码对象
#define FTPRNOTINITTAGLIST		-13				// TAG码值列表对象未初始化
#define FTPRPARAMERR			-14				// 函数传入参数非法(如：TAG码长度不为：2或4、TAG的ASCII值长度超过：255等)

#define FTPROTHERERR            -99				// 其他错误

// 定义公共函数相关常量
#define FTTAGMAXNUM             256				// TAG码最大个数

#define NO_ERROR 0L //成功

#ifdef __cplusplus
}
#endif
#endif /*(defined __publicFunc_)*/
