#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b1e40 { uint8_t Unknown00[6]; uint16_t Value06; uint8_t Unknown08[0x20]; uint8_t Flag28; uint8_t Unknown29[0xd7]; };
#pragma pack(pop)

// Sets the flag byte at +0x28 and stores the low word of the stack argument at +6; ax keeps the word
// (`mov ax, [esp+4]; mov byte [ecx+0x28], 1; mov [ecx+6], ax; ret 4`).
// FUNCTION: GP4 0x005b1e40
uint16_t __fastcall FUN_005b1e40(Unknown_005b1e40* self, void* /*edx*/, int32_t value) {
    const uint16_t word = (uint16_t)value;
    self->Flag28 = 1;
    self->Value06 = word;
    return word;
}
GP4_IMPL(0x005b1e40, FUN_005b1e40, "thiscall(ptr[0x100]:bytes, i32) -> u16")
