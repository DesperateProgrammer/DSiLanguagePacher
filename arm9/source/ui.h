// User interface

#pragma once

#include <stdint.h>


bool WaitForKonami(const char *msg) ;
void InfoBorder() ;
void WaitForPowercord() ;
void WaitForSuccessRestart() ;
void WaitForErrorRestart(char *msg) ;

void CreateProgress(const char *msg) ;
void UpdateProgress(uint8_t progress) ;
void ClearProgress() ;
