#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cbd80 { uint8_t Unknown00[0x18]; uint8_t Value18; uint8_t Unknown19[0xe7]; };
#pragma pack(pop)

// Copies the byte at +0x18 of the object to *out (`mov al,[ecx+0x18]; mov ecx,[esp+4]; mov [ecx],al;
// ret 4`); al keeps the copied byte.
// FUNCTION: GP4 0x005cbd80
uint8_t __fastcall FUN_005cbd80(const Unknown_005cbd80* self, void* /*edx*/, uint8_t* out) {
    const uint8_t value = self->Value18;
    *out = value;
    return value;
}
GP4_IMPL(0x005cbd80, FUN_005cbd80, "thiscall(ptr[0x100]:bytes, ptr[8]:bytes) -> u8")
