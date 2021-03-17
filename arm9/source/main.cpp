#include <nds.h>
#include <stdio.h>
#include "gm9i/nandio.h"
#include <fat.h>
#include<stdarg.h>
#include<stdio.h>
#include <dirent.h>
#include <stdint.h>
#include "gm9i/crypto.h"
#include "gm9i/f_xy.h"
#include "polarssl/aes.h"
#include "twltool/dsi.h"
#include "u128_math.h"
#include "patch.h"
#include "patch_data.h"
#include "system_info.h"
#include "ui.h"

#define TARGETBUFFER 0x02900000

PrintConsole topScreen;
PrintConsole bottomScreen;
PrintConsole *currentScreen = &topScreen ;

void humanReadableByteSize(long size, char *buffer, int bufferLen)
{
	long full = abs(size), deci = 0 ;
	int exponent = 0 ;
	while ( full > 1024)
	{
		deci = (full * 10 / 1024) % 10 ;
		full = full / 1024 ;
		exponent++ ;
	}
	const char exponents[] = {' ', 'k', 'M', 'G', 'T'} ;
	snprintf(buffer, bufferLen, "%li.%u %cB", full, (u8)deci, exponents[exponent]) ;
}

typedef enum LOGLEVEL
{
	LOGLEVEL_ERROR,
	LOGLEVEL_WARNING,
	LOGLEVEL_INFO,
	LOGLEVEL_PROGRESS
} LOGLEVEL;

void Log(LOGLEVEL level, const char *format, ...)
{
  char buffer[256] ;
  consoleSelect(&topScreen);
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap) ;
	vsprintf(buffer, format, ap) ;
	va_end(ap);
  consoleSelect(&bottomScreen);

	if (level == LOGLEVEL_ERROR)
	{
		WaitForErrorRestart(buffer) ;
	}
}

typedef void (* progress_callback_t)(uint8_t percent) ;

