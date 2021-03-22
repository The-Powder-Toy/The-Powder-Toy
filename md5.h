#ifndef MD5_H
#define MD5_H

struct md5_context {
	unsigned buf[4];
	unsigned bits[2];
	unsigned char in[64];
};

void md5_init(struct md5_context *context);
void md5_update(struct md5_context *context, unsigned char const *buf, unsigned len);
void md5_final(unsigned char digest[16], struct md5_context *context);
void md5_transform(unsigned buf[4], const unsigned char in[64]);

void md5_ascii(char *result, unsigned char const *buf, unsigned len);

#endif
