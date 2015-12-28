/*
 * xpay.c
 *
 *  Created on: Nov 28, 2015
 *      Author: axis
 */

#include <stdio.h>                   // main.cpp
#include <openssl/evp.h>
//#include <crypto/evp/evp_locl.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include "RSA.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <mysql/mysql.h>
//#include <openssl/des.h>
#include "utils.h"
#include "publicFunc.h"

//const char* DB_HOST="127.0.0.1";
//const char* DB_USER="test";
//const char* DB_PWD="";
//const char* DB="test";

int main()
{
//       char origin_text[] = "hello world!";
//
//       // 由于采用RSA_PKCS1_PADDING方式，因此最大长度不要超过（即- 11）
//       int origin_len = sizeof(origin_text);
//       int enc_len = 0;
//       int dec_len = 0;
//       unsigned char *enc_data = NULL;
//       unsigned char *dec_data = NULL;
//
//       CRSA ro;
//       // 下面是重新生成key的代码，一般不需要
//       // ro.generate_key_str();
//
//       ro.set_params();
//       //ro.open_prikey_pubkey();
//       //ro.open_pubkey();
//       ro.open_prikey();
//
//       // 下面两行是私钥加密，公钥解密
//       ro.prikey_encrypt((const unsigned char *)origin_text, origin_len, (unsigned char **)&enc_data, enc_len);
//       char *e = bytes2hexString(enc_data,enc_len);
//       printf("密文：%s\n",e);
//       ro.pubkey_decrypt(enc_data, enc_len, (unsigned char **)&dec_data, dec_len);
//       char *d = bytes2hexString(dec_data,dec_len);
//       printf("明文：%s\n",d);
//
//       // 下面两行是公钥加密，私钥解密
//       //ro.pubkey_encrypt((const unsigned char *)origin_text, origin_len, (unsigned char **)&enc_data, enc_len);
//       //ro.prikey_decrypt(enc_data, enc_len, (unsigned char **)&dec_data, dec_len);
//
//       delete []enc_data;
//       delete []dec_data;

       char *asc="131415";
       char bcd[3];
       ftAtoh(asc,bcd,3);
       printf("bcd %s",bcd);

       return 0;
}
//int main() {
//	// insert code here...
//
//	// mysql test
//	MYSQL *dbconn;
//	dbconn = mysql_init(NULL);
//	if(dbconn == NULL)
//	{
//		printf("mysql_init failed!\n");
//		return EXIT_FAILURE;
//	}
//	dbconn = mysql_real_connect(dbconn, DB_HOST, DB_USER, DB_PWD, DB, 0, NULL, 0);
//	if (dbconn)
//	{
//		printf("db connected\n");
//	}else{
//		printf("db disconnect\n");
//	}
//	// ber-tlv
//	char* bertlv = "5A0888880003000000995F3401019F3901059F26083C5F9D7C62C0F93F9F02060000000100009F03060000000000009F1A020156950508800078005F2A0201569A031511279C01639F370425DC25DC82027C009F360200039F101307010103A02010010A010000020000E694008D";
//	BerTlv* p = (BerTlv*)malloc(100*sizeof(BerTlv));
//	int length = 0;
//	splitBerTlv(bertlv, (int) strlen(bertlv), p, &length);
//	// put p
//	for (int i = 0; i < length; i++) {
//		printf("tag: %s ", p[i].t);
//		printf("length: %s ", p[i].l);
//		printf("value: %s ", p[i].v);
//		printf("valuelength: %d\n", p[i].vlength);
//	}
//	printf("well done!\n");
//
//	unsigned char bytes[4] = {0x12,0x23,0x34,0x35};
//	int len = 4;
//	char* s= bytes2hexString(bytes,len);
//	printf("%s", s);
//
//	return EXIT_SUCCESS;
//}
//
