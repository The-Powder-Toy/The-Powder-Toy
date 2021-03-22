// based on public-domain code from Colin Plumb (1993)

#include <string.h>
#include "md5.h"

static unsigned getu32(const unsigned char *addr)
{
    return (((((unsigned long)addr[3] << 8) | addr[2]) << 8) | addr[1]) << 8 | addr[0];
}

static void putu32(unsigned data, unsigned char *addr)
{
    addr[0] = (unsigned char)data;
    addr[1] = (unsigned char)(data >> 8);
    addr[2] = (unsigned char)(data >> 16);
    addr[3] = (unsigned char)(data >> 24);
}

void md5_init(struct md5_context *ctx)
{
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xefcdab89;
    ctx->buf[2] = 0x98badcfe;
    ctx->buf[3] = 0x10325476;

    ctx->bits[0] = 0;
    ctx->bits[1] = 0;
}

void md5_update(struct md5_context *ctx, unsigned char const *buf, unsigned len)
{
    unsigned t;

    // update bit count
    t = ctx->bits[0];
    if((ctx->bits[0] = (t + ((unsigned)len << 3)) & 0xffffffff) < t)
	ctx->bits[1]++; // carry
    ctx->bits[1] += len >> 29;

    t = (t >> 3) & 0x3f;

    // use leading data to top up the buffer

    if(t) {
	unsigned char *p = ctx->in + t;

	t = 64-t;
	if (len < t) {
	    memcpy(p, buf, len);
	    return;
	}
	memcpy(p, buf, t);
	md5_transform(ctx->buf, ctx->in);
	buf += t;
	len -= t;
    }

    // following 64-byte chunks

    while(len >= 64) {
	memcpy(ctx->in, buf, 64);
	md5_transform(ctx->buf, ctx->in);
	buf += 64;
	len -= 64;
    }

    // save rest of bytes for later

    memcpy(ctx->in, buf, len);
}

void md5_final(unsigned char digest[16], struct md5_context *ctx)
{
    unsigned count;
    unsigned char *p;

    // #bytes mod64
    count = (ctx->bits[0] >> 3) & 0x3F;

    // first char of padding = 0x80
    p = ctx->in + count;
    *p++ = 0x80;

    // calculate # of bytes to pad
    count = 64 - 1 - count;

    // Pad out to 56 mod 64
    if(count < 8) {
	// we need to finish a whole block before padding
	memset(p, 0, count);
	md5_transform(ctx->buf, ctx->in);
	memset(ctx->in, 0, 56);
    } else {
	// just pad to 56 bytes
	memset(p, 0, count-8);
    }

    // append length & final transform
    putu32(ctx->bits[0], ctx->in + 56);
    putu32(ctx->bits[1], ctx->in + 60);

    md5_transform(ctx->buf, ctx->in);
    putu32(ctx->buf[0], digest);
    putu32(ctx->buf[1], digest + 4);
    putu32(ctx->buf[2], digest + 8);
    putu32(ctx->buf[3], digest + 12);
    memset(ctx, 0, sizeof(ctx));
}

#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

#define MD5STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data, w &= 0xffffffff, w = w<<s | w>>(32-s), w += x )

void md5_transform(unsigned buf[4], const unsigned char inraw[64])
{
    unsigned a, b, c, d;
    unsigned in[16];
    int i;

    for (i = 0; i < 16; ++i)
	in[i] = getu32 (inraw + 4 * i);

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    MD5STEP(F1, a, b, c, d, in[ 0]+0xd76aa478,  7);
    MD5STEP(F1, d, a, b, c, in[ 1]+0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in[ 2]+0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in[ 3]+0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in[ 4]+0xf57c0faf,  7);
    MD5STEP(F1, d, a, b, c, in[ 5]+0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in[ 6]+0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in[ 7]+0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in[ 8]+0x698098d8,  7);
    MD5STEP(F1, d, a, b, c, in[ 9]+0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in[10]+0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in[11]+0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in[12]+0x6b901122,  7);
    MD5STEP(F1, d, a, b, c, in[13]+0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in[14]+0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in[15]+0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, in[ 1]+0xf61e2562,  5);
    MD5STEP(F2, d, a, b, c, in[ 6]+0xc040b340,  9);
    MD5STEP(F2, c, d, a, b, in[11]+0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in[ 0]+0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in[ 5]+0xd62f105d,  5);
    MD5STEP(F2, d, a, b, c, in[10]+0x02441453,  9);
    MD5STEP(F2, c, d, a, b, in[15]+0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in[ 4]+0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in[ 9]+0x21e1cde6,  5);
    MD5STEP(F2, d, a, b, c, in[14]+0xc33707d6,  9);
    MD5STEP(F2, c, d, a, b, in[ 3]+0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in[ 8]+0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in[13]+0xa9e3e905,  5);
    MD5STEP(F2, d, a, b, c, in[ 2]+0xfcefa3f8,  9);
    MD5STEP(F2, c, d, a, b, in[ 7]+0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in[12]+0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, in[ 5]+0xfffa3942,  4);
    MD5STEP(F3, d, a, b, c, in[ 8]+0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in[11]+0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in[14]+0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in[ 1]+0xa4beea44,  4);
    MD5STEP(F3, d, a, b, c, in[ 4]+0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in[ 7]+0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in[10]+0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in[13]+0x289b7ec6,  4);
    MD5STEP(F3, d, a, b, c, in[ 0]+0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in[ 3]+0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in[ 6]+0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in[ 9]+0xd9d4d039,  4);
    MD5STEP(F3, d, a, b, c, in[12]+0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in[15]+0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in[ 2]+0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, in[ 0]+0xf4292244,  6);
    MD5STEP(F4, d, a, b, c, in[ 7]+0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in[14]+0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in[ 5]+0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in[12]+0x655b59c3,  6);
    MD5STEP(F4, d, a, b, c, in[ 3]+0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in[10]+0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in[ 1]+0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in[ 8]+0x6fa87e4f,  6);
    MD5STEP(F4, d, a, b, c, in[15]+0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in[ 6]+0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13]+0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in[ 4]+0xf7537e82,  6);
    MD5STEP(F4, d, a, b, c, in[11]+0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in[ 2]+0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in[ 9]+0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}

static char hex[] = "0123456789abcdef";
void md5_ascii(char *result, unsigned char const *buf, unsigned len)
{
    struct md5_context md5;
    unsigned char hash[16];
    int i;

    if(len==0)
	len = strlen((char *)buf);

    md5_init(&md5);
    md5_update(&md5, buf, len);
    md5_final(hash, &md5);

    for(i=0;i<16;i++) {
	result[i*2] = hex[hash[i]>>4];
	result[i*2+1] = hex[hash[i]&15];
    }
    result[32] = 0;
}
