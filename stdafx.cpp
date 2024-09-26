
// stdafx.cpp : source file that includes just the standard includes
// dinputproxy.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include <wchar.h>
#include <new>
#include <memory>

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


#define LOG_LINESIZE 1000
HANDLE log_file = NULL;

void LogInfo(const char *format, ...)
{
  int ercd;
  va_list vl;

  char *buff = new(std::nothrow) char[LOG_LINESIZE]();
  if (buff == NULL) return;

  std::unique_ptr<char[]> auto_free_buff(buff);


  va_start(vl,format);

  ercd = vsnprintf_s(buff, LOG_LINESIZE, _TRUNCATE, format, vl);
  va_end(vl);

  if(ercd < 0) return;
  if(ercd > LOG_LINESIZE) ercd = LOG_LINESIZE;

  DWORD written;
  WriteFile(log_file, buff, ercd, &written, NULL);
}

void LogInfo_static(const char *format, ...)
{
  int ercd;
  va_list vl;
  char buff[LOG_LINESIZE];

  va_start(vl,format);

  ercd = vsnprintf_s(buff, LOG_LINESIZE, _TRUNCATE, format, vl);
  va_end(vl);

  if(ercd < 0) return;
  if(ercd > LOG_LINESIZE) ercd = LOG_LINESIZE;

  DWORD written;
  WriteFile(log_file, buff, ercd, &written, NULL);
}

void LogInfoW(const wchar_t *format, ...)
{
  int ercd;
  uint_t count = 0;
  va_list vl;
  wchar_t *wbuff = new(std::nothrow) wchar_t[LOG_LINESIZE * sizeof(wchar_t)]();
  if(wbuff == NULL) return;

  std::unique_ptr<wchar_t[]> auto_free_wbuf(wbuff);

  wbuff[0] = 0;
  va_start(vl,format);

  ercd = _vsnwprintf_s(wbuff, LOG_LINESIZE, _TRUNCATE, format, vl);
  va_end(vl);

  count = wchartoascii((char*)wbuff, LOG_LINESIZE * sizeof(wchar_t), wbuff);
    
  if(count == 0) return;

  DWORD written;
  WriteFile(log_file, wbuff, (DWORD)count, &written, NULL);
}

typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

BOOL IsWow64(void)
{
    BOOL bIsWow64 = FALSE;
    LPFN_ISWOW64PROCESS fnIsWow64Process;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
        GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

    if (NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
        {
            //handle error
        }
    }
    return bIsWow64;
}

uint_t wchartoascii(char *dest, uint_t dest_sz, const wchar_t *src)
{
  mbstate_t state;
  size_t avail = dest_sz, count;
  const wchar_t *srct = src;
  bool convissue = false;

  do {
    count = wcsrtombs(dest, &srct, avail, &state);
    if (srct)
    {
      convissue = true;
      srct++;
      if (*dest)
      {
        size_t xcount = strlen(dest);
        if (xcount >= avail) break;
        dest += xcount;
        avail -= xcount;
      }
    }
  } while (srct && *srct);

  if (count >= dest_sz || convissue) { dest[dest_sz - 1] = 0; count = strlen(dest); }

  return (uint_t)count;
}
