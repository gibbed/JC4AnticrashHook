#ifndef MAKE_SHIM
#define MAKE_SHIM(x,y)
#define __MAKE_SHIM_UNDEF
#endif

MAKE_SHIM(DllMain,1)
MAKE_SHIM(XInputGetCapabilities,2)
MAKE_SHIM(XInputGetDSoundAudioDeviceGuids,3)
MAKE_SHIM(XInputGetState,4)
MAKE_SHIM(XInputSetState,5)

#ifdef __MAKE_SHIM_UNDEF
#undef __MAKE_SHIM_UNDEF
#undef MAKE_SHIM
#endif
