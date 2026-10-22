#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b8bb0 { int32_t Value00; int32_t Value04; int32_t Value08; uint8_t Flag0c; };
#pragma pack(pop)

// Zeroes the three dwords at +0, +4, +8 and the byte at +0xc of the object and returns the object
// (`mov eax, ecx; xor ecx, ecx; mov [eax], ecx; ...; mov [eax+0xc], cl; ret`).
// FUNCTION: GP4 0x005b8bb0
Unknown_005b8bb0* __fastcall FUN_005b8bb0(Unknown_005b8bb0* self, void* /*edx*/) {
    self->Value00 = 0;
    self->Value04 = 0;
    self->Value08 = 0;
    self->Flag0c = 0;
    return self;
}
GP4_IMPL(0x005b8bb0, FUN_005b8bb0, "thiscall(ptr[0x10]:bytes) -> i32")
