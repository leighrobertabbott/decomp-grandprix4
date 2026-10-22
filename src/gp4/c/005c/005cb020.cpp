#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cb020 { uint8_t Unknown00[0x20]; int32_t Value20; uint8_t Unknown24[0xdc]; };
#pragma pack(pop)

// Tests whether the dword at +0x20 equals the argument (`cmp eax, ecx; sete al; ret 4`).
// Only al is written: the upper 24 bits of eax keep the upper bits of the loaded dword.
// FUNCTION: GP4 0x005cb020
int32_t __fastcall FUN_005cb020(const Unknown_005cb020* self, void* /*edx*/, int32_t value) {
    const int32_t field = self->Value20;
    return (int32_t)(((uint32_t)field & 0xffffff00u) | (uint32_t)(field == value));
}
GP4_IMPL(0x005cb020, FUN_005cb020, "thiscall(ptr[0x100]:{*bytes, 0x20:i32{=0,1,-1,2}}, i32{=0,1,-1,2}) -> i32")
