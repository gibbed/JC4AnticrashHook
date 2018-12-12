#include "stdafx.h"
#include "misc.h"

struct ResourceArray
{
  void** ptrs;
  uint32_t count;
};

struct
{
  HMODULE GameModule;

  struct
  {
    void* ImageBase;
    void* ImageEnd;
    size_t ImageSize;
  }
  D3D;

  struct
  {
    void* GlobalMemoryStatusEx;
    void* UpdateResources;
  }
  Targets;

  struct
  {
    BOOL(__fastcall* GlobalMemoryStatusEx)(LPMEMORYSTATUSEX);
    int(__fastcall* UpdateResources)(void*, uint32_t, ResourceArray*);
  }
  Originals;
}
gHookState = { };

int __fastcall MyUpdateResources(void* context, uint32_t index, ResourceArray* resources)
{
  struct UnknownType3
  {
    uint32_t type; // 0 = shader resource, 4 = constant buffer, 5 = sampler
    uint32_t start;
    uint32_t count;
  };

  struct UnknownType2
  {
    void* unknown_00;
    UnknownType3* unknown_08;
    uint32_t count;
    uint32_t unknown_14;
    void* unknown_18;
  };

  struct UnknownType1
  {
    UnknownType2 unknown_000[8];
    uint32_t unknown_100[8];
  };

  auto unknown1 = *AtOffset<UnknownType1**>(context, 0x18108);

  uint32_t needed_count = 0;
  const auto unknown2 = &unknown1->unknown_000[index];

  bool has_bad_resources = false;
  for (uint32_t i = 0; i < resources->count; ++i)
  {
    auto ptr = resources->ptrs[i];
    if (!ptr) continue;

    // We REALLY shouldn't use this, but we do.
    if (IsBadReadPtr(ptr, 8) == TRUE)
    {
      has_bad_resources = true;
      break;
    }

    auto vftable = *((void **)ptr);
    if (gHookState.D3D.ImageBase && (vftable < gHookState.D3D.ImageBase || vftable >= gHookState.D3D.ImageEnd))
    {
      has_bad_resources = true;
      break;
    }
  }

  for (uint32_t i = 0; i < unknown2->count; ++i)
  {
    needed_count += unknown2->unknown_08[i].count;
  }

  if (needed_count <= resources->count && has_bad_resources == false)
  {
    return gHookState.Originals.UpdateResources(context, index, resources);
  }

  printf("DETECTED BAD CALL!\n");
  printf("  context = %p, index = %u, resources = (%p, %u)\n", context, index, resources->ptrs, resources->count);
  printf("  needed %u, have %u, bad resources = %s\n", needed_count, resources->count, has_bad_resources ? "yes" : "no");
  //printf("  return address = %p\n", _ReturnAddress());

  ResourceArray new_resources;
  new_resources.ptrs = new void*[needed_count];
  new_resources.count = resources->count;

  std::memcpy(new_resources.ptrs, resources->ptrs, sizeof(void*) * resources->count);
  if (needed_count != resources->count)
  {
    std::memset(&new_resources.ptrs[resources->count], 0, sizeof(void*) * (needed_count - resources->count));
  }

  if (has_bad_resources == true)
  {
    for (uint32_t i = 0; i < resources->count; ++i)
    {
      auto ptr = new_resources.ptrs[i];
      if (!ptr) continue;

      // We REALLY shouldn't use this, but we do.
      if (IsBadReadPtr(ptr, 8) == TRUE)
      {
        printf("  index %u is INVALID: %p\n", i, ptr);
        new_resources.ptrs[i] = nullptr;
      }
      else if (gHookState.D3D.ImageBase)
      {
        auto vftable = *((void **)ptr);
        if (vftable < gHookState.D3D.ImageBase || vftable >= gHookState.D3D.ImageEnd)
        {
          printf("  index %u is INVALID: %p\n", i, ptr);
          new_resources.ptrs[i] = nullptr;
        }
      }
    }
  }

  auto result = gHookState.Originals.UpdateResources(context, index, &new_resources);

  delete[] new_resources.ptrs;
  return result;
}

