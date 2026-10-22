#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0046bc80 {
    uint8_t Unknown00[8];
    int32_t Value08;
    int32_t Value0c;
};
#pragma pack(pop)

// Formats the two integers at +0xc and +8 as "%02d:%02d" (a clock reading) into the caller's
// buffer through the CRT sprintf (0x005db088) and returns its result.
// FUNCTION: GP4 0x0046bc80
int __fastcall FUN_0046bc80(Unknown_0046bc80* self, void* /*edx*/, char* buffer) {
    return gp4::cdecl_<0x005db088, int>(buffer, GP4_ARRAY(const char, 0x00640898), self->Value0c, self->Value08);
}
GP4_IMPL(0x0046bc80, FUN_0046bc80, "thiscall(ptr[0x20]:{8:i32, 0xc:i32}, ptr[0x40]:bytes) -> i32")
