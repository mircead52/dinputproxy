/* Copyright 2021 Mircea-Dacian Munteanu
 *
 * The Source Code is this file is released under the terms of the New BSD License,
 * see LICENSE file, or the project homepage: https://github.com/mircead52/dinputproxy
 */

#include "stdafx.h"
#include "config.h"
#include "dinputproxy.h"

#define DIRECTINPUT_VERSION 0x0800


#pragma warning(push)
#pragma warning(disable:6000 28251)
#include <dinput.h>
#pragma warning(pop)

#include <assert.h>

#include <stdio.h>
#include <varargs.h>

#include <string>
#include <list>

#include <new>
#include <memory>

#if _MSC_VER <= 1700
#define LOG_FILENAME TEXT(MODULE_NAME)TEXT(".log")
#else
#define LOG_FILENAME TEXT(MODULE_NAME".log")
#endif
#define MY_TOKEN_VAL 0x5aB63792u

#define GUID_NUM_CHARS 40u

typedef HRESULT(WINAPI *dicreate_t)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);

struct enumcbA
{
    LPDIENUMDEVICESCALLBACKA cb;
    LPVOID ex;
};

struct enumcbW
{
    LPDIENUMDEVICESCALLBACKW cb;
    LPVOID ex;
};

typedef std::list<std::wstring> wstrlist;
static void find_dinput_file(TCHAR *dir, wstrlist &out, size_t out_sz, int depth = 0);

HINSTANCE dllHandle = NULL;
DWORD token = MY_TOKEN_VAL;

static dicreate_t dicreateep = NULL;
static BOOL isWOW64 = FALSE;

static BOOL CALLBACK EnumFFDevicesCallbackA(const DIDEVICEINSTANCEA* pInst,
    VOID* pContext)
{
    DIDEVICEINSTANCEA *data = (DIDEVICEINSTANCEA*)new(std::nothrow) char[pInst->dwSize]();
    if (data == NULL) return DIENUM_CONTINUE;
    std::unique_ptr<char[]> auto_free_x((char*)data); //use this to free memory on function exit
    struct enumcbA *callback = (struct enumcbA*)pContext;
    BOOL skip = FALSE;
    const char *act = "";
    DWORD devType = pInst->dwDevType;

    DWORD type = pInst->dwDevType & 0xFF;

    memcpy(data, pInst, pInst->dwSize);

    if (type == DI8DEVTYPE_1STPERSON && configx.mask_xone_ctrl)
    {
        data->dwDevType &= ~0xFF;
        data->dwDevType |= DI8DEVTYPE_GAMEPAD;
        type = DI8DEVTYPE_GAMEPAD;
        act = "Mask";
    }

#if 0
    if (type == DI8DEVTYPE_KEYBOARD)
    {
        x->dwDevType = DI8DEVTYPE_KEYBOARD | DI8DEVTYPEKEYBOARD_UNKNOWN << 8;
    }
    else if (type == DI8DEVTYPE_MOUSE)
    {
        x->dwDevType = DI8DEVTYPE_MOUSE | DI8DEVTYPEMOUSE_UNKNOWN << 8;
    }
#endif

    if(configx.allowclass.count(type) || configx.allowclass.count(0xFFu))
    {
        //device is allowed
    }
    else
    {
        skip = TRUE;
        act = "Disallow";
    }

    wchar_t *strguid = NULL;
    StringFromIID(pInst->guidInstance, &strguid);

    char *strguidascii = (char*)strguid;
    uint_t count = wchartoascii(strguidascii, (uint_t)(wcslen(strguid) + 1) * sizeof(wchar_t), strguid);

    if(configx.blacklist.count(strguidascii))
    {
        skip = TRUE;
        act = "Blacklist";
    }

    LogInfo("\tEnumCB %9s %06x %s %s\r\n", act, devType, pInst->tszInstanceName, strguidascii);

    CoTaskMemFree(strguid);

    if (skip)
        return DIENUM_CONTINUE;
    else
        return callback->cb(data, callback->ex);
}

