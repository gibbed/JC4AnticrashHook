#pragma once

#include "stdafx.h"

void FixConsole(void);

MH_STATUS DoHook(void* target, LPVOID hook, LPVOID* orig);
MH_STATUS DoHook(const wchar_t* module_name, const char* proc_name, LPVOID hook, LPVOID* orig, void** out_target);
MH_STATUS DoHook(const wchar_t* module_name, const char* proc_name, LPVOID hook, LPVOID* orig);
