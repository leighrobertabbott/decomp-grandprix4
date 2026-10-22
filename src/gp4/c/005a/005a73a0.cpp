#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_005a73a0 { const void* Vptr00; int32_t Field04; };
#pragma pack(pop)

// Constructor-style initialiser: installs the vtable at 0x005f9a0c, clears +4, returns this
// (same shape as the twin 0x0045afb0 with another vtable).
// FUNCTION: GP4 0x005a73a0
Object_005a73a0* __fastcall FUN_005a73a0(Object_005a73a0* self, void* /*edx*/) {
    self->Vptr00 = GP4_ARRAY(const uint32_t, 0x005f9a0c);
    self->Field04 = 0;
    return self;
}
GP4_IMPL(0x005a73a0, FUN_005a73a0, "thiscall(ptr[0x10]:bytes) -> u32")
