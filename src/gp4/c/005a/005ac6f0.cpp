#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_005ac6f0 { const void* Vptr00; };
#pragma pack(pop)

// Constructor-like routine: stores the vtable pointer 0x005f9c48 at +0.
// FUNCTION: GP4 0x005ac6f0
void __fastcall FUN_005ac6f0(Object_005ac6f0* self, void* /*edx*/) {
    self->Vptr00 = GP4_ARRAY(const uint32_t, 0x005f9c48);
}
GP4_IMPL(0x005ac6f0, FUN_005ac6f0, "thiscall(ptr[0x10]:bytes) -> void")
