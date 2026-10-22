#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_0044e740 { const void* Vptr00; };
#pragma pack(pop)

// Constructor-like routine: stores the vtable pointer 0x005f04e8 at +0.
// FUNCTION: GP4 0x0044e740
void __fastcall FUN_0044e740(Object_0044e740* self, void* /*edx*/) {
    self->Vptr00 = GP4_ARRAY(const uint32_t, 0x005f04e8);
}
GP4_IMPL(0x0044e740, FUN_0044e740, "thiscall(ptr[0x10]:bytes) -> void")
