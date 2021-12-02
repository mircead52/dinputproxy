/* Copyright 2021 Mircea-Dacian Munteanu
 *
 * The Source Code is this file is released under the terms of the New BSD License,
 * see LICENSE file, or the project homepage: https://github.com/mircead52/dinputproxy
 */

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DINPUTPROXY_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DINPUTPROXY_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef DINPUTPROXY_EXPORTS
#define DINPUTPROXY_API __declspec(dllexport)
#else
#define DINPUTPROXY_API __declspec(dllimport)
#endif

extern DWORD token;
extern "C" HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);