static BOOL CALLBACK EnumFFDevicesCallbackW(const DIDEVICEINSTANCEW* pInst,
    VOID* pContext)
{
    DIDEVICEINSTANCEW* data = (DIDEVICEINSTANCEW*)new(std::nothrow) char[pInst->dwSize]();
    if (data == NULL) return DIENUM_CONTINUE;
    std::unique_ptr<char[]> auto_free_x((char*)data); //use this to free memory on function exit
    struct enumcbW *callback = (struct enumcbW*)pContext;
    BOOL skip = FALSE;
    const TCHAR *act = TEXT("");
    DWORD devType = pInst->dwDevType;
  
    DWORD type = pInst->dwDevType & 0xFF;

    memcpy(data, pInst, pInst->dwSize);

    if (type == DI8DEVTYPE_1STPERSON && configx.mask_xone_ctrl)
    {
        data->dwDevType &= ~0xFF;
        data->dwDevType |= DI8DEVTYPE_GAMEPAD;
        type = DI8DEVTYPE_GAMEPAD;
        act = TEXT("Mask");
    }

#if 0
    if (type == DI8DEVTYPE_KEYBOARD)
    {
        data->dwDevType = DI8DEVTYPE_KEYBOARD | DI8DEVTYPEKEYBOARD_UNKNOWN << 8;
    }
    else if (type == DI8DEVTYPE_MOUSE)
    {
        data->dwDevType = DI8DEVTYPE_MOUSE | DI8DEVTYPEMOUSE_UNKNOWN << 8;
    }
#endif

    if(configx.allowclass.count(type) || configx.allowclass.count(0xFFu))
    {
    //device is allowed
    }
    else
    {
        skip = TRUE;
        act = TEXT("Disallow");
    }

    TCHAR *strguid = NULL;
    StringFromIID(pInst->guidInstance, &strguid);

    char *strguidascii = new(std::nothrow) char[GUID_NUM_CHARS]();
    std::unique_ptr<char[]> auto_free_strguidascii(strguidascii);
    uint_t count = wchartoascii(strguidascii, GUID_NUM_CHARS, strguid);
  
    if(configx.blacklist.count(strguidascii))
    {
        skip = TRUE;
        act = TEXT("Blacklist");
    }

    LogInfoW(L"\tEnumCB %9s %06x %s %s\r\n", act, devType, pInst->tszInstanceName, strguid);
    
    CoTaskMemFree(strguid);
    
    if (skip)
        return DIENUM_CONTINUE;
    else
        return callback->cb(data, callback->ex);
}

class DITestA : public IDirectInput8A
{
private:
  struct IDirectInput8A *m_pDI;
public:
  DITestA(IDirectInput8A *pDI) : m_pDI(pDI) {}
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) { return m_pDI->QueryInterface(riid, ppvObj); }
    STDMETHOD_(ULONG, AddRef)(THIS) { return m_pDI->AddRef(); }
    STDMETHOD_(ULONG, Release)(THIS) { ULONG ret = m_pDI->Release(); if(ret == 0) delete this; return ret; }

    STDMETHOD(CreateDevice)(THIS_ REFGUID a, LPDIRECTINPUTDEVICE8A *b, LPUNKNOWN c) { return m_pDI->CreateDevice(a, b, c); }
    STDMETHOD(EnumDevices)(THIS_ DWORD a, LPDIENUMDEVICESCALLBACKA b, LPVOID c, DWORD d)
    {
        struct enumcbA extra = { b, c };
        LogInfo("EnumDevicesA DevType %u Flags %x\r\n", a, d);
        return m_pDI->EnumDevices(a, EnumFFDevicesCallbackA, &extra, d);
    }
    STDMETHOD(GetDeviceStatus)(THIS_ REFGUID a) { return m_pDI->GetDeviceStatus(a); }
    STDMETHOD(RunControlPanel)(THIS_ HWND a, DWORD b) { return m_pDI->RunControlPanel(a,b); }
    STDMETHOD(Initialize)(THIS_ HINSTANCE a, DWORD b) { return m_pDI->Initialize(a,b); }
    STDMETHOD(FindDevice)(THIS_ REFGUID a, LPCSTR b, LPGUID c) { return m_pDI->FindDevice(a,b,c); }
    STDMETHOD(EnumDevicesBySemantics)(THIS_ LPCSTR a, LPDIACTIONFORMATA b, LPDIENUMDEVICESBYSEMANTICSCBA c, LPVOID d, DWORD e) { return m_pDI->EnumDevicesBySemantics(a, b, c, d, e); }
    STDMETHOD(ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK a, LPDICONFIGUREDEVICESPARAMSA b, DWORD c, LPVOID d) { return m_pDI->ConfigureDevices(a,b,c,d); }
};

