#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005bbae0 { int32_t Field00; int32_t Field04; uint8_t Unknown08[0xf8]; };
#pragma pack(pop)

// Clears the two dwords at +0 and +4 and returns the object (`mov eax, ecx; mov [eax], 0; mov [eax+4], 0; ret`).
// FUNCTION: GP4 0x005bbae0
Unknown_005bbae0* __fastcall FUN_005bbae0(Unknown_005bbae0* self, void* /*edx*/) {
    self->Field00 = 0;
    self->Field04 = 0;
    return self;
}
GP4_IMPL(0x005bbae0, FUN_005bbae0, "thiscall(ptr[0x100]:bytes) -> i32")
