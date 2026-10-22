#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0044a4f0 { uint8_t Unknown00[0x10]; int32_t State10; uint8_t Flags14; uint8_t Unknown15[0xeb]; };
#pragma pack(pop)

// Returns 1 when the dword at +0x10 equals 1 and bit 0x40 of the byte at +0x14 is set, else 0.
// FUNCTION: GP4 0x0044a4f0
int32_t __fastcall FUN_0044a4f0(Unknown_0044a4f0* self, void* /*edx*/) {
    int32_t result = 1;
    if (self->State10 != 1 || (self->Flags14 & 0x40) == 0) {
        result = 0;
    }
    return result;
}
GP4_IMPL(0x0044a4f0, FUN_0044a4f0, "thiscall(ptr[0x20]:u32{1,1,0,0x40,0x41,0x80}) -> i32")
