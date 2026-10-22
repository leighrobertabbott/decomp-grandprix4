#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cbe60 { uint8_t Unknown00[0x18]; uint8_t Value18; uint8_t Unknown19[0xe7]; };
#pragma pack(pop)

// Tests whether the byte at +0x18 equals the low byte of the argument
// (`mov al, [ecx+0x18]; mov cl, [esp+4]; cmp al, cl; sete al; ret 4`); the result is in al only.
// FUNCTION: GP4 0x005cbe60
uint8_t __fastcall FUN_005cbe60(const Unknown_005cbe60* self, void* /*edx*/, int32_t value) {
    return self->Value18 == (uint8_t)value;
}
GP4_IMPL(0x005cbe60, FUN_005cbe60, "thiscall(ptr[0x100]:bytes, i32) -> u8")
