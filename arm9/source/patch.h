// Patch

#pragma once

#include <stdint.h>

/************************ Constants / Defines *********************************/


/************************ Structures / Datatypes ******************************/

typedef struct SPATCHLISTENTRY
{
	const uint8_t *pattern ;
	const uint8_t *patch ;
	const uint16_t size ;
	const char *name ;
} SPATCHLISTENTRY ;

typedef struct SPATCHRESULT
{
  uint32_t lastFoundOffset ;
  uint32_t matchCount ;
} SPATCHRESULT ;

/************************ Function Protoypes **********************************/

void patch_applyPatternPatches(uint8_t *target, uint32_t size, 
                               const SPATCHLISTENTRY *patches, const uint32_t count,
                               SPATCHRESULT *result) ;