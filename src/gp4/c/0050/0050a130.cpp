#include <gp4/gp4.h>

typedef void(__stdcall* OutputDebugStringA_t)(const char* text);
typedef uint32_t(__stdcall* timeKillEvent_t)(uint32_t id);
typedef uint32_t(__stdcall* timeEndPeriod_t)(uint32_t period);

// Shuts the multimedia timer down: when the running flag at 0x00b2cf00 is set it kills the timer event
// stored at 0x00b2ce60 (logging "MM Timer shutdown error" plus "Invalid Paramters" for error 0xb or
// "Unknown error" otherwise) and clears the flag, then ends the timer period at 0x0066d710 (logging
// "Timer end period error" on failure).
// FUNCTION: GP4 0x0050a130
void __cdecl MMTimer_Stop() {
    if (GP4_GLOBAL(uint8_t, 0x00b2cf00) != 0) {
        uint32_t result = GP4_IAT(timeKillEvent_t, 0x005f0370)(GP4_GLOBAL(uint32_t, 0x00b2ce60));
        if (result != 0) {
            GP4_IAT(OutputDebugStringA_t, 0x005f0290)(GP4_ARRAY(const char, 0x0066d750));
            if (result == 0xb) {
                GP4_IAT(OutputDebugStringA_t, 0x005f0290)(GP4_ARRAY(const char, 0x0066d73c));
            } else {
                GP4_IAT(OutputDebugStringA_t, 0x005f0290)(GP4_ARRAY(const char, 0x0066d72c));
            }
        }
        GP4_GLOBAL(uint8_t, 0x00b2cf00) = 0;
    }
    if (GP4_IAT(timeEndPeriod_t, 0x005f0384)(GP4_GLOBAL(uint32_t, 0x0066d710)) != 0) {
        GP4_IAT(OutputDebugStringA_t, 0x005f0290)(GP4_ARRAY(const char, 0x0066d714));
    }
}
GP4_IMPL(0x0050a130, MMTimer_Stop, "cdecl() -> void; globals=fuzz:i32; gfields={0x00b2cf00:u8{0,1}}; emu=skip:win32 timeEndPeriod")
