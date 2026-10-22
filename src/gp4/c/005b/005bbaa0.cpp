#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005bbaa0 { uint8_t Unknown00[8]; int32_t Value08; uint8_t Unknown0c[0xf4]; };
#pragma pack(pop)

// Clears the dword at +8 of the object and returns the object (`mov eax, ecx; mov [eax+8], 0; ret`).
// FUNCTION: GP4 0x005bbaa0
Unknown_005bbaa0* __fastcall FUN_005bbaa0(Unknown_005bbaa0* self, void* /*edx*/) {
    self->Value08 = 0;
    return self;
}
GP4_IMPL(0x005bbaa0, FUN_005bbaa0, "thiscall(ptr[0x100]:bytes) -> i32")
