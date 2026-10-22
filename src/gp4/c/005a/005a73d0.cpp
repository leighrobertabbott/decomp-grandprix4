#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_005a73d0 { const void* Vptr00; };
#pragma pack(pop)

// Constructor-like routine: stores the vtable pointer 0x005f9a0c at +0.
// FUNCTION: GP4 0x005a73d0
void __fastcall FUN_005a73d0(Object_005a73d0* self, void* /*edx*/) {
    self->Vptr00 = GP4_ARRAY(const uint32_t, 0x005f9a0c);
}
GP4_IMPL(0x005a73d0, FUN_005a73d0, "thiscall(ptr[0x10]:bytes) -> void")
