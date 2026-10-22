#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005d5280 { uint8_t Unknown00[0x178]; uint32_t Current178[4]; uint32_t Saved188[4]; uint8_t Unknown198[8]; };
#pragma pack(pop)

// Copies the four dwords at +0x178..+0x187 to +0x188..+0x197, then continues in 0x005cecc0 (`jmp`) on the same object.
// FUNCTION: GP4 0x005d5280
void __fastcall FUN_005d5280(Unknown_005d5280* self, void* /*edx*/) {
    self->Saved188[0] = self->Current178[0];
    self->Saved188[1] = self->Current178[1];
    self->Saved188[2] = self->Current178[2];
    self->Saved188[3] = self->Current178[3];
    gp4::thiscall_<0x005cecc0, void>(self);
}
GP4_IMPL(0x005d5280, FUN_005d5280, "thiscall(ptr[0x1a0]:bytes) -> void")
