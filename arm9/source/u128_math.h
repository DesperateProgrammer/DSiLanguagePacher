// u128_math

#include <stdint.h>

void u128_lrot(uint8_t *num, uint32_t shift) ;
void u128_rrot(uint8_t *num, uint32_t shift) ;
void u128_xor(uint8_t *a, const uint8_t *b) ;
void u128_or(uint8_t *a, const uint8_t *b) ;
void u128_and(uint8_t *a, const uint8_t *b) ;
void u128_add(uint8_t *a, const uint8_t *b) ;
void u128_add32(uint8_t *a, const uint32_t b) ;
void u128_sub(uint8_t *a, const uint8_t *b) ;

void u128_swap(uint8_t *out, const uint8_t *in) ;