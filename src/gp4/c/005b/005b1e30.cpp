#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b1e30 { uint8_t Unknown00[8]; int32_t Value08; uint8_t Unknown0c[0x1d]; uint8_t Flag29; uint8_t Unknown2a[0xd6]; };
#pragma pack(pop)

// Sets the flag byte at +0x29 and stores the stack argument at +8 of the object (eax keeps the argument:
// `mov eax,[esp+4]; mov byte [ecx+0x29],1; mov [ecx+8],eax; ret 4`).
// FUNCTION: GP4 0x005b1e30
int32_t __fastcall FUN_005b1e30(Unknown_005b1e30* self, void* /*edx*/, int32_t value) {
    self->Flag29 = 1;
    self->Value08 = value;
    return value;
}
GP4_IMPL(0x005b1e30, FUN_005b1e30, "thiscall(ptr[0x100]:bytes, i32) -> i32")
