#include <gp4/gp4.h>

typedef void(__stdcall* OutputDebugStringA_t)(const char* text);

// Runs the CRT strstr (0x005db400) of the message against the literal "*ERROR*" at 0x00640670
// (result unused), then writes the message to the debugger.
// FUNCTION: GP4 0x0046b820
void __cdecl FUN_0046b820(const char* message) {
    gp4::cdecl_<0x005db400, char*>(message, GP4_ARRAY(const char, 0x00640670));
    GP4_IAT(OutputDebugStringA_t, 0x005f0290)(message);
}
GP4_IMPL(0x0046b820, FUN_0046b820, "cdecl(ptr[0x40]:u8{0x00,0x2a,0x45,0x52,0x4f,0x52,0x20,0x41}) -> void")
