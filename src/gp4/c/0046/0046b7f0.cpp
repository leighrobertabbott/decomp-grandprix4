#include <gp4/gp4.h>

// Formats the message "~~~ WAD opening from asm %s\n" for the given name into a 0x80-byte stack buffer
// with the CRT sprintf (0x005db088); the buffer is not used further. Returns sprintf's result.
// FUNCTION: GP4 0x0046b7f0
int __stdcall FUN_0046b7f0(const char* name) {
    char buffer[0x80];
    return gp4::cdecl_<0x005db088, int>(buffer, GP4_ARRAY(const char, 0x0063f8e0), name);
}
GP4_IMPL(0x0046b7f0, FUN_0046b7f0, "stdcall(ptr[0x40]:{0*63/1:u8{=65,98,46,0}, 63:u8{=0}}) -> i32")
