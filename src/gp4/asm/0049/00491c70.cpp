#include <gp4/gp4.h>

// Copy constructor of a 0x40-byte value: copies 16 dwords from the stack argument to the
// object (rep movsd) and returns the object (eax = ecx).
// FUNCTION: GP4 0x00491c70
uint32_t* __fastcall FUN_00491c70(uint32_t* self, void* /*edx*/, const uint32_t* src) {
    for (int i = 0; i < 16; i++) {
        self[i] = src[i];
    }
    return self;
}
GP4_IMPL(0x00491c70, FUN_00491c70, "thiscall(ptr[0x40]:bytes, ptr[0x40]:bytes) -> u32")
