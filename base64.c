/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005-2011, Jouni Malinen <j@w1.fi>
 * http://web.mit.edu/freebsd/head/contrib/wpa/src/utils/base64.c
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

//#include "includes.h"

//#include "os.h"

#include "base64.h"
#include <stdio.h>
#include <math.h>


static const unsigned char base64_table[65] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const unsigned char base64_itable[65] =
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/";


unsigned char* base64_long_encode(const long long lon,
	size_t* out_len) {
	unsigned char* out, * pos;
	const unsigned char* end, * in;
	long long olen;
	long long tmp=lon;
  

	olen = lon/64;
	olen = log2(lon) / 8;

	olen+=3; /* nul termination */
	//printf("00lon %lld %lld\n", lon,olen);
	//*out_len=olen;
	//if (olen < len)
	//	return NULL; /* integer overflow */
	out = malloc(olen);
	if (out == NULL)
		return NULL;
	//printf("1lon %lld \n", lon);

	pos=out;
  do {
      //printf("1lon %lld olen %lld tmp %lld %lld\n",lon,olen,tmp,tmp % 64);
      *pos++ = base64_itable[tmp % 64];
      tmp= tmp>>6;
      //printf("2lon %lld olen %lld tmp %lld %lld\n",lon,olen,tmp,tmp % 64);
  } while (tmp && olen--);
   
  *pos = '\0';
	if (out_len)
		*out_len = pos - out;
	strrev(out);
  return out;
}

long long base64_long_decode(const unsigned char* src, size_t len) {
  unsigned char* pos,dtable[256];
	
//	printf("\n1l:%i s:%s\n",len,src);
	
	memset(dtable, 0x00, 256);
	for (int i = 0; i < sizeof(base64_itable) - 1; i++)
		dtable[base64_itable[i]] = (unsigned char)i;
	dtable['='] = 0;
	
	long long tmp=0;
	
//	printf("2l:%i d:%s %i\n",len,src,tmp);
	
	for (int i=0;i<len;i++) {
//  printf("3l:%i src[i]:%i d2:%i i:%i = %i\n",len,src[i],dtable[src[i]],i,tmp);
	
    if (!src[i]) break;
	  tmp=(tmp << 6) + dtable[src[i]];
//  printf("4l:%i src[i]:%i d2:%i i:%i = %i\n",len,src[i],dtable[src[i]],i,tmp);
	
  }
  
  free(dtable);
  return tmp;
}



/**
 * base64_encode - Base64 encode
 * @src: Data to be encoded
 * @len: Length of the data to be encoded
 * @out_len: Pointer to output length variable, or %NULL if not used
 * Returns: Allocated buffer of out_len bytes of encoded data,
 * or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer. Returned buffer is
 * nul terminated to make it easier to use as a C string. The nul terminator is
 * not included in out_len.
 */
unsigned char* base64_encode(const unsigned char* src, size_t len,
	size_t* out_len)
{
	unsigned char* out, * pos;
	const unsigned char* end, * in;
	size_t olen;
	int line_len;

	olen = len * 4 / 3 + 4; /* 3-byte blocks to 4-byte */
	olen += olen / 72; /* line feeds */
	olen++; /* nul termination */
	if (olen < len)
		return NULL; /* integer overflow */
	out = malloc(olen);
	if (out == NULL)
		return NULL;

	end = src + len;
	in = src;
	pos = out;
	line_len = 0;
	while (end - in >= 3) {
		*pos++ = base64_table[in[0] >> 2];
		*pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
		*pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
		*pos++ = base64_table[in[2] & 0x3f];
		in += 3;
		line_len += 4;
		if (line_len >= 72) {
			//*pos++ = '\n';
			line_len = 0;
		}
	}

	if (end - in) {
		*pos++ = base64_table[in[0] >> 2];
		if (end - in == 1) {
			*pos++ = base64_table[(in[0] & 0x03) << 4];
			*pos++ = '=';
		}
		else {
			*pos++ = base64_table[((in[0] & 0x03) << 4) |
				(in[1] >> 4)];
			*pos++ = base64_table[(in[1] & 0x0f) << 2];
		}
		*pos++ = '=';
		line_len += 4;
	}

//	if (line_len)
//		*pos++ = '\n';

	*pos = '\0';
	if (out_len)
		*out_len = pos - out;
	return out;
}


/**
 * base64_decode - Base64 decode
 * @src: Data to be decoded
 * @len: Length of the data to be decoded
 * @out_len: Pointer to output length variable
 * Returns: Allocated buffer of out_len bytes of decoded data,
 * or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer.
 */
unsigned char* base64_decode(const unsigned char* src, size_t len,
	size_t* out_len)
{
	unsigned char dtable[256], * out, * pos, block[4], tmp;
	size_t i, count, olen;
	int pad = 0;

	memset(dtable, 0x80, 256);
	for (i = 0; i < sizeof(base64_table) - 1; i++)
		dtable[base64_table[i]] = (unsigned char)i;
	dtable['='] = 0;

	count = 0;
	for (i = 0; i < len; i++) {
		if (dtable[src[i]] != 0x80)
			count++;
	}

	if (count == 0 || count % 4)
		return NULL;

	olen = count / 4 * 3;
	pos = out = malloc(olen);
	if (out == NULL)
		return NULL;

	count = 0;
	for (i = 0; i < len; i++) {
		tmp = dtable[src[i]];
		if (tmp == 0x80)
			continue;

		if (src[i] == '=')
			pad++;
		block[count] = tmp;
		count++;
		if (count == 4) {
			*pos++ = (block[0] << 2) | (block[1] >> 4);
			*pos++ = (block[1] << 4) | (block[2] >> 2);
			*pos++ = (block[2] << 6) | block[3];
			count = 0;
			if (pad) {
				if (pad == 1)
					pos--;
				else if (pad == 2)
					pos -= 2;
				else {
					/* Invalid padding */
					free(out);
					return NULL;
				}
				break;
			}
		}
	}

	*out_len = pos - out;
	return out;
}