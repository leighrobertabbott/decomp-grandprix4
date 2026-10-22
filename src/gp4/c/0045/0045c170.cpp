#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_0045c170 { const void* Vptr00; };
#pragma pack(pop)

// Constructor-like routine: stores the vtable pointer 0x005f05c8 at +0.
// FUNCTION: GP4 0x0045c170
void __fastcall FUN_0045c170(Object_0045c170* self, void* /*edx*/) {
    self->Vptr00 = GP4_ARRAY(const uint32_t, 0x005f05c8);
}
GP4_IMPL(0x0045c170, FUN_0045c170, "thiscall(ptr[0x10]:bytes) -> void")