class DITestW : public IDirectInput8W
{
private:
  struct IDirectInput8W *m_pDI;
public:
  DITestW(IDirectInput8W *pDI) : m_pDI(pDI) {}
  /*** IUnknown methods ***/
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) { return m_pDI->QueryInterface(riid, ppvObj); }
  STDMETHOD_(ULONG,AddRef)(THIS) { return m_pDI->AddRef(); }
  STDMETHOD_(ULONG,Release)(THIS) { ULONG ret = m_pDI->Release(); if(ret == 0) delete this; return ret; }

  /*** IDirectInput8W methods ***/
  STDMETHOD(CreateDevice)(THIS_ REFGUID a,LPDIRECTINPUTDEVICE8W *b,LPUNKNOWN c) { return m_pDI->CreateDevice(a, b, c); }
  STDMETHOD(EnumDevices)(THIS_ DWORD a,LPDIENUMDEVICESCALLBACKW b,LPVOID c,DWORD d)
  {
      struct enumcbW extra = { b, c };
      LogInfo("EnumDevicesW DevType %u Flags %x\r\n", a, d);
      return m_pDI->EnumDevices(a, EnumFFDevicesCallbackW, &extra, d);
  }
  STDMETHOD(GetDeviceStatus)(THIS_ REFGUID a) { return m_pDI->GetDeviceStatus(a); }
  STDMETHOD(RunControlPanel)(THIS_ HWND a,DWORD b) { return m_pDI->RunControlPanel(a,b); }
  STDMETHOD(Initialize)(THIS_ HINSTANCE a,DWORD b) { return m_pDI->Initialize(a,b); }
  STDMETHOD(FindDevice)(THIS_ REFGUID a,LPCWSTR b,LPGUID c) { return m_pDI->FindDevice(a,b,c); }
  STDMETHOD(EnumDevicesBySemantics)(THIS_ LPCWSTR a,LPDIACTIONFORMATW b,LPDIENUMDEVICESBYSEMANTICSCBW c,LPVOID d,DWORD e) { return m_pDI->EnumDevicesBySemantics(a, b, c, d, e); }
  STDMETHOD(ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK a,LPDICONFIGUREDEVICESPARAMSW b,DWORD c,LPVOID d) { return m_pDI->ConfigureDevices(a,b,c,d); }
};

