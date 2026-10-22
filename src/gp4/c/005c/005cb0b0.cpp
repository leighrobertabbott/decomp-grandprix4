#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cb0b0 { uint8_t Unknown00[0x18]; int32_t Value18; uint8_t Unknown1c[0xe4]; };
#pragma pack(pop)

// Tests whether the argument equals the dword at +0x18 (`mov eax, [esp+4]; mov edx, [ecx+0x18];
// cmp eax, edx; sete al; ret 4`). Only al is written: the upper 24 bits of eax keep the upper
// bits of the argument.
// FUNCTION: GP4 0x005cb0b0
int32_t __fastcall FUN_005cb0b0(const Unknown_005cb0b0* self, void* /*edx*/, int32_t value) {
    const int32_t field = self->Value18;
    return (int32_t)(((uint32_t)value & 0xffffff00u) | (uint32_t)(value == field));
}
GP4_IMPL(0x005cb0b0, FUN_005cb0b0, "thiscall(ptr[0x100]:{0x18:i32{=0,1,-1,2}}, i32{=0,1,-1,2}) -> i32")
