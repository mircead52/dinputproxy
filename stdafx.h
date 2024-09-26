
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <commctrl.h>
#include <basetsd.h>
#include <mmsystem.h>

#include <stdio.h>



typedef unsigned int uint_t;

#define FREE(X) if(X) { free(X); X = NULL; }

#if defined _M_AMD64
#define idx64 1
#endif

extern HINSTANCE dllHandle;
extern HANDLE log_file;

void LogInfo(const char *format, ...);
void LogInfo_static(const char *format, ...);
void LogInfoW(const wchar_t *format, ...);

#ifdef _UNICODE
#define LOGINFOTC(F, ...) LogInfoW(_T(F), __VA_ARGS__)
#else
#define LOGINFOTC LogInfo
#endif

BOOL IsWow64(void);

uint_t wchartoascii(char *dest, uint_t dest_sz, const wchar_t *src);