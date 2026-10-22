#include <gp4/gp4.h>

typedef void(__cdecl* LogFn_0046bb40)(const void* text);

// Reports an error through the logging callback stored at 0x00640664: writes the second argument, then
// "(<third>) : [CODE: <fourth>]" (formatted with the CRT sprintf 0x005db088 into a 40-byte stack buffer),
// then the string at 0x00640878 (" : *ERROR*\n"), the first argument and the string at 0x00640874.
// FUNCTION: GP4 0x0046bb40
void __cdecl FUN_0046bb40(const char* first, const char* second, int third, int fourth) {
    char buffer[0x28];
    LogFn_0046bb40 log = GP4_GLOBAL(LogFn_0046bb40, 0x00640664);
    log(second);
    gp4::cdecl_<0x005db088, int>(buffer, GP4_ARRAY(const char, 0x00640884), third, fourth);
    log(buffer);
    log(GP4_ARRAY(const char, 0x00640878));
    log(first);
    log(GP4_ARRAY(const char, 0x00640874));
}
GP4_IMPL(0x0046bb40, FUN_0046bb40, "cdecl(ptr[0x20]:u8{0x41,0x42,0x43,0,0}, ptr[0x20]:u8{0x41,0x42,0x43,0,0}, i32, i32) -> void")
