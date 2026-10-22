#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_0045ef30 { const void* Vptr00; };
#pragma pack(pop)

// Constructor-like routine: stores the vtable pointer 0x005f05f4 at +0.
// FUNCTION: GP4 0x0045ef30
void __fastcall FUN_0045ef30(Object_0045ef30* self, void* /*edx*/) {
    self->Vptr00 = GP4_ARRAY(const uint32_t, 0x005f05f4);
}
GP4_IMPL(0x0045ef30, FUN_0045ef30, "thiscall(ptr[0x10]:bytes) -> void")
