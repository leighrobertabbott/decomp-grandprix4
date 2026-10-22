#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_005b42f0 { const void* Vptr00; int32_t Field04; int32_t Field08; uint8_t Unknown0c[0xf4]; };
#pragma pack(pop)

// Constructor-style initialiser: installs the vtable at 0x005fa068, clears +4 and +8, returns this
// (same shape as the twin 0x004cad80 with another vtable).
// FUNCTION: GP4 0x005b42f0
Object_005b42f0* __fastcall FUN_005b42f0(Object_005b42f0* self, void* /*edx*/) {
    self->Vptr00 = GP4_ARRAY(const uint32_t, 0x005fa068);
    self->Field08 = 0;
    self->Field04 = 0;
    return self;
}
GP4_IMPL(0x005b42f0, FUN_005b42f0, "thiscall(ptr[0x100]:bytes) -> u32")
