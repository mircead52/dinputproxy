/* Copyright 2021 Mircea-Dacian Munteanu
 *
 * The Source Code is this file is released under the terms of the New BSD License,
 * see LICENSE file, or the project homepage: https://github.com/mircead52/dinputproxy
 */

// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <assert.h>
#include <stdio.h>


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
           )
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH: break;
  case DLL_THREAD_ATTACH: break;
  case DLL_THREAD_DETACH: break;
  case DLL_PROCESS_DETACH:
    if(log_file)
    {
      CloseHandle(log_file);
      log_file = NULL;
    }
    if (dllHandle)
    {
      FreeLibrary(dllHandle);
      dllHandle = NULL;
    }
    break;
  }
  return TRUE;
}

