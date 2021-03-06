#include <nds.h>
#include <stdio.h>
#include "nandio.h"
#include <fat.h>
#include<stdarg.h>
#include<stdio.h>
#include <dirent.h>
#include <stdint.h>
#include "aes.h"
#include "crypto.h"
#include "f_xy.h"
#include "aes.h"
#include "dsi.h"

#define TARGETBUFFER 0x02900000

#define VERSION_STRING "ver. 2.0rc1"

PrintConsole topScreen;
PrintConsole bottomScreen;

const u8 patternLangMaskPatch[] =
	{ 0x01, 0x48, 0x00, 0x68, 0x70, 0x47, 0xC0, 0x46, 0x68, 0xFD, 0xFF, 0x02 } ;
const u8 patchLangMaskPatch[] =
	{ 0x01, 0x4B, 0x7F, 0x20, 0x18, 0x60, 0x70, 0x47, 0x68, 0xFD, 0xFF, 0x02 } ;
const u8 patternFilterPatch[] = 
  { 0x02, 0x98, 0x08, 0x42, 0x00, 0xD1, 0x00, 0x22, 0x00, 0x2A} ;
const u8 patchFilterPatch[] = 
  { 0x02, 0x98, 0x08, 0x42, 0xc0, 0x46, 0x01, 0x22, 0x00, 0x2A} ;
const u8 patternRegionPatch[] = 
  { 0x01, 0x48, 0x00, 0x7a, 0x70, 0x47, 0xc0, 0x46, 0x68, 0xfd, 0xff, 0x02} ;
const u8 patchRegionPatch[] = 
  { 0x01, 0x4B, 0x02, 0x20, 0x18, 0x72, 0x70, 0x47, 0x68, 0xfd, 0xff, 0x02} ;	
//             /    \
//            /      \
//           /        \
//          Target Region

	
typedef struct SPATCHLISTENTRY
{
	const u8 *pattern ;
	const u8 *patch ;
	const u16 size ;
	const char *name ;
} SPATCHLISTENTRY ;

const SPATCHLISTENTRY patchList[] =
{
	{ patternLangMaskPatch, patchLangMaskPatch, sizeof(patternLangMaskPatch), "Language Mask"},
	{ patternFilterPatch, patchFilterPatch, sizeof(patternFilterPatch), "App Filter"},
	{ patternRegionPatch, patchRegionPatch, sizeof(patternRegionPatch), "Region Info"},
	{ 0, 0, 0, ""}
} ;

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

struct SREGIONINFO
{
	u8 						code;
	char 					folderKey;
	const char *	name;
} ;

struct SREGIONINFO knownRegions[] = 
{
	{0, 'J', "Japan"},
	{1, 'E', "N.America"},
	{2, 'P', "Europe"},
	{3, 'U', "Australia"},
	{4, 'C', "China"},
	{5, 'K', "Korea"}
} ;

typedef enum LOGLEVEL
{
	LOGLEVEL_ERROR,
	LOGLEVEL_WARNING,
	LOGLEVEL_INFO,
	LOGLEVEL_PROGRESS
} LOGLEVEL;

void Log(LOGLEVEL level, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap) ;
	va_end(ap);
	if (consoleGetDefault()->cursorY >= 21)
	{
		if (consoleGetDefault() == &topScreen)
		{
			consoleSelect(&bottomScreen);
		} else
		{
			consoleSelect(&topScreen);
		}
		consoleClear() ;
	}
	if (level == LOGLEVEL_ERROR)
	{
		while (1) 
			;
	}
}

#define min(a,b) (((a)<(b))?(a):(b))

