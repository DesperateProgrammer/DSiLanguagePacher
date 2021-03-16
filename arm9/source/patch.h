// Patch

#pragma once

#include <stdint.h>
#include <vector>

/************************ Constants / Defines *********************************/

#define OPTION          false
#define DEFAULT_OPTION  true

/************************ Structures / Datatypes ******************************/

typedef struct SPATCHOPTION
{
  const char *name ;
  const char *value ;
  const bool asDefault ;
  const uint32_t inPatchOffset ;
  const uint32_t inPatchSize ;
  const uint8_t *inPatchChange ;
} SPATCHOPTION ;

typedef struct SPATCHLISTENTRY
{
	const uint8_t *pattern ;
	const uint8_t *patch ;
	const uint16_t size ;
	const char *name ;
  const SPATCHOPTION *options ;
  uint32_t optionCount ;  
} SPATCHLISTENTRY ;

typedef struct SPATCHRESULT
{
  uint32_t lastFoundOffset ;
  uint32_t matchCount ;
} SPATCHRESULT ;

typedef struct SOPTIONSELECT
{
  const char * name ;
  uint32_t selection ;
  std::vector<const char *> values;
} SOPTIONSELECT ;

/************************ Function Protoypes **********************************/

void patch_applyPatternPatches(uint8_t *target, uint32_t size, 
                               const SPATCHLISTENTRY *patches, const uint32_t count,
                               SPATCHRESULT *result) ;

std::vector<SOPTIONSELECT> patch_getAvailableOptions(const SPATCHLISTENTRY *patches, 
                                                const uint32_t count) ;
