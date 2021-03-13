/* f_xy.c
 *
 * This file was imported from godmode9i, but it is liely not the
 * original source. twltool uses the same file.
 *
 * If you happen to know whom to credit I'd love to add the name
 *
 * Refactored to reduce the pointer casts and remove the dependency 
 * from tonccpy.
 */

#include <string.h>
#include <stdint.h>

/************************ Constants / Defines *********************************/

static const uint8_t keyconst[] = { 0x79, 0x3e, 0x4f, 0x1a, 0x5f, 0x0f, 0x68, 0x2a,
                                    0x58, 0x02, 0x59, 0x29, 0x4e, 0xfb, 0xfe, 0xff }; 

/************************ Functions *******************************************/

// rotate a 128bit, little endian by shift bits in direction of increasing significance.
void n128_lrot(uint8_t *num, uint32_t shift)
{
  uint8_t tmp[16];
  for (int i=0;i<16;i++)
  {
    // rot: rotate to more significant.
    // LSB is tmp[0], MSB is tmp[15]
    const uint32_t byteshift = shift / 8 ;
    const uint32_t bitshift = shift % 8;
    tmp[(i+byteshift) % 16] = (num[i] << bitshift)
           | ((num[(i-1) % 16] >> (8-bitshift)) & 0xff);    
  }
  memcpy(num, tmp, 16) ;
}

// rotate a 128bit, little endian by shift bits in direction of decreasing significance.
void n128_rrot(uint8_t *num, uint32_t shift)
{
  uint8_t tmp[16];
  for (int i=0;i<16;i++)
  {
    // rot: rotate to less significant.
    // LSB is tmp[0], MSB is tmp[15]
    const uint32_t byteshift = shift / 8 ;
    const uint32_t bitshift = shift % 8;
    tmp[i] = (num[(i+byteshift) % 16] >> bitshift)
           | ((num[(i+byteshift+1) % 16] << (8-bitshift)) & 0xff);    
  }
  memcpy(num, tmp, 16) ;
}

// add two 128bit, little endian values and store the result into the first
void n128_add(uint8_t *a, const uint8_t *b)
{
  uint8_t carry = 0 ;
  for (int i=0;i<16;i++)
  {
    uint16_t sum = a[i] + b[i] + carry ;
    a[i] = sum & 0xff ;
    carry = sum >> 8 ;
  }
}

// sub two 128bit, little endian values and store the result into the first
void n128_sub(uint8_t *a, const uint8_t *b)
{
  uint8_t carry = 0 ;
  for (int i=0;i<16;i++)
  {
    uint16_t sub = a[i] - b[i] - (carry & 1);
    a[i] = sub & 0xff ;
    carry = sub >> 8 ;
  }
}

void F_XY(uint8_t *key, const uint8_t *key_x, const uint8_t *key_y)
{
	uint8_t key_xy[16];

	for(int i=0; i<16; i++)
    key_xy[i] = key_x[i] ^ key_y[i];

  memcpy(key, keyconst, sizeof(keyconst));

	n128_add(key, key_xy);
	n128_lrot(key, 42);
}

//F_XY_reverse does the reverse of F(X^Y): takes (normal)key, and does F in reverse to generate the original X^Y key_xy.
void F_XY_reverse(const uint8_t *key, uint8_t *key_xy)
{
	memcpy(key_xy, key, 16);
	n128_rrot(key_xy, 42);
	n128_sub(key_xy, keyconst);
}

