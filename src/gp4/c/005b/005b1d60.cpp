#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_005b1d60 { const void* Vptr00; };
#pragma pack(pop)

// Constructor-like routine: stores the vtable pointer 0x005f9e8c at +0.
// FUNCTION: GP4 0x005b1d60
void __fastcall FUN_005b1d60(Object_005b1d60* self, void* /*edx*/) {
    self->Vptr00 = GP4_ARRAY(const uint32_t, 0x005f9e8c);
}
GP4_IMPL(0x005b1d60, FUN_005b1d60, "thiscall(ptr[0x10]:bytes) -> void")
