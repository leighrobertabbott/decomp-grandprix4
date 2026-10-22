#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b7bd0 { int32_t Value00; int32_t Value04; int32_t Value08; int32_t Value0c; int32_t Unknown10; int32_t Value14; };
#pragma pack(pop)

// Zeroes the dwords at +0, +4, +8, +0xc and +0x14 of the object and returns 0
// (`xor eax, eax; mov [ecx+4], eax; ...; ret`).
// FUNCTION: GP4 0x005b7bd0
int32_t __fastcall FUN_005b7bd0(Unknown_005b7bd0* self, void* /*edx*/) {
    self->Value04 = 0;
    self->Value08 = 0;
    self->Value0c = 0;
    self->Value00 = 0;
    self->Value14 = 0;
    return 0;
}
GP4_IMPL(0x005b7bd0, FUN_005b7bd0, "thiscall(ptr[0x20]:bytes) -> i32")
