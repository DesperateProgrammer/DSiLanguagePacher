/* f_xy.h
 *
 * This file was imported from godmode9i, but it is liely not the
 * original source. twltool uses the same file.
 *
 * If you happen to know whom to credit I'd love to add the name
 *
 * Refactored to reduce the pointer casts and remove the dependency 
 * from tonccpy.
 */
 
#ifndef _H_F_XY
#define _H_F_XY

#ifdef __cplusplus
extern "C" {
#endif

/************************ Function Protoypes **********************************/

void F_XY(uint8_t *key, const uint8_t *key_x, const uint8_t *key_y);
void F_XY_reverse(const uint8_t *key, uint8_t *key_xy);

#ifdef __cplusplus
}
#endif

#endif

