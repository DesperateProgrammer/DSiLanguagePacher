#include "patch.h"
#include <string.h>

void patch_applyPatternPatches(uint8_t *target, uint32_t size, 
                               const SPATCHLISTENTRY *patches, const uint32_t count,
                               SPATCHRESULT *result) 
 {
 	for (uint32_t i=0;i<size;i++)
	{
		for (uint32_t patch = 0; patch < count; patch++)
		{
      // check if the pattern could still be contained in the reminder
      if (patches[patch].size + i > size)
        continue ;
      // check if the pattern does match at this position
			if (memcmp(target+i, patches[patch].pattern, patches[patch].size) == 0)
			{
        // apply the patch!
				memcpy(target+i, patches[patch].patch, patches[patch].size) ;
        // write back result
        result[patch].matchCount++ ;
        result[patch].lastFoundOffset = i ;
			}
		}		
	}
 }