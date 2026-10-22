#include <gp4/gp4.h>

// Forwards its four stack arguments to the CRT fread (0x005dc726: buffer, size, count, FILE*) and
// returns its result.
// FUNCTION: GP4 0x0046b390
int __stdcall FUN_0046b390(int buffer, int size, int count, int file) {
    return gp4::cdecl_<0x005dc726, int>(buffer, size, count, file);
}
GP4_IMPL(0x0046b390, FUN_0046b390, "stdcall(ptr[0x100]:bytes, i32[1..8], i32[1..8], ptr[0x40]:bytes) -> i32; emu=skip:win32 EnterCriticalSection")