void decrypt_modcrypt_area(dsi_context* ctx, u8 *buffer, unsigned int size)
{
	uint32_t len = size / 0x10;
	u8 block[0x10];
	while(len>0)
	{
		memset(block, 0, 0x10);
		dsi_crypt_ctr_block(ctx, buffer, block);
		memcpy(buffer, block, 0x10);
		buffer+=0x10;
		len--;
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

	Log(LOGLEVEL_INFO, "Language Patcher\n");
	Log(LOGLEVEL_INFO, VERSION_STRING);
	Log(LOGLEVEL_INFO, "\n================\n");
	
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

	Log(LOGLEVEL_PROGRESS, "[-] Mounting NAND\n") ;
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
	
	Log(LOGLEVEL_PROGRESS, "[-] Locating Launcher\n") ;
	DIR * appLauncherDir = 0 ;
	int launcherRegion = -1 ;
	
	char * appLauncherDirName = "";
	
	for (int i=0;i<sizeof(knownRegions) / sizeof(*knownRegions);i++)
	{
	// nand:/title/00030017/484e41??/content
		static char folderToCheck[260] = {0};
		siprintf(folderToCheck, "nand:title/00030017/484e41%02x/content", knownRegions[i].folderKey) ;
		appLauncherDir = opendir(folderToCheck) ;
		if (appLauncherDir)
		{
			appLauncherDirName = strdup(folderToCheck) ;
			launcherRegion = i ;
			break;
		}
	}
	
	if (!appLauncherDir)
	{
		Log(LOGLEVEL_ERROR, "[E] Could not enter folder\n");
	}
	
	Log(LOGLEVEL_INFO, "[i] Launcher Region:\n      %s\n",  knownRegions[launcherRegion].name);
	if ( knownRegions[launcherRegion].code != 0)
	{
		Log(LOGLEVEL_ERROR, "[E] Launcher is not japanese\n");
	}	
	
	char * appFileName = 0 ;
	bool unlaunchInstalled = false ;
	struct dirent *fileInfo;
	while ((fileInfo=readdir(appLauncherDir))!=NULL) 
	{
		if (strcmp(fileInfo->d_name + strlen(fileInfo->d_name) - 4, ".app") == 0)
		{
			appFileName = (char *)malloc(260) ;
			strcpy(appFileName,appLauncherDirName) ;
			strcat(appFileName,"/") ;
			strcat(appFileName,fileInfo->d_name) ;
		} else if (strcmp(fileInfo->d_name + strlen(fileInfo->d_name) - 4, ".tmd") == 0)
		{
			// ToDo: If that file is longer than 1k, unlaunch is appended
		}
	}
	if (!appFileName)
	{
		Log(LOGLEVEL_ERROR, "[E] Could not find app\n");
	}
	
	FILE *f = fopen(appFileName, "rb") ;
	fseek(f, 0, SEEK_END) ;
	long appLauncherSize = ftell(f) ;
	fseek(f, 0, SEEK_SET) ;
	
	
	humanReadableByteSize(appLauncherSize, buffer, sizeof(buffer)) ;
	Log(LOGLEVEL_INFO, "[i] Launcher Size: %s\n", buffer);	


	Log(LOGLEVEL_PROGRESS, "[-] Reading launcher\n") ;
	long pos = 0 ;
	u8 *target = (u8 *)TARGETBUFFER ;
	while (pos < appLauncherSize)
	{
		long read = fread(target, 1, min(512, (appLauncherSize-pos)), f) ;
		pos += read ;
		target += read ;
	}
	fclose(f) ;
	
	if (pos != appLauncherSize)
	{
		Log(LOGLEVEL_ERROR, "[E] Could not read launcher\n");
	}
	Log(LOGLEVEL_PROGRESS, "[-] Reading done\n") ;
	
	target = (u8 *)TARGETBUFFER ;
	if (target[0x01C] & 2)
	{
		u8 key[16] = {0} ;
		u8 keyp[2*16] = {0} ;
		Log(LOGLEVEL_PROGRESS, "[-] Undo modcrypt\n") ;
		if (target[0x01C] & 4)
		{
			// Debug Key
			Log(LOGLEVEL_PROGRESS, "[-] Debug key\n") ;
			memcpy(key, target, 16) ;
		} else
		{
			//Retail key
			Log(LOGLEVEL_PROGRESS, "[-] Retail key\n") ;
			char modcrypt_shared_key[8] = {'N','i','n','t','e','n','d','o'};
			memcpy(keyp, modcrypt_shared_key, 8) ;
			for (int i=0;i<4;i++)
			{
				keyp[8+i] = target[0x0c+i] ;
				keyp[15-i] = target[0x0c+i] ;
			}
			memcpy(keyp+16, target+0x350, 16) ;
			
				// Key = ((Key_X XOR Key_Y) + FFFEFB4E295902582A680F5F1A4F3E79h) ROL 42
			// equivalent to F_XY in twltool/f_xy.c
			const uint32_t DSi_KEY_MAGIC[4] =
				{0x1a4f3e79u, 0x2a680f5fu, 0x29590258u, 0xfffefb4eu};			
			xor_128((uint32_t *)key, (uint32_t *)keyp, (uint32_t *)(keyp+16));
			add_128((uint32_t *)key, DSi_KEY_MAGIC);
			rol42_128((uint32_t *)key);	
		}
		uint32_t modcryptOffsets[2], modcryptLengths[2] ;
		modcryptOffsets[0] = ((uint32_t *)(target+0x220))[0] ;
		modcryptOffsets[1] = ((uint32_t *)(target+0x220))[2] ;
		modcryptLengths[0] = ((uint32_t *)(target+0x220))[1] ;
		modcryptLengths[1] = ((uint32_t *)(target+0x220))[3] ;
		Log(LOGLEVEL_INFO, "[i] Region at %04xh\n", modcryptOffsets[0]) ;
		Log(LOGLEVEL_INFO, "      size %04xh\n", modcryptLengths[0]) ;
		Log(LOGLEVEL_INFO, "[i] Region at %04xh\n", modcryptOffsets[1]) ;
		Log(LOGLEVEL_INFO, "      size %04xh\n", modcryptLengths[1]) ;


		uint32_t rk[4];
		memcpy(rk, key, 16) ;
		
		dsi_context ctx;
		dsi_set_key(&ctx, key);
		dsi_set_ctr(&ctx, &target[0x300]);
		if (modcryptLengths[0])
		{
			decrypt_modcrypt_area(&ctx, target+modcryptOffsets[0], modcryptLengths[0]);
		}
		dsi_set_key(&ctx, key);
		dsi_set_ctr(&ctx, &target[0x314]);
		if (modcryptLengths[1])
		{
			decrypt_modcrypt_area(&ctx, target+modcryptOffsets[1], modcryptLengths[1]);
		}

		Log(LOGLEVEL_PROGRESS, "[-] fixing regions\n") ;
		for (int i=0;i<4;i++)
		{
			((uint32_t *)(target+0x220))[i] = 0;
		}
		Log(LOGLEVEL_PROGRESS, "[-] modcrypt done\n") ;
	}

//	consoleSelect(&bottomScreen);

	Log(LOGLEVEL_PROGRESS, "[-] patching ...\n") ;

	u8 patchFlags = 0 ;
	/* now search for the bytes to patch */
	for (uint32_t i=0;i<appLauncherSize;i++)
	{
		uint32_t patch = 0 ;
		while (patchList[patch].size != 0)
		{
			if (memcmp(target+i, patchList[patch].pattern, patchList[patch].size) == 0)
			{
				Log(LOGLEVEL_INFO, "[i] Patch \'%s\' found\n", patchList[patch].name) ;
				if ( patchFlags & (1 << patch))
				{
					Log(LOGLEVEL_ERROR, "[E] Multiple instances found\n") ;
				}
				memcpy(target+i, patchList[patch].patch, patchList[patch].size) ;
				patchFlags |= (1 << patch) ;
			}
			patch++ ;
		}		
	}
	if (patchFlags != 0x07)
	{
		Log(LOGLEVEL_ERROR, "[E] Patches not found\n") ;
	}
	
	Log(LOGLEVEL_PROGRESS, "[-] patching done in RAM\n") ;
	
	Log(LOGLEVEL_WARNING, "\n[!] Please plug in") ;
	Log(LOGLEVEL_WARNING, "\n      power cord\n");
	
	while(1) {
		swiWaitForVBlank();
		scanKeys();
		if (getBatteryLevel() & 0x80)
			break;
	}	
	
	Log(LOGLEVEL_INFO, "\n==================");
	Log(LOGLEVEL_INFO, "\n Hit [A] to apply ");
	Log(LOGLEVEL_INFO, "\n    !CAUTION!     ");
	Log(LOGLEVEL_INFO, "\n  Risk to brick!  ");
	Log(LOGLEVEL_INFO, "\n==================\n\n");

	while(1) {
		swiWaitForVBlank();
		scanKeys();
		if (keysDown() & KEY_A)
		{
			// we will write the file back to NAND at root
			Log(LOGLEVEL_PROGRESS, "[-] writing ...\n") ;
#if 1
			pos = 0 ;
			target = (u8 *)TARGETBUFFER ;
			f = fopen("nand:/launcher.dsi", "wb+") ;
			if (!f)
			{
				Log(LOGLEVEL_ERROR, "[E] Create file failed\n    You can turn off now\n") ;
			}
			while (pos < appLauncherSize)
			{
				long written = fwrite(target, 1, min(256, (appLauncherSize-pos)), f) ;
				pos += written ;
				target += written ;
			}
#endif
			fclose(f) ;
			Log(LOGLEVEL_PROGRESS, "[-] Unmounting\n") ;
			fatUnmount("nand:") ;
			Log(LOGLEVEL_PROGRESS, "[-] Merging stages\n");
			nandio_shutdown() ;			
			Log(LOGLEVEL_INFO, "[i] ALL DONE\n") ;
			Log(LOGLEVEL_INFO, "    You can turn off now\n") ;
			while(1) ;
		}
	}

	return 0;
}
