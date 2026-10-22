#include <gp4/gp4.h>

typedef void (__stdcall *OutputDebugStringA_t)(const char* text);
typedef int (__stdcall *QueryPerformanceFrequency_t)(void* frequency);
typedef uint32_t (__stdcall *timeBeginPeriod_t)(uint32_t period);

// Logs "\nInitTimers()\n" to the debugger. When there is no high-resolution counter, asks the
// multimedia timer for a resolution of 1..5 ms (the first period it accepts is kept in 0x00a66974;
// if every period reports 0x61, falls back to 1) and registers the timer shutdown 0x004b4c20 with
// the CRT's atexit. Always marks the timers as initialised in the byte 0x00a66970.
// FUNCTION: GP4 0x004b4c40
void __cdecl FUN_004b4c40() {
    GP4_IAT(OutputDebugStringA_t, 0x005f0290)(GP4_ARRAY(const char, 0x0065e3c0));
    if (GP4_IAT(QueryPerformanceFrequency_t, 0x005f0250)(GP4_ARRAY(uint8_t, 0x00a66968)) == 0) {
        GP4_GLOBAL(uint32_t, 0x00a66974) = 0;
        do {
            GP4_GLOBAL(uint32_t, 0x00a66974) = GP4_GLOBAL(uint32_t, 0x00a66974) + 1;
            if (GP4_IAT(timeBeginPeriod_t, 0x005f0388)(GP4_GLOBAL(uint32_t, 0x00a66974)) != 0x61) {
                goto registered;
            }
        } while (GP4_GLOBAL(uint32_t, 0x00a66974) < 5);
        GP4_GLOBAL(uint32_t, 0x00a66974) = 1;
        GP4_IAT(timeBeginPeriod_t, 0x005f0388)(1);
    registered:
        gp4::cdecl_<0x005daa79, int>(0x004b4c20);
    }
    GP4_GLOBAL(uint8_t, 0x00a66970) = 1;
}
GP4_IMPL(0x004b4c40, FUN_004b4c40, "cdecl() -> void; globals=fuzz:i32; emu=skip:win32 timeBeginPeriod")
