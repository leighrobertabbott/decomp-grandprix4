#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cbcf0 { uint8_t Unknown00[0x24]; int32_t Value24; uint8_t Unknown28[0xd8]; };
#pragma pack(pop)

// Tests whether the dword at +0x24 equals the argument (`cmp eax, ecx; sete al; ret 4`).
// Only al is written: the upper 24 bits of eax keep the upper bits of the loaded dword.
// FUNCTION: GP4 0x005cbcf0
int32_t __fastcall FUN_005cbcf0(const Unknown_005cbcf0* self, void* /*edx*/, int32_t value) {
    const int32_t field = self->Value24;
    return (int32_t)(((uint32_t)field & 0xffffff00u) | (uint32_t)(field == value));
}
GP4_IMPL(0x005cbcf0, FUN_005cbcf0, "thiscall(ptr[0x100]:{*bytes, 0x24:i32{=0,1,-1,2}}, i32{=0,1,-1,2}) -> i32")
