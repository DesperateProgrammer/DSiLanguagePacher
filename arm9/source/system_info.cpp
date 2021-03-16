#include "system_info.h"

#include <stdio.h>
#include <algorithm>
#include <stdint.h>
#include <dirent.h>
#include <string.h>
#include <malloc.h>

const struct SREGIONINFO knownRegions[6] = 
{
	{0, 'J', "Japan"},
	{1, 'E', "N.America"},
	{2, 'P', "Europe"},
	{3, 'U', "Australia"},
	{4, 'C', "China"},
	{5, 'K', "Korea"}
} ;

char *system_getLauncherPath(uint8_t *region)
{
  char *appLauncherDirName = 0 ;
  DIR *appLauncherDir = 0;
	for (uint32_t i=0;i<sizeof(knownRegions) / sizeof(*knownRegions);i++)
	{
	// nand:/title/00030017/484e41??/content
		static char folderToCheck[260] = {0};
		siprintf(folderToCheck, "nand:title/00030017/484e41%02x/content", knownRegions[i].folderKey) ;
		appLauncherDir = opendir(folderToCheck) ;
		if (appLauncherDir)
		{
			appLauncherDirName = strdup(folderToCheck) ;
      if (region)
        *region = i ;
			break;
		}
	}
  return appLauncherDirName ;
}

char *system_getAppFilename(const char *dirName) 
{
  char * appFileName = 0 ;
	struct dirent *fileInfo;
  DIR *appDir = opendir(dirName) ;
	while ((fileInfo=readdir(appDir))!=NULL) 
	{
		if (strcmp(fileInfo->d_name + strlen(fileInfo->d_name) - 4, ".app") == 0)
		{
			appFileName = (char *)malloc(strlen(dirName)+1+strlen(fileInfo->d_name)+1) ;
			strcpy(appFileName,dirName) ;
			strcat(appFileName,"/") ;
			strcat(appFileName,fileInfo->d_name) ;
		} 
	}
  return appFileName ;
}

char *system_getTmdFilename(const char *dirName) 
{
  char * appFileName = 0 ;
	struct dirent *fileInfo;
  DIR *appDir = opendir(dirName) ;
	while ((fileInfo=readdir(appDir))!=NULL) 
	{
		if (strcmp(fileInfo->d_name + strlen(fileInfo->d_name) - 4, ".tmd") == 0)
		{
			appFileName = (char *)malloc(strlen(dirName)+1+strlen(fileInfo->d_name)+1) ;
			strcpy(appFileName,dirName) ;
			strcat(appFileName,"/") ;
			strcat(appFileName,fileInfo->d_name) ;
		} 
	}
  return appFileName ;
}

uint32_t system_readFile(uint8_t *buffer, const char *filename)
{
	FILE *f = fopen(filename, "rb") ;
  if (!f)
    return 0;
	fseek(f, 0, SEEK_END) ;
	uint32_t fileSize = ftell(f) ;
	fseek(f, 0, SEEK_SET) ;
	uint32_t pos = 0 ;
	uint8_t *target = buffer ;
	while (pos < fileSize)
	{
		uint32_t read = fread(target, 1, std::min<uint32_t>(512, (fileSize-pos)), f) ;
		pos += read ;
		target += read ;
	}
	fclose(f) ;
  return pos ;
}

uint32_t system_writeFile(uint8_t *buffer, uint32_t size, const char *filename)
{
  uint32_t pos = 0 ;
  FILE *f = fopen(filename, "wb+") ;
  if (!f)
  {
    return 0;
  }
  while (pos < size)
  {
    uint32_t written = fwrite(buffer, 1, std::min<uint32_t>(256, (size-pos)), f) ;
    pos += written ;
    buffer += written ;
  }
  fclose(f) ;
  return pos;
}
