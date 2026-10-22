#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Triple_00494ab0 { uint32_t Word00; uint32_t Word04; uint32_t Word08; };
#pragma pack(pop)

// Constructor of a 12-byte value: stores its three stack arguments at +0, +4, +8 and returns the
// object (eax = ecx).
// FUNCTION: GP4 0x00494ab0
Triple_00494ab0* __fastcall FUN_00494ab0(Triple_00494ab0* self, void* /*edx*/, uint32_t a, uint32_t b, uint32_t c) {
    self->Word00 = a;
    self->Word04 = b;
    self->Word08 = c;
    return self;
}
GP4_IMPL(0x00494ab0, FUN_00494ab0, "thiscall(ptr[0x10]:bytes, u32, u32, u32) -> u32")
