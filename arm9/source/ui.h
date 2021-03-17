// User interface

#pragma once

#include <stdint.h>
#include <vector>

bool WaitForKonami(const char *msg) ;
void InfoBorder() ;
void WaitForPowercord() ;
void WaitForSuccessRestart() ;
void WaitForErrorRestart(char *msg) ;

void CreateProgress(const char *msg) ;
void UpdateProgress(uint8_t progress) ;
void ClearProgress() ;

uint32_t OptionSelect(const char *name, std::vector<const char *>values, uint32_t defaultIndex) ;
