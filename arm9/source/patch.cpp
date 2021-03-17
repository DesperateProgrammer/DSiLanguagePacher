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
 
static uint32_t patch_findOption(std::vector<SOPTIONSELECT> &options, const char *name)
{
  for (uint32_t i=0;i<options.size();i++)
  {
    if (strcmp(name, options[i].name) == 0)
    {
      return i ;
    }
  }
  return (uint32_t)-1;
}


static uint32_t patch_addOrFindOption(std::vector<SOPTIONSELECT> &options, const char *name)
{
  for (uint32_t i=0;i<options.size();i++)
  {
    if (strcmp(name, options[i].name) == 0)
    {
      return i ;
    }
  }
  SOPTIONSELECT entry ;
  entry.name = name ;
  entry.selection = 0 ;
  options.push_back(entry) ;
  return patch_findOption(options, name) ;
}

static uint32_t patch_addorFindOptionValue(std::vector<const char *> &values, const char *value)
{
  for (uint32_t i=0;i<values.size();i++)
  {
    if (strcmp(value, values[i]) == 0)
    {
      return i ;
    }
  }
  values.push_back(value) ;
  return patch_addorFindOptionValue(values, value) ;
}
 
std::vector<SOPTIONSELECT> patch_getAvailableOptions(const SPATCHLISTENTRY *patches, 
                                                const uint32_t count) 
{
  std::vector<SOPTIONSELECT> options ;
  for (uint32_t patch = 0; patch < count; patch++)
  {
    for (uint32_t optionNum = 0; optionNum < patches[patch].optionCount; optionNum++)
    {
      uint32_t optionIndex = patch_addOrFindOption(options, patches[patch].options[optionNum].name) ;
      uint32_t valueIndex = patch_addorFindOptionValue(options[optionIndex].values, patches[patch].options[optionNum].value) ;
      if (patches[patch].options[optionNum].asDefault)
      {
        options[optionIndex].selection = valueIndex ;
      }
    }
  }  
  return options;
}

void patch_applySelectedOptions(uint8_t *target, uint32_t size, 
                                const SPATCHLISTENTRY *patches, const SPATCHRESULT *patchResult,
                                const uint32_t count, std::vector<SOPTIONSELECT> selectedOptions)
{
  std::vector<SOPTIONSELECT> options ;
  for (uint32_t patch = 0; patch < count; patch++)
  {
    for (uint32_t optionNum = 0; optionNum < patches[patch].optionCount; optionNum++)
    {
      uint32_t optionIndex = patch_findOption(selectedOptions, patches[patch].options[optionNum].name) ;
      if (optionIndex == (uint32_t)-1)
        continue ;
      if (strcmp(patches[patch].options[optionNum].value, selectedOptions[optionIndex].values[selectedOptions[optionIndex].selection]) == 0)
      {
        // this is the selected option
        if (patchResult[patch].matchCount)
        {
          memcpy(target + patchResult[patch].lastFoundOffset + patches[patch].options[optionNum].inPatchOffset, 
                  patches[patch].options[optionNum].inPatchChange, patches[patch].options[optionNum].inPatchSize) ;
        }
      }
    }
  }  
}
