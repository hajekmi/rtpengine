#include <assert.h>
#include <stdio.h>

#include "crypto.h"
#include "rtplib.h"
#include "log.h"
#include "main.h"

#include <openssl/evp.h>

// rfc 7714 section 16
// contains 16/32 bytes of key, 12 bytes of salt [32:44]
uint8_t test_key[44] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x51, 0x75, 0x69, 0x64, 0x20, 0x70, 0x72, 0x6f,
	0x20, 0x71, 0x75, 0x6f
};

// rfc 7714 section 16
uint8_t srtp_pt[66] = {
	0x80, 0x40, 0xf1, 0x7b, 0x80, 0x41, 0xf8, 0xd3,
	0x55, 0x01, 0xa0, 0xb2, 0x47, 0x61, 0x6c, 0x6c,
	0x69, 0x61, 0x20, 0x65, 0x73, 0x74, 0x20, 0x6f,
	0x6d, 0x6e, 0x69, 0x73, 0x20, 0x64, 0x69, 0x76,
	0x69, 0x73, 0x61, 0x20, 0x69, 0x6e, 0x20, 0x70,
	0x61, 0x72, 0x74, 0x65, 0x73, 0x20, 0x74, 0x72,
	0x65, 0x73,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t answer128[66] = {
	0x80, 0x40, 0xf1, 0x7b, 0x80, 0x41, 0xf8, 0xd3,
	0x55, 0x01, 0xa0, 0xb2, 0xf2, 0x4d, 0xe3, 0xa3,
	0xfb, 0x34, 0xde, 0x6c, 0xac, 0xba, 0x86, 0x1c,
	0x9d, 0x7e, 0x4b, 0xca, 0xbe, 0x63, 0x3b, 0xd5,
	0x0d, 0x29, 0x4e, 0x6f, 0x42, 0xa5, 0xf4, 0x7a,
	0x51, 0xc7, 0xd1, 0x9b, 0x36, 0xde, 0x3a, 0xdf,
	0x88, 0x33, 0x89, 0x9d, 0x7f, 0x27, 0xbe, 0xb1,
	0x6a, 0x91, 0x52, 0xcf, 0x76, 0x5e, 0xe4, 0x39,
	0x0c, 0xce
};

uint8_t answer256[66] = {
	0x80, 0x40, 0xf1, 0x7b, 0x80, 0x41, 0xf8, 0xd3,
	0x55, 0x01, 0xa0, 0xb2, 0x32, 0xb1, 0xde, 0x78,
	0xa8, 0x22, 0xfe, 0x12, 0xef, 0x9f, 0x78, 0xfa,
	0x33, 0x2e, 0x33, 0xaa, 0xb1, 0x80, 0x12, 0x38,
	0x9a, 0x58, 0xe2, 0xf3, 0xb5, 0x0b, 0x2a, 0x02,
	0x76, 0xff, 0xae, 0x0f, 0x1b, 0xa6, 0x37, 0x99,
	0xb8, 0x7b, 0x7a, 0xa3, 0xdb, 0x36, 0xdf, 0xff,
	0xd6, 0xb0, 0xf9, 0xbb, 0x78, 0x78, 0xd7, 0xa7,
	0x6c, 0x13
};
// rfc 7714 section 17.1 - this is NOT the same
// as the putative test vector in section 17
// typo perhaps?
uint8_t srtcp_pt[68] = {
	0x81, 0xc8, 0x00, 0x0d, 0x4d, 0x61, 0x72, 0x73,
	0x4e, 0x54, 0x50, 0x31, 0x4e, 0x54, 0x50, 0x32,
	0x52, 0x54, 0x50, 0x20, 0x00, 0x00, 0x04, 0x2a,
	0x00, 0x00, 0xe9, 0x30, 0x4c, 0x75, 0x6e, 0x61,
	0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef,
	0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef,
	0xde, 0xad, 0xbe, 0xef,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t answer128_srtcp[72] = {
	0x81, 0xc8, 0x00, 0x0d, 0x4d, 0x61, 0x72, 0x73,
	0x63, 0xe9, 0x48, 0x85, 0xdc, 0xda, 0xb6, 0x7c,
	0xa7, 0x27, 0xd7, 0x66, 0x2f, 0x6b, 0x7e, 0x99,
	0x7f, 0xf5, 0xc0, 0xf7, 0x6c, 0x06, 0xf3, 0x2d,
	0xc6, 0x76, 0xa5, 0xf1, 0x73, 0x0d, 0x6f, 0xda,
	0x4c, 0xe0, 0x9b, 0x46, 0x86, 0x30, 0x3d, 0xed,
	0x0b, 0xb9, 0x27, 0x5b, 0xc8, 0x4a, 0xa4, 0x58,
	0x96, 0xcf, 0x4d, 0x2f, 0xc5, 0xab, 0xf8, 0x72,
	0x45, 0xd9, 0xea, 0xde, 0x80, 0x00, 0x05, 0xd4
};

uint8_t answer256_srtcp[72] = {
	0x81, 0xc8, 0x00, 0x0d, 0x4d, 0x61, 0x72, 0x73,
	0xd5, 0x0a, 0xe4, 0xd1, 0xf5, 0xce, 0x5d, 0x30,
	0x4b, 0xa2, 0x97, 0xe4, 0x7d, 0x47, 0x0c, 0x28,
	0x2c, 0x3e, 0xce, 0x5d, 0xbf, 0xfe, 0x0a, 0x50,
	0xa2, 0xea, 0xa5, 0xc1, 0x11, 0x05, 0x55, 0xbe,
	0x84, 0x15, 0xf6, 0x58, 0xc6, 0x1d, 0xe0, 0x47,
	0x6f, 0x1b, 0x6f, 0xad, 0x1d, 0x1e, 0xb3, 0x0c,
	0x44, 0x46, 0x83, 0x9f, 0x57, 0xff, 0x6f, 0x6c,
	0xb2, 0x6a, 0xc3, 0xbe, 0x80, 0x00, 0x05, 0xd4
};

struct rtpengine_config rtpe_config = {
};

int main(void)
{

	str suite, payload;
	const struct crypto_suite *c;
	struct crypto_context ctx;
	int rc;

	uint8_t working[100];

	crypto_init_main();

	suite = STR("AEAD_AES_128_GCM");
	c = crypto_find_suite(&suite);
	assert(c);

	memset(&ctx, 0, sizeof(ctx));
	ctx.params.crypto_suite = c;
	ctx.session_key_ctx[0] = EVP_CIPHER_CTX_new();

	memcpy(ctx.session_key, test_key, 16);
	memcpy(ctx.session_salt, (uint8_t *)test_key + 32, 12);

	memcpy(working, srtp_pt, 50);
	payload = STR_LEN((char *) working + 12, 38);

	rc = crypto_encrypt_rtp(&ctx, (struct rtp_header *)working,
					   &payload,
					   0x00000000f17bULL);
	assert(rc == 0 && payload.len == 54);
	assert(memcmp(working, answer128, 66) == 0);
	printf("RTP/AEAD-AES-128-GCM Encrypt - PASS\n");

	payload = STR_LEN((char *) working + 12, 54);

	rc = crypto_decrypt_rtp(&ctx, (struct rtp_header *)working,
					   &payload,
					   0x00000000f17bULL);

	assert(rc == 0 && payload.len == 38);
	assert(memcmp(working, srtp_pt, 50) == 0);
	printf("RTP/AEAD-AES-128-GCM Decrypt - PASS\n");

	// RTCP
	memcpy(working, srtcp_pt, 52);
	payload = STR_LEN((char *) working + 8, 44);

	rc = crypto_encrypt_rtcp(&ctx, (struct rtcp_packet *)working,
						&payload,
						0x0000000005d4ULL);
	assert(rc == 0 && payload.len == 60);
	assert(memcmp(working, answer128_srtcp, 68) == 0);
	printf("RTCP/AEAD-AES-128-GCM Encrypt - PASS\n");

	payload = STR_LEN((char *) working + 8, 60);

	rc = crypto_decrypt_rtcp(&ctx, (struct rtcp_packet *)working,
						&payload,
						0x000000005d4ULL);

	assert(rc == 0 && payload.len == 44);
	assert(memcmp(working, srtcp_pt, 52) == 0);
	printf("RTCP/AEAD-AES-128-GCM Decrypt - PASS\n");

	crypto_cleanup_session_key(&ctx);

	// AES 256
	suite = STR("AEAD_AES_256_GCM");
	c = crypto_find_suite(&suite);
	assert(c);

	memset(&ctx, 0, sizeof(ctx));
	ctx.params.crypto_suite = c;
	ctx.session_key_ctx[0] = EVP_CIPHER_CTX_new();

	memcpy(ctx.session_key, test_key, 32);
	memcpy(ctx.session_salt, (uint8_t *)test_key + 32, 12);

	memcpy(working, srtp_pt, 50);
	payload = STR_LEN((char *) working + 12, 38);

	rc = crypto_encrypt_rtp(&ctx, (struct rtp_header *)working,
					   &payload,
					   0x00000000f17bULL);
	assert(rc == 0 && payload.len == 54);
	assert(memcmp(working, answer256, 66) == 0);
	printf("RTP/AEAD-AES-256-GCM Encrypt - PASS\n");

	payload = STR_LEN((char *) working + 12, 54);

	rc = crypto_decrypt_rtp(&ctx, (struct rtp_header *)working,
					   &payload,
					   0x00000000f17bULL);
	assert(rc == 0 && payload.len == 38);
	assert(memcmp(working, srtp_pt, 50) == 0);
	printf("RTP/AEAD-AES-256-GCM Decrypt - PASS\n");

	// RTCP
	memcpy(working, srtcp_pt, 52);
	payload = STR_LEN((char *) working + 8, 44);

	rc = crypto_encrypt_rtcp(&ctx, (struct rtcp_packet *)working,
						&payload,
						0x0000000005d4ULL);
	assert(rc == 0 && payload.len == 60);
	assert(memcmp(working, answer256_srtcp, 68) == 0);
	printf("RTCP/AEAD-AES-256-GCM Encrypt - PASS\n");

	payload = STR_LEN((char *) working + 8, 60);

	rc = crypto_decrypt_rtcp(&ctx, (struct rtcp_packet *)working,
						&payload,
						0x000000005d4ULL);

	assert(rc == 0 && payload.len == 44);
	assert(memcmp(working, srtcp_pt, 52) == 0);
	printf("RTCP/AEAD-AES-256-GCM Decrypt - PASS\n");

	crypto_cleanup_session_key(&ctx);
}

int get_local_log_level(unsigned int u) {
	return -1;
}