// We leverage a call to GlobalMemoryStatusEx as a way to indicate
// that Denuvo has done its thing and decrypted most of the game code.
BOOL __stdcall MyGlobalMemoryStatusEx(LPMEMORYSTATUSEX lpBuffer)
{
  auto result = gHookState.Originals.GlobalMemoryStatusEx(lpBuffer);
  MH_DisableHook(gHookState.Targets.GlobalMemoryStatusEx);
  gHookState.Targets.GlobalMemoryStatusEx = nullptr;

  auto updateResources = InGameModule<void*>(gHookState.GameModule, 0x000000014B536260ULL);

  auto codeCheck = "\x49\x89\xE3\x55\x56\x41\x54\x41\x55\x48\x83\xEC\x58\x4C\x8B\x89\x08\x81\x01\x00";
  if (memcmp(updateResources, codeCheck, sizeof(codeCheck)))
  {
    printf("Code check failed. Unsupported version of Just Cause 4.\n");
    return result;
  }

  auto d3dModule = GetModuleHandleW(L"d3d11");
  MODULEINFO d3dInfo;
  if (d3dModule != nullptr && GetModuleInformation(GetCurrentProcess(), d3dModule, &d3dInfo, sizeof(d3dInfo)) == TRUE)
  {
    gHookState.D3D.ImageBase = d3dInfo.lpBaseOfDll;
    gHookState.D3D.ImageSize = d3dInfo.SizeOfImage;
    gHookState.D3D.ImageEnd = &static_cast<uint8_t*>(gHookState.D3D.ImageBase)[gHookState.D3D.ImageSize];
  }
  else
  {
    gHookState.D3D.ImageBase = gHookState.D3D.ImageEnd = nullptr;
    gHookState.D3D.ImageSize = 0;
  }

  printf("D3D = %p-%p (%llx)\n", gHookState.D3D.ImageBase, gHookState.D3D.ImageEnd, gHookState.D3D.ImageSize);

  auto status = DoHook(
    updateResources,
    MyUpdateResources, (LPVOID*)&gHookState.Originals.UpdateResources);
  if (status != MH_OK)
  {
    MessageBoxW(0, L"Failed to do hook for UpdateResources.", L"Error", MB_ICONERROR);
    ExitProcess(-3);
  }

  gHookState.Targets.UpdateResources = updateResources;

  return result;
}

BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  auto gameModule = GetModuleHandle(_T("JustCause4.exe"));
  if (gameModule == NULL)
  {
    return TRUE;
  }

  if (fdwReason == DLL_PROCESS_ATTACH)
  {
    FixConsole();

    MH_STATUS status;

    status = MH_Initialize();
    if (status != MH_OK)
    {
      MessageBoxW(0, L"Failed to initialize MinHook.", L"Error", MB_ICONERROR);
      exit(-1);
      return FALSE;
    }

    void* globalMemoryStatusEx;
    status = DoHook(
      L"kernel32", "GlobalMemoryStatusEx",
      MyGlobalMemoryStatusEx, (LPVOID*)&gHookState.Originals.GlobalMemoryStatusEx, &globalMemoryStatusEx);
    if (status != MH_OK)
    {
      MessageBoxW(0, L"Failed to do hook for GlobalMemoryStatusEx.", L"Error", MB_ICONERROR);
      exit(-2);
    }

    gHookState.GameModule = gameModule;
    gHookState.Targets.GlobalMemoryStatusEx = globalMemoryStatusEx;
  }
  else if (fdwReason == DLL_PROCESS_DETACH)
  {
    if (gHookState.Targets.UpdateResources)
    {
      MH_DisableHook(gHookState.Targets.UpdateResources);
      gHookState.Targets.UpdateResources = nullptr;
    }

    if (gHookState.Targets.GlobalMemoryStatusEx)
    {
      MH_DisableHook(gHookState.Targets.GlobalMemoryStatusEx);
      gHookState.Targets.GlobalMemoryStatusEx = nullptr;
    }

    gHookState = { };
    MH_Uninitialize();
  }

  return TRUE;
}
