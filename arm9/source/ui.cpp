// User interface 

#include "ui.h"
#include <nds.h>
#include <stdio.h>

const uint16_t konamiSequence[] = 
{
  KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT, KEY_B, KEY_A
} ;

static void SetCursorPos(uint8_t x, uint8_t y)
{
  iprintf("\x1b[%d;%dH", y, x); 
}

static void SetTextColor(uint8_t color)
{
  iprintf("\x1b[%dm", color) ;
}

void InfoBorder()
{
  iprintf("DSi Language Patcher     v2.0rc4") ;
  iprintf("--------------------------------") ; 
}

uint32_t OptionSelect(const char *name, std::vector<const char *>values, uint32_t defaultIndex)
{
  consoleClear() ;
  InfoBorder() ;
  uint32_t selected = defaultIndex ;
  while (true)
  {
    swiWaitForVBlank();
    scanKeys() ;
    uint16_t keys_down = keysDown() ;
    SetCursorPos(5,5) ;
    SetTextColor(39) ;
    iprintf("Select %s:", name) ;
    for (uint32_t i=0;i<values.size();i++)
    {
      SetTextColor(i == selected?32:39) ;
      SetCursorPos(5, 7+i) ;
      iprintf(" - %s", values[i]) ;
    }
    if (keys_down & KEY_UP)
    {
      if (selected == 0)
        selected = values.size()-1 ;
      else
        selected-- ;        
    }
    if (keys_down & KEY_DOWN)
    {
      if (selected == values.size()-1)
        selected = 0 ;
      else
        selected++ ;        
    }
    if (keys_down & KEY_A)
    {
      SetTextColor(39) ;
      consoleClear() ;
      InfoBorder() ;
      return selected ;
    }    
  }
}


void CreateProgress(const char *msg)
{
  consoleClear() ;
  InfoBorder() ;
  SetCursorPos(6,5) ;
  iprintf("%s", msg) ;
  SetCursorPos(0,10) ;
  iprintf(
    "      --------------------      "
    "     |                    |     "
    "      --------------------      "
  ) ;
}

void UpdateProgress(uint8_t progress)
{
  char buffer[64] ;
  snprintf(buffer, 64, "     |\x1b[32m                    \x1b[39m|     ") ;
  uint8_t steps = progress / 5 ;
  for (int i=0;i<steps;i++)
  {
    if (buffer[i + 6 + 5] == ' ')
      buffer[i + 6 + 5] = '#' ;
  }
  SetCursorPos(0,11) ;
  swiWaitForVBlank(); 
  iprintf("%s", buffer) ;
}

void ClearProgress() 
{
  SetTextColor(39) ;
  consoleClear() ;
  InfoBorder() ;
}

void WaitForPowercord()
{
  consoleClear() ;
  InfoBorder() ;
  while (true)
  {
    if (getBatteryLevel() & 0x80)
    {
      consoleClear() ;
      InfoBorder() ;
      break ;
    }
    swiWaitForVBlank();
    scanKeys() ;
    SetCursorPos(6,5) ;
    iprintf("Please plug in power") ;
    SetCursorPos(0,10) ;
    SetTextColor(33) ;
    iprintf(
      "   ###         ----             "
      "   |          |    \\            "
      "   O      ====|     \\           "
      "   | <<       | AC ~ ======     "
      "   O      ====|     /           "
      "   |          |    /            "
      "   ###         ----             "
    );
    SetTextColor(39) ;
  }
}

bool WaitForKonami(const char *msg)
{
  consoleClear() ;
  InfoBorder() ;
  uint16_t progress = 0 ;
  while (true)
  {
    swiWaitForVBlank();
    scanKeys() ;
    uint16_t keys_down = keysDown() ;
    SetCursorPos(5,5) ;
    iprintf("%s", msg) ;
    
    SetCursorPos(5,11) ;
    iprintf("Enter sequence:") ;
    
    for (uint32_t i=0;i<sizeof(konamiSequence) / sizeof(konamiSequence[0]);i++)
    {
      SetCursorPos(7 + (i % 5) * 4, 13 + (i / 5) * 2) ;
      SetTextColor((progress > i)?32:39) ;
      switch (konamiSequence[i])
      {
        case KEY_UP:
          iprintf("/\\") ;
          break;
        case KEY_DOWN:
          iprintf("\\/") ;
          break;
        case KEY_LEFT:
          iprintf("<-") ;
          break;
        case KEY_RIGHT:
          iprintf("->") ;
          break;
        case KEY_A:
          iprintf("[A]") ;
          break;
        case KEY_B:
          iprintf("[B]") ;
          break;
      }
    }
    SetTextColor(39) ;
    if (keys_down == konamiSequence[progress])
    {
      progress++ ;
      if (progress >= sizeof(konamiSequence) / sizeof(konamiSequence[0]))
      {
        consoleClear() ;
        InfoBorder() ;
        return true ;
      }
    } else if (keys_down)
    {
      progress = 0 ;
    }
  }
}

void WaitForErrorRestart(char *msg)
{
  consoleClear() ;
  InfoBorder() ;
  SetCursorPos(0, 2) ;
  SetTextColor(31) ;
  iprintf(
  
    "                                "
    "    EEEE RRR  RRR   OO  RRR     "
    "    E    R  R R  R O  O R  R    "
    "    EE   RRR  RRR  O  O RRR     "
    "    E    R R  R R  O  O R R     "
    "    EEE  R  R R  R  OO  R  R    "
    "                                "
    "                                "
  ) ;
  iprintf("%s", msg) ;
  SetTextColor(39) ;
  iprintf(
    "                                "
    "                                "
    "   You can turn off your DSi    "
    "                                "
    "              or                "
    "                                "
    "    Press [START] to reboot     "
  ) ;
  while (true)
  {
    swiWaitForVBlank();
    scanKeys() ;
    uint16_t keys_down = keysDown() ;
    if (keys_down & KEY_START)
    {
      // reboot ;
      exit(0) ;
    }
  }
}

void WaitForSuccessRestart()
{
  consoleClear() ;
  InfoBorder() ;
  SetCursorPos(0, 2) ;
  SetTextColor(32) ;
  iprintf(
    "                                "
    "       OOOOO      KK   KK       "
    "      OOOOOOO     KK  KK        "
    "     OO     OO    KK KK         "
    "     OO     OO    KKKK          "
    "     OO     OO    KKK           "
    "     OO     OO    KKK           "
    "     OO     OO    KKKK          "
    "     OO     OO    KK KK         "
    "      OOOOOOO     KK  KK        "
    "       OOOOO      KK   KK       "
    "                                "
    "                                "
  ) ;
  SetTextColor(39) ;
  iprintf(
    "   You can turn off your DSi    "
    "                                "
    "              or                "
    "                                "
    "    Press [START] to reboot     "
  ) ;
  while (true)
  {
    swiWaitForVBlank();
    scanKeys() ;
    uint16_t keys_down = keysDown() ;
    if (keys_down & KEY_START)
    {
      // reboot ;
      exit(0) ;
    }
  }
}
