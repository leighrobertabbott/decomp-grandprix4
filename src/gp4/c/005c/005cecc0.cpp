#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cecc0 { uint8_t Unknown00[0x54]; uint32_t Current54[8]; uint32_t Saved74[8]; uint8_t Unknown94[0x6c]; };
#pragma pack(pop)

// Copies the eight dwords at +0x54..+0x73 to +0x74..+0x93.
// FUNCTION: GP4 0x005cecc0
void __fastcall FUN_005cecc0(Unknown_005cecc0* self, void* /*edx*/) {
    self->Saved74[0] = self->Current54[0];
    self->Saved74[1] = self->Current54[1];
    self->Saved74[2] = self->Current54[2];
    self->Saved74[3] = self->Current54[3];
    self->Saved74[4] = self->Current54[4];
    self->Saved74[5] = self->Current54[5];
    self->Saved74[6] = self->Current54[6];
    self->Saved74[7] = self->Current54[7];
}
GP4_IMPL(0x005cecc0, FUN_005cecc0, "thiscall(ptr[0x100]:bytes) -> void")