void decrypt_modcrypt_area(dsi_context* ctx, u8 *buffer, unsigned int size, progress_callback_t callback)
{
	uint32_t len = size / 0x10;
	u8 block[0x10];

  uint32_t bytesPerPercent = (size + 1) / 101 ;
  uint8_t lastReportedProgress = (uint8_t)-1 ;
  uint32_t pos = 0 ;

	while(len>0)
	{
		memset(block, 0, 0x10);
		dsi_crypt_ctr_block(ctx, buffer, block);
		memcpy(buffer, block, 0x10);
		buffer+=0x10;
    pos += 0x10;
		len--;
    if (callback)
    {
      uint8_t progress = pos / bytesPerPercent ;
      if (lastReportedProgress != progress)
      {
        lastReportedProgress = progress ;
        callback(progress) ;
      }
    }
	}
}

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------	
	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);

	consoleInit(&topScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
	consoleInit(&bottomScreen, 3,BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);


	consoleSelect(&topScreen);  
  InfoBorder() ;
  consoleSetWindow(&topScreen, 0, 2, 32, 22) ;
	consoleSelect(&bottomScreen);  
  InfoBorder() ;
	
	for (int i=0;i<30;i++)
		swiWaitForVBlank() ;
		
	Log(LOGLEVEL_INFO, "[i] CID:\n      ") ;
	u8 *CID = (u8 *)0x2FFD7BC ;
	for (int i=0;i<8;i++)
	{
		Log(LOGLEVEL_INFO, "%02X", CID[i]) ;
	}
	Log(LOGLEVEL_INFO, "\n      ") ;
	for (int i=8;i<16;i++)
	{
		Log(LOGLEVEL_INFO, "%02X", CID[i]) ;
	}
	Log(LOGLEVEL_INFO, "\n") ;
	
	Log(LOGLEVEL_INFO, "[i] ConsoleID:\n      ") ;
	u8 consoleID[8] ;
	getConsoleID(consoleID) ;
	for (int i=0;i<8;i++)
	{
		Log(LOGLEVEL_INFO, "%02X", consoleID[7-i]) ;
	}
	Log(LOGLEVEL_INFO, "\n") ;
	
	if(consoleID[7] != 0x08)
	{
		Log(LOGLEVEL_ERROR, "[E] Invalid ConsoleID found!\n");
	}

	if (!fatMountSimple("nand", &io_dsi_nand))
	{
		Log(LOGLEVEL_ERROR, "[E] Could not mount NAND\n");
	}
	
	long nandSize = 0;
	struct statvfs st;
	if (statvfs("nand:/", &st) == 0) {
		nandSize = st.f_bsize * st.f_blocks;
	}

	char buffer[20] ;

	humanReadableByteSize(nandSize, buffer, sizeof(buffer)) ;
	Log(LOGLEVEL_INFO, "[i] NAND Size: %s\n", buffer);
	
	uint8_t launcherRegion = 0 ;
	
	char * appLauncherDirName = system_getLauncherPath(&launcherRegion);
	
	if (!appLauncherDirName)
	{
		Log(LOGLEVEL_ERROR, "[E] Could not find folder\n");
	}
	
	Log(LOGLEVEL_INFO, "[i] Launcher Region:\n      %s\n",  knownRegions[launcherRegion].name);
	if ( knownRegions[launcherRegion].code > 3)
	{
		Log(LOGLEVEL_ERROR, "[E] Launcher is KOR or CHN\n");
	}	
	
	char * appFileName = system_getAppFilename(appLauncherDirName) ;
  char * tmdFileName = system_getTmdFilename(appLauncherDirName) ;
  
  
	bool unlaunchInstalled = false ;
	
  // If that file is longer than 1k, unlaunch is appended
  // Todo: get version of unlaunch
  struct stat tmdInfo ;
  memset(&tmdInfo, 0, sizeof(tmdInfo)) ;	
  stat(tmdFileName, &tmdInfo) ;
  free(tmdFileName) ;
  unlaunchInstalled = (tmdInfo.st_size > 1024) ;
  
  
  if (unlaunchInstalled)
  {
    Log(LOGLEVEL_INFO, "[i] Unlaunch is installed\n") ;
  } else
  {
    Log(LOGLEVEL_INFO, "[i] Unlaunch is not installed\n") ;
  }

	if (!appFileName)
	{
		Log(LOGLEVEL_ERROR, "[E] Could not find app\n");
	}
	
  uint8_t *target = (uint8_t *)TARGETBUFFER ;
  
  CreateProgress("Reading Launcher") ;
	uint32_t appLauncherSize = system_readFile(target, appFileName, &UpdateProgress) ;
  ClearProgress() ;
	
	humanReadableByteSize(appLauncherSize, buffer, sizeof(buffer)) ;
	Log(LOGLEVEL_INFO, "[i] Launcher Size: %s\n", buffer);	


	if (!appLauncherSize)
	{
		Log(LOGLEVEL_ERROR, "[E] Could not read launcher\n");
	}
  
	if (target[0x01C] & 2)
	{
    CreateProgress("Processing modcrypt #1") ;

		u8 key[16] = {0} ;
		u8 keyp[16] = {0} ;
		if (target[0x01C] & 4)
		{
			// Debug Key
			memcpy(key, target, 16) ;
		} else
		{
			//Retail key
			char modcrypt_shared_key[8] = {'N','i','n','t','e','n','d','o'};
			memcpy(keyp, modcrypt_shared_key, 8) ;
			for (int i=0;i<4;i++)
			{
				keyp[8+i] = target[0x0c+i] ;
				keyp[15-i] = target[0x0c+i] ;
			}
			memcpy(key, target+0x350, 16) ;
			
			u128_xor(key, keyp);
			u128_add(key, DSi_KEY_MAGIC);
      u128_lrot(key, 42) ;
		}
		uint32_t modcryptOffsets[2], modcryptLengths[2] ;
		modcryptOffsets[0] = ((uint32_t *)(target+0x220))[0] ;
		modcryptOffsets[1] = ((uint32_t *)(target+0x220))[2] ;
		modcryptLengths[0] = ((uint32_t *)(target+0x220))[1] ;
		modcryptLengths[1] = ((uint32_t *)(target+0x220))[3] ;

		uint32_t rk[4];
		memcpy(rk, key, 16) ;
		
		dsi_context ctx;
		dsi_set_key(&ctx, key);
		dsi_set_ctr(&ctx, &target[0x300]);
		if (modcryptLengths[0])
		{
			decrypt_modcrypt_area(&ctx, target+modcryptOffsets[0], modcryptLengths[0], &UpdateProgress);
		}
    ClearProgress() ;

    CreateProgress("Processing modcrypt #2") ;
		dsi_set_key(&ctx, key);
		dsi_set_ctr(&ctx, &target[0x314]);
		if (modcryptLengths[1])
		{
			decrypt_modcrypt_area(&ctx, target+modcryptOffsets[1], modcryptLengths[1], &UpdateProgress);
		}

		for (int i=0;i<4;i++)
		{
			((uint32_t *)(target+0x220))[i] = 0;
		}
    
    ClearProgress() ;
	}

  SPATCHRESULT patchResults[] =
  {
    {0, 0},
    {0, 0},
    {0, 0}
  };  
  
  const uint32_t patchCount = sizeof(patchList) / sizeof(SPATCHLISTENTRY) ;

  CreateProgress("Applying patches") ;
  patch_applyPatternPatches(target, appLauncherSize,
                            patchList, patchCount, patchResults, &UpdateProgress) ;
  ClearProgress() ;

  for (uint32_t i=0;i<patchCount;i++)
  {
    if (patchResults[i].matchCount == 0)
    {
      Log(LOGLEVEL_ERROR, "[E] Pattern %s not found\n", patchList[i].name) ;
    }
    if (patchResults[i].matchCount > 1)
    {
      Log(LOGLEVEL_ERROR, "[E] Pattern %s too often\n", patchList[i].name) ;
    }
    Log(LOGLEVEL_INFO, "[i] Patch \'%s\' found\n", patchList[i].name) ;
  }
  
  Log(LOGLEVEL_PROGRESS, "[-] getting options\n") ;
  std::vector<SOPTIONSELECT> 
    options = patch_getAvailableOptions(patchList, patchCount) ;
    
  for (uint32_t i = 0;i<options.size();i++)
  {
    options[i].selection = OptionSelect(options[i].name, options[i].values, options[i].selection) ;
  }
  
  patch_applySelectedOptions(target, appLauncherSize,
                                patchList, patchResults, patchCount, 
                                options) ;              
  
	WaitForPowercord() ;
	
  if (!WaitForKonami("Write to internal NAND\n"
                "     CAUTION! Risk to brick!"))
  {
    Log(LOGLEVEL_ERROR, "[E] Failed to enter code\n") ;
  }

  // we will write the file back to NAND at root
  CreateProgress("Writing to NAND") ;
  uint32_t written = system_writeFile((uint8_t *)TARGETBUFFER, appLauncherSize, "nand:/launcher.dsi", &UpdateProgress) ;
  ClearProgress() ;
  
  if (!written)
  {
    Log(LOGLEVEL_ERROR, "[E] Create file failed\n    You can turn off now\n") ;
  }
  if (written < appLauncherSize)
  {
    Log(LOGLEVEL_ERROR, "[E] Write file failed\n    You can turn off now\n") ;
  }

  Log(LOGLEVEL_PROGRESS, "[-] Unmounting\n") ;
  fatUnmount("nand:") ;
  Log(LOGLEVEL_PROGRESS, "[-] Merging stages\n");
  nandio_shutdown() ;			
  
  WaitForSuccessRestart() ;
  while(true) 
    ;

}
