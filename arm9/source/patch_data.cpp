// Patch Data

#include "patch_data.h"
#include <stdint.h>

const uint8_t patternLangMaskPatch[] =
	{ 0x01, 0x48, 0x00, 0x68, 0x70, 0x47, 0xC0, 0x46, 0x68, 0xFD, 0xFF, 0x02 } ;
const uint8_t patchLangMaskPatch[] =
	{ 0x01, 0x4B, 0x7F, 0x20, 0x18, 0x60, 0x70, 0x47, 0x68, 0xFD, 0xFF, 0x02 } ;
/*             /    \
              /      \
             /        \
            Language Mask
*/  
const uint8_t patternFilterPatch[] = 
  { 0x02, 0x98, 0x08, 0x42, 0x00, 0xD1, 0x00, 0x22, 0x00, 0x2A} ;
const uint8_t patchFilterPatch[] = 
  { 0x02, 0x98, 0x08, 0x42, 0xc0, 0x46, 0x01, 0x22, 0x00, 0x2A} ;
const uint8_t patternRegionPatch[] = 
  { 0x01, 0x48, 0x00, 0x7a, 0x70, 0x47, 0xc0, 0x46, 0x68, 0xfd, 0xff, 0x02} ;
const uint8_t patchRegionPatch[] = 
  { 0x01, 0x4B, 0x02, 0x20, 0x18, 0x72, 0x70, 0x47, 0x68, 0xfd, 0xff, 0x02} ;	
/*             /    \
              /      \
             /        \
            Target Region
*/

const uint8_t regionJPN[] = { 0x00 } ;
const uint8_t regionNA[] = { 0x01 } ;
const uint8_t regionEUR[] = { 0x02 } ;
const uint8_t regionA[] = { 0x03 } ;

const SPATCHOPTION patchOptionsRegions[] = 
{
  { "Region", "Japan", OPTION, 2, 1, regionJPN },
  { "Region", "North America", OPTION, 2, 1, regionNA },
  { "Region", "Europe", DEFAULT_OPTION, 2, 1, regionEUR },
  { "Region", "Australia", OPTION, 2, 1, regionA }
} ;

const SPATCHLISTENTRY patchList[] =
{
	{ patternLangMaskPatch, patchLangMaskPatch, sizeof(patternLangMaskPatch), "Language Mask", 0, 0},
	{ patternFilterPatch, patchFilterPatch, sizeof(patternFilterPatch), "App Filter", 0, 0},
	{ patternRegionPatch, patchRegionPatch, sizeof(patternRegionPatch), "Region Info", patchOptionsRegions, sizeof(patchOptionsRegions) / sizeof(SPATCHOPTION)}
} ;

SPATCHRESULT patchResults[] =
{
  {0, 0},
  {0, 0},
  {0, 0}
};
