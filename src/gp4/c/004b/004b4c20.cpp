#include <gp4/gp4.h>

typedef void (__stdcall *OutputDebugStringA_t)(const char* text);
typedef uint32_t (__stdcall *timeEndPeriod_t)(uint32_t period);

// Logs "\nUninitTimers()\n" to the debugger and ends the timer-resolution period stored at 0x00a66974.
// FUNCTION: GP4 0x004b4c20
uint32_t __cdecl FUN_004b4c20() {
    GP4_IAT(OutputDebugStringA_t, 0x005f0290)(GP4_ARRAY(const char, 0x0065e3ac));
    return GP4_IAT(timeEndPeriod_t, 0x005f0384)(GP4_GLOBAL(uint32_t, 0x00a66974));
}
GP4_IMPL(0x004b4c20, FUN_004b4c20, "cdecl() -> i32; globals=fuzz:i32; emu=skip:win32 timeEndPeriod")
