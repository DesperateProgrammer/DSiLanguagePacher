// system_info

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct SREGIONINFO
{
	uint8_t				code;
	char 					folderKey;
	const char *	name;
} ;

extern const struct SREGIONINFO knownRegions[6];

char *system_getLauncherPath(uint8_t *region);
char *system_getAppFilename(const char *dirName) ;
char *system_getTmdFilename(const char *dirName) ;

uint32_t system_readFile(uint8_t *buffer, const char *filename);
uint32_t system_writeFile(uint8_t *buffer, uint32_t size, const char *filename);

#ifdef __cplusplus
}
#endif