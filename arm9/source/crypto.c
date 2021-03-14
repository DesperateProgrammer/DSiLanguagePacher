#include <stdint.h>
#include "_aes.h"
#include "crypto.h"
#include "u128_math.h"
#include "f_xy.h"

// more info:
//		https://github.com/Jimmy-Z/TWLbf/blob/master/dsi.c
//		https://github.com/Jimmy-Z/bfCL/blob/master/dsi.h
// ported back to 32 bit for ARM9

static uint32_t nand_rk[RK_LEN];
static uint32_t es_rk[RK_LEN];
static uint32_t boot2_rk[RK_LEN];

static uint8_t nand_ctr_iv[16];
static uint8_t boot2_ctr[16];
static int tables_generated = 0;


static void dsi_aes_set_key(uint32_t *rk, const uint32_t *console_id, key_mode_t mode) 
{
	uint32_t key[4];
	switch (mode) 
  {
    case NAND:
      key[0] = console_id[0];
      key[1] = console_id[0] ^ KEYSEED_DSI_NAND_0;
      key[2] = console_id[1] ^ KEYSEED_DSI_NAND_1;
      key[3] = console_id[1];
      break;
    case NAND_3DS:
      key[0] = (console_id[0] ^ KEYSEED_3DS_NAND_0) | 0x80000000;
      key[1] = KEYSEED_3DS_NAND_1;
      key[2] = KEYSEED_3DS_NAND_2;
      key[3] = console_id[1] ^ KEYSEED_3DS_NAND_3;
      break;
    case ES:
      key[0] = KEYSEED_ES_0;
      key[1] = KEYSEED_ES_1;
      key[2] = console_id[1] ^ KEYSEED_ES_2;
      key[3] = console_id[0];
      break;
    default:
      break;
	}
	u128_xor(key, mode == ES ? DSi_ES_KEY_Y : DSi_NAND_KEY_Y);
	u128_add(key, DSi_KEY_MAGIC);
  u128_lrot(key, 42) ;
	aes_set_key_enc_128_be(rk, (uint8_t*)key);
}

int dsi_sha1_verify(const void *digest_verify, const void *data, unsigned len) 
{
	uint8_t digest[SHA1_LEN];
	swiSHA1Calc(digest, data, len);
	return memcmp(digest, digest_verify, SHA1_LEN);
}

void dsi_crypt_init(const uint8_t *console_id_be, const uint8_t *emmc_cid, int is3DS) 
{
	if (tables_generated == 0) 
  {
		aes_gen_tables();
		tables_generated = 1;
	}
	
	uint32_t console_id[2];
	GET_UINT32_BE(console_id[0], console_id_be, 4);
	GET_UINT32_BE(console_id[1], console_id_be, 0);

	dsi_aes_set_key(nand_rk, console_id, is3DS ? NAND_3DS : NAND);
	dsi_aes_set_key(es_rk, console_id, ES);

	aes_set_key_enc_128_be(boot2_rk, (uint8_t*)DSi_BOOT2_KEY);

	swiSHA1Calc(nand_ctr_iv, emmc_cid, 16);

}

static inline void aes_ctr(const uint8_t *rk, const uint8_t *ctr, uint8_t *in, uint8_t *out) 
{
	uint8_t xor[16];
	aes_encrypt_128_be(rk, ctr, xor);
  memcpy(out, in, 16) ;
	u128_xor(out, xor);
}

// crypt one block, in/out must be aligned to 32 bit(restriction induced by xor_128)
// offset as block offset, block as AES block
void dsi_nand_crypt_1(uint8_t* out, const uint8_t* in, uint32_t offset) 
{
	uint8_t ctr[16] ;
  memcpy(ctr, nand_ctr_iv, sizeof(nand_ctr_iv)) ;
	u128_add32(ctr, offset);
	aes_ctr(nand_rk, ctr, (uint32_t*)in, (uint32_t*)out);
}

void dsi_nand_crypt(uint8_t* out, const uint8_t* in, uint32_t offset, unsigned count) 
{
	uint8_t ctr[16] ;
  memcpy(ctr, nand_ctr_iv, sizeof(nand_ctr_iv)) ;
	u128_add32(ctr, offset);
	for (unsigned i = 0; i < count; ++i) 
  {
		aes_ctr(nand_rk, ctr, (uint32_t*)in, (uint32_t*)out);
		out += AES_BLOCK_SIZE;
		in += AES_BLOCK_SIZE;
    u128_add32(ctr, 1);
	}
}
	
void dsi_boot2_crypt_set_ctr(uint32_t size_r) 
{
  for (int i=0;i<4;i++)
  {
    boot2_ctr[i] = (size_r) >> (8*i) ;
    boot2_ctr[i+4] = (-size_r) >> (8*i) ;
    boot2_ctr[i+8] = (~size_r) >> (8*i) ;
    boot2_ctr[i+12] = 0 ;
  }
}

void dsi_boot2_crypt(uint8_t* out, const uint8_t* in, unsigned count) 
{
	for (unsigned i = 0; i < count; ++i) 
  {
		aes_ctr(boot2_rk, boot2_ctr, (uint32_t*)in, (uint32_t*)out);
		out += AES_BLOCK_SIZE;
		in += AES_BLOCK_SIZE;
    u128_add32(boot2_ctr, 1);
	}
}