#define PATH_SZ MAX_PATH
extern "C" HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
{
    VOID *pDI = NULL;
  
    if(log_file == NULL)
    {
        HANDLE h = CreateFile(LOG_FILENAME,               // file name 
                            GENERIC_WRITE,          
                            FILE_SHARE_READ | FILE_SHARE_DELETE,                     
                            NULL,                  // default security 
                            CREATE_ALWAYS,         // existing file only 
                            FILE_ATTRIBUTE_NORMAL, // normal file 
                            NULL);                 // no template 
        if (h != INVALID_HANDLE_VALUE)
            log_file = h;
    }

    if (dllHandle == NULL)
    {
        size_t count = 0;
        wstrlist found_dlls;
        wstrlist::const_iterator dlls_it;

        read_conf();

        TCHAR *dllpath = new(std::nothrow) TCHAR[PATH_SZ]();
        std::unique_ptr<TCHAR[]> auto_free_dllpath(dllpath);
        TCHAR *filepath = NULL;
        std::unique_ptr<TCHAR[]> auto_free_filepath;

        if(dllpath == NULL)
            return DIERR_OUTOFMEMORY;

        if(configx.dllpath)
        {
            char *src = configx.dllpath;
            mbstate_t state;
            filepath = new(std::nothrow) TCHAR[PATH_SZ]();
            if(filepath == NULL)
            return DIERR_OUTOFMEMORY;
            auto_free_filepath.reset(filepath);
            filepath[0] = 0;
            count = mbsrtowcs(filepath, (const char**)&src, PATH_SZ, &state);
            if(count == 0 || count >= PATH_SZ)
            {
            LogInfo("Unable to parse config value for DLL Path errinfo 0x%x\r\n",src ? *src : 0);
            return DIERR_GENERIC;
            }
            goto load_dll;
        }

        int ercd = GetWindowsDirectory(dllpath, PATH_SZ);
        if(ercd == 0 || ercd >= PATH_SZ)
        {
            return DIERR_GENERIC;
        }

#if idx64
        wcscat_s(dllpath, PATH_SZ, TEXT("\\WinSxS"));
#else
        isWOW64 = IsWow64();
        if (isWOW64)
        {
            //wcscat_s(dllpath, PATH_SZ, TEXT("\\WinSxS"));
            wcscat_s(dllpath, PATH_SZ, TEXT("\\SysWOW64"));
        }
        else
        {
            wcscat_s(dllpath, PATH_SZ, TEXT("\\WinSxS"));
        }
#endif
        LogInfoW(L"Search for dinput8 dll in %s\r\n", dllpath);

        {
            find_dinput_file(dllpath, found_dlls, PATH_SZ);

            if(found_dlls.size() == 0)
            {
                LogInfo("SearchPath failed\r\n");
                return DIERR_GENERIC;
            }

            //our first option is the dll placed in the root of system32 folder
            found_dlls.emplace_front(L".\\dinput8.dll");

            dlls_it = found_dlls.begin();
            //dlls_it++;// dlls_it++;
            filepath = const_cast<TCHAR*>(dlls_it->c_str());

            if(configx.find_all_dlls && found_dlls.size() > 1)
            {
                wstrlist::const_iterator it;
                for (it = found_dlls.begin(); it != found_dlls.end(); ++it)
                {
                    LogInfoW(L"Found dll: %s\r\n", it->c_str());
                }
            }
        }

    load_dll:
        LogInfoW(L"Try loading system dinput8 dll: %s\r\n", filepath);
        //Load the dll and keep the handle to it
        dllHandle = LoadLibraryEx(filepath, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if(dllHandle == NULL)
        {
            if ((found_dlls.size() != 0) && (++dlls_it != found_dlls.end()))
            {
                filepath = const_cast<TCHAR*>(dlls_it->c_str());
                goto load_dll;
            }
            return DIERR_GENERIC;
        }

        //Get pointer to our function using GetProcAddress:
        dicreateep = (dicreate_t)GetProcAddress(dllHandle, "DirectInput8Create");
        DWORD *token_adr = (DWORD*)GetProcAddress(dllHandle, "token");

        if (dicreateep == NULL)
        {
            LogInfo("Failed to find function DirectInput8Create\r\n");
            FreeLibrary(dllHandle);
            dllHandle = NULL;
            return DIERR_GENERIC;
        }
        if(dicreateep == &DirectInput8Create)
        {
            LogInfo("Recursive entrypoint for DirectInput8Create\r\n");
            FreeLibrary(dllHandle);
            dllHandle = NULL;
            return DIERR_GENERIC;
        }
        if(token_adr != NULL && *token_adr == MY_TOKEN_VAL)
        {
            LogInfo("Token detected, assuming recursive load\r\n");
            FreeLibrary(dllHandle);
            dllHandle = NULL;
            return DIERR_GENERIC;
        }
    }

    HRESULT hr = dicreateep(hinst, dwVersion, riidltf, (VOID**)&pDI, punkOuter);
    if (hr >= 0)
    {
        if(pDI == 0)
            return DIERR_OUTOFMEMORY;

        REFIID uniguid = IID_IDirectInput8W;
        if(0 == memcmp(&riidltf, &uniguid, sizeof(IID)))
        {
              //UNICODE variant
              *ppvOut = new DITestW((IDirectInput8W *)pDI);
        }
        else
        {
              *ppvOut = new DITestA((IDirectInput8A *)pDI);
        }
    }

    return hr;
}

#define SEARCH_MAX_DEPTH 2

static void find_dinput_file(TCHAR *dir, wstrlist &out, size_t out_sz, int depth)
{
    WIN32_FIND_DATA *find = new(std::nothrow) WIN32_FIND_DATA();
    std::unique_ptr<WIN32_FIND_DATA> auto_free_find(find);
    TCHAR *wbuff = new(std::nothrow) TCHAR[PATH_SZ]();
    std::unique_ptr<TCHAR[]> auto_free_wbuff(wbuff);
    HANDLE hnd;
    BOOL res = TRUE;

    if (wbuff == NULL || find == NULL) return;

    wcscpy_s(wbuff, PATH_SZ, dir);
    wcscat_s(wbuff, PATH_SZ, L"\\");
    wcscat_s(wbuff, PATH_SZ, L"dinput8.dll");
    hnd = FindFirstFile(wbuff, find);

    if(hnd != INVALID_HANDLE_VALUE)
    {
        FindClose(hnd);
        out.emplace_back(wbuff);
        return;
    }

    if(depth >= SEARCH_MAX_DEPTH)
    {
        return;
    }
  
    wcscpy_s(wbuff, PATH_SZ, dir);
    wcscat_s(wbuff, PATH_SZ, L"\\*");
    hnd = FindFirstFile(wbuff, find);


    while(hnd != INVALID_HANDLE_VALUE && res == TRUE)
    {
        if((find->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) goto nextfile;
        if(0 == wcscmp(find->cFileName, L".")) goto nextfile;
        if(0 == wcscmp(find->cFileName, L"..")) goto nextfile;
        wcscpy_s(wbuff, PATH_SZ, dir);
        wcscat_s(wbuff, PATH_SZ, L"\\");
        wcscat_s(wbuff, PATH_SZ, find->cFileName);
        find_dinput_file(wbuff, out, out_sz, depth +1);
        if (configx.find_all_dlls == true)
        {
            //continue search
        }
        else if(out.size() != 0)
        {
            FindClose(hnd);
            return;
        }
    nextfile:
        res = FindNextFile(hnd, find);
    }
    if(hnd != INVALID_HANDLE_VALUE)
    {
        FindClose(hnd);
    }
}
