#include "stdafx.h"

void FixConsole(void)
{
  if (AttachConsole((DWORD)-1) == FALSE)
  {
    AllocConsole();
  }
  FILE *dummy;
  freopen_s(&dummy, "CONIN$", "rb", stdin);
  freopen_s(&dummy, "CONOUT$", "wb", stdout);
  freopen_s(&dummy, "CONOUT$", "wb", stderr);
}

MH_STATUS DoHook(void* target, LPVOID hook, LPVOID* orig)
{
  auto status = MH_CreateHook(target, hook, orig);
  if (status != MH_OK)
  {
    return status;
  }
  return MH_EnableHook(target);
}

MH_STATUS DoHook(const wchar_t* module_name, const char* proc_name, LPVOID hook, LPVOID* orig, void** out_target)
{
  auto module = GetModuleHandleW(module_name);
  if (module == nullptr)
  {
    return MH_ERROR_MODULE_NOT_FOUND;
  }
  auto target = GetProcAddress(module, proc_name);
  if (target == nullptr)
  {
    return MH_ERROR_FUNCTION_NOT_FOUND;
  }

  if (out_target != nullptr)
  {
    *out_target = (void*)target;
  }

  return DoHook(target, hook, orig);
}

MH_STATUS DoHook(const wchar_t* module_name, const char* proc_name, LPVOID hook, LPVOID* orig)
{
  return DoHook(module_name, proc_name, hook, orig, nullptr);
}
