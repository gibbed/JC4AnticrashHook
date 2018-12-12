#include "stdafx.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C"
{
  typedef void(*shim)(void);
  void __stdcall wrapper_load(void);
#define MAKE_SHIM(x,y) \
  extern shim shim_p_##x; \
  void shim_l_##x(void); \
  void shim_h_##x(void);
#include "wrapper_shims.inc"
#undef MAKE_SHIM
}

class Wrapper
{
public:
  Wrapper(const char *name) :
    _Name(name),
    _Module(nullptr)
  {
    InitializeCriticalSection(&this->_CriticalSection);
    this->Reset();
  }

  ~Wrapper()
  {
    EnterCriticalSection(&this->_CriticalSection);
    if (this->_Module != NULL)
    {
      this->Clear();
      FreeLibrary(this->_Module);
      this->_Module = nullptr;
    }
    LeaveCriticalSection(&this->_CriticalSection);
    DeleteCriticalSection(&this->_CriticalSection);
  }

private:
  const char *_Name;
  HMODULE _Module;
  CRITICAL_SECTION _CriticalSection;

  void CrashAndBurn(const wchar_t *fmt, ...)
  {
    const wchar_t *caption = L"Critical Error";
    wchar_t message[1024] = { 0 };
    va_list args;
    va_start(args, fmt);
    _vsnwprintf_s(message, _countof(message), fmt, args);
    va_end(args);

    MessageBoxW(GetDesktopWindow(), message, caption, MB_OK | MB_ICONERROR);
    ExitProcess('WRAP');
    DebugBreak();
  }

  void Clear(void)
  {
#define MAKE_SHIM(x,y) shim_p_##x = nullptr;
#include "wrapper_shims.inc"
#undef MAKE_SHIM
  }

  void Reset(void)
  {
#define MAKE_SHIM(x,y) shim_p_##x = (shim)&shim_l_##x;
#include "wrapper_shims.inc"
#undef MAKE_SHIM
  }

public:
  void Load(void)
  {
    EnterCriticalSection(&this->_CriticalSection);

    const char *missing_export_name = nullptr;

    if (this->_Module == NULL)
    {
      char path[MAX_PATH];
      GetSystemDirectoryA(path, MAX_PATH);
      strcat_s(path, "\\");
      strcat_s(path, this->_Name);

      HMODULE module = LoadLibraryA(path);
      if (module == NULL)
      {
        LeaveCriticalSection(&this->_CriticalSection);
        this->CrashAndBurn(L"Failed to load real %S. :(", this->_Name);
        return;
      }

#define MAKE_SHIM(x,y) \
      shim_p_##x = (shim)GetProcAddress(module, #x); \
      if (shim_p_##x == nullptr) \
      { \
        missing_export_name = #x; \
        goto missing_export; \
      }
#include "wrapper_shims.inc"
#undef MAKE_SHIM

      this->_Module = module;
    }

    LeaveCriticalSection(&this->_CriticalSection);
    return;

  missing_export:
    LeaveCriticalSection(&this->_CriticalSection);
    this->CrashAndBurn(L"Export '%S' is missing from real %S!", missing_export_name, this->_Name);
  }
};

Wrapper wrapper("XInput9_1_0");

extern "C"
{
    void __stdcall wrapper_load(void)
    {
        wrapper.Load();
    }
}
