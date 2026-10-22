#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_005a7390 { const void* Vptr00; };
#pragma pack(pop)

// Constructor-like routine: stores the vtable pointer 0x005f9a04 at +0.
// FUNCTION: GP4 0x005a7390
void __fastcall FUN_005a7390(Object_005a7390* self, void* /*edx*/) {
    self->Vptr00 = GP4_ARRAY(const uint32_t, 0x005f9a04);
}
GP4_IMPL(0x005a7390, FUN_005a7390, "thiscall(ptr[0x10]:bytes) -> void")
