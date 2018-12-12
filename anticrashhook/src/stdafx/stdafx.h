#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <tchar.h>
#include <stdio.h>
#include <intrin.h>
#include <psapi.h>

#include <string>

#include <MinHook.h>

template<typename Out>
Out inline AtOffset(void* ptr, UINT64 offset)
{
  return reinterpret_cast<Out>(&(static_cast<uint8_t*>(ptr))[offset]);
}

template<typename Out>
Out __forceinline InGameModule(HMODULE module, UINT64 offset)
{
  const auto relative_offset = offset - 0x0000000140000000ULL;
  return reinterpret_cast<Out>(&(reinterpret_cast<uint8_t*>(module))[relative_offset]);
}
